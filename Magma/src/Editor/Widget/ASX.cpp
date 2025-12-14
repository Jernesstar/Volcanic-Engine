#include "ASX.h"

#include <cctype>

#include <VolcaniCore/Core/FileUtils.h>

namespace Magma::UI {

char ASXLexer::Peek(u32 offset) const {
	return m_Pos < m_Source.size() ? m_Source[m_Pos + offset] : '\0';
}

char ASXLexer::Advance() {
	return m_Source[m_Pos++];
}

bool ASXLexer::Match(char c) {
	Advance();
	while(std::isspace(Peek()))
		Advance();
	return Peek() == c;
}

Token ASXLexer::NextToken() {
	if(m_Pos >= m_Source.size())
		return { TokenType::EndOfFile };

	switch(m_State.Mode) {
		case LexerMode::AngelScript: return LexAngelScript();
		case LexerMode::XML:         return LexXML();
		case LexerMode::Expression:  return LexExpression();
	}

	return { TokenType::EndOfFile };
}

void ASXLexer::Revert() {
	VOLCANICORE_ASSERT(m_Pos > 0);
	m_Pos--;
}

Token ASXLexer::LexAngelScript() {
	char c = Advance();

	if(std::isspace(c))
		return NextToken();

	if(c == '(') {
		auto pos = m_Pos;
		if(Match('<')) {
			// Not advancing so as to re-emit the '<'
			// Advance();
			m_State.ParentDepth++;
			m_State.Mode = LexerMode::XML;
			return { TokenType::XMLBlockOpen, "(<", m_Line, m_Column };
		}

		m_Pos = pos;
	}

	if(std::isalpha(c)) {
		std::string id(1, c);
		while(std::isalnum(Peek()) || Peek() == '_')
			id += Advance();

		return { TokenType::Identifier, id, m_Line, m_Column };
	}

	if(std::isdigit(c)) {
		std::string num(1, c);
		while(std::isdigit(Peek()))
			num += Advance();
		return { TokenType::Number, num, m_Line, m_Column };
	}

	if(c == '"' || c == '\'') {
		std::string str;
		while(Peek() && Peek() != c)
			str += Advance();
		Advance();
		return { TokenType::String, str, m_Line, m_Column };
	}

	return { TokenType::Symbol, std::string(1, c), m_Line, m_Column };
}

Token ASXLexer::LexExpression() {
	if(Peek() == '}') {
		Advance();
		m_State.BraceDepth--;
		if(m_State.BraceDepth == 0)
			m_State.Mode = LexerMode::XML;
		return { TokenType::ExpressionClose, "}", m_Line, m_Column };
	}

	return LexAngelScript();
}

Token ASXLexer::LexXML() {
	if(Peek() == '/' && Peek(1) == '>') {
		Advance();
		Advance();
		return { TokenType::XMLSelfClose, "/>", m_Line, m_Column };
	}

	if(Peek() == '{') {
		Advance();
		m_State.BraceDepth++;
		m_State.Mode = LexerMode::Expression;
		return { TokenType::ExpressionOpen, "{", m_Line, m_Column };
	}

	if(Peek() == '<') {
		if(Peek(1) == '/') {
			Advance();
			return { TokenType::XMLEndTagOpen, "</", m_Line, m_Column };
		}

		Advance();
		return { TokenType::XMLTagOpen, "<", m_Line, m_Column };
	}

	if(Peek() == '>') {
		Advance();
		return { TokenType::XMLTagClose, ">", m_Line, m_Column };
	}

	if(Peek() == ')') {
		Advance();
		m_State.Mode = LexerMode::Expression;
		return { TokenType::XMLBlockClose, ">)", m_Line, m_Column };
	}

	char c = Advance();

	if(c == '"' || c == '\'') {
		std::string value;
		while(Peek() && Peek() != c)
			value += Advance();
		Advance();
		return { TokenType::XMLString, value, m_Line, m_Column };
	}

	if(std::isalpha(c)) {
		std::string id(1, c);
		while(std::isalnum(Peek()) || Peek() == '-')
			id += Advance();
		return { TokenType::XMLIdentifier, id, m_Line, m_Column };
	}

	return LexXML();
}

ASXNode* ASXCompiler::Compile(const std::string& path) {
	auto str = FileUtils::ReadFile(path);
	ASXLexer lexer(str);

	ASXNode* root = new ASXRootNode();
	printf("Root\n");
	printf("  |\n");
	printf("  -->");
	while(true) {
		auto token = lexer.NextToken();
		if(token.Type == TokenType::EndOfFile)
			break;
		Parse(lexer, token, root);
	}

	return root;
}

void ASXCompiler::Parse(ASXLexer& lexer, Token token, ASXNode* parent) {
	// VOLCANICORE_LOG_INFO("{ %i, %s }", token.Type, token.Lexeme.c_str());
	// return;

	if(token.Type == TokenType::EndOfFile) {
		printf("EOF\n");
		return;
	}

	ASXNode* node = nullptr;
	if((u32)token.Type <= (u32)TokenType::Symbol) { // AngelScript
		printf("Symbol %s\n", token.Lexeme.c_str());
		printf("  |\n");
		printf("  -->");

		// while(true) {
		// 	auto token = lexer.NextToken();
		// 	if(token.Type == TokenType::XMLBlockOpen) {
		// 		lexer.Revert();
		// 		break;
		// 	}
		// 	parent->Children.Add(Parse(lexer, token, parent));
		// }

		// if(token.Lexeme == "import") {
		// 	auto node = new ASXScriptNode();
		// 	while(true) {
		// 		auto token = lexer.NextToken();
		// 		node->Children.Add(Parse(lexer, token, node));
		// 		if(token.Type == TokenType::String) // reached include path
		// 			break;
		// 	}
		// }
	}
	else if(token.Type == TokenType::XMLBlockOpen) {
		printf(" BlockOpen\n");
		printf("\t|\n");
		printf("\t-->");
		node = new ASXBlockNode();
		while(true) {
			auto token = lexer.NextToken();
			if(token.Type == TokenType::XMLBlockClose)
				break;
			Parse(lexer, token, node);
		}
		printf(" BlockClose\n\r");
	}
	else if(token.Type == TokenType::XMLTagOpen) {
		node = new ASXElementNode();
		auto n = node->As<ASXElementNode>();
		auto nameToken = lexer.NextToken();
		VOLCANICORE_ASSERT(nameToken.Type == TokenType::XMLIdentifier);
		n->TypeName = nameToken.Lexeme;
		printf(" Tag:%s<", nameToken.Lexeme.c_str());

		// Parse attributes
		while(true) {
			auto attrToken = lexer.NextToken();
			if(attrToken.Type == TokenType::XMLTagClose) {
				printf(">\n");
				break;
			}
			if(attrToken.Type == TokenType::XMLSelfClose) {
				printf("/>\n");
				parent->Children.Add(node);
				return;
			}

			VOLCANICORE_ASSERT(attrToken.Type == TokenType::XMLIdentifier);
			auto valueToken = lexer.NextToken();
			VOLCANICORE_ASSERT(valueToken.Type == TokenType::XMLString);
			n->Attributes[attrToken.Lexeme] = valueToken.Lexeme;

			printf("%s:%s,", attrToken.Lexeme.c_str(), valueToken.Lexeme.c_str());
		}

		// If not self close tag, parse children
		while(true) {
			auto token = lexer.NextToken();
			if(token.Type == TokenType::XMLEndTagOpen)
				break;
			Parse(lexer, token, node);
		}

		// Parse end tag
		auto endToken = lexer.NextToken();
		VOLCANICORE_ASSERT(endToken.Lexeme == nameToken.Lexeme);
	}

	else if(token.Type == TokenType::ExpressionOpen) {
		node = new ASXExpressionNode();
		while(true) {
			auto token = lexer.NextToken();
			if(token.Type == TokenType::ExpressionClose)
				break;
			Parse(lexer, token, node);
		}
	}

	parent->Children.Add(node);
}

}
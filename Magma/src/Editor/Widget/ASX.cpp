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

void ASXCompiler::Compile(const std::string& path) {
	auto str = FileUtils::ReadFile(path);
	ASXLexer lexer(str);

	ASXNode* root = new ASXRootNode();
	while(true) {
		auto token = lexer.NextToken();
		if(token.Type == TokenType::EndOfFile)
			break;
		Parse(lexer, token, root);
	}

	File script(path + ".compiled", true);
	Emit(root, script);
}

void ASXCompiler::Parse(ASXLexer& lexer, Token token, ASXNode* parent) {
	VOLCANICORE_LOG_INFO("{ %i, %s }", token.Type, token.Lexeme.c_str());
	// return;

	if(token.Type == TokenType::EndOfFile)
		return;

	ASXNode* node = nullptr;
	if((u32)token.Type <= (u32)TokenType::Symbol) { // AngelScript
		// node = new ASXScriptNode();
		// auto n = node->As<ASXScriptNode>();

		// if(token.Lexeme == "#") { // include
		// 	while(true) {
		// 		auto t = lexer.NextToken();
		// 		n->Script += t.Lexeme;
		// 		if(t.Type == TokenType::String)
		// 			break;
		// 	}
		// }
		// else {
		// 	while(true) {
		// 		auto t = lexer.NextToken();
		// 		if(t.Type == TokenType::XMLBlockOpen) {
		// 			n->Script += "{0}"; 
		// 			Parse(lexer, t, node);
		// 		}

		// 		n->Script += t.Lexeme;
		// 		if(t.Lexeme == ";")
		// 			break;
		// 	}
		// }
	}
	else if(token.Type == TokenType::XMLBlockOpen) {
		node = new ASXBlockNode();
		while(true) {
			auto t = lexer.NextToken();
			if(t.Type == TokenType::XMLBlockClose)
				break;
			Parse(lexer, t, node);
		}
	}
	else if(token.Type == TokenType::XMLTagOpen) {
		node = new ASXElementNode();
		auto n = node->As<ASXElementNode>();
		auto nameToken = lexer.NextToken();
		VOLCANICORE_ASSERT(nameToken.Type == TokenType::XMLIdentifier);
		n->TypeName = nameToken.Lexeme;

		// Parse attributes
		while(true) {
			auto attrToken = lexer.NextToken();
			if(attrToken.Type == TokenType::XMLTagClose)
				break;
			if(attrToken.Type == TokenType::XMLSelfClose) {
				parent->Children.Add(node);
				return;
			}

			VOLCANICORE_ASSERT(attrToken.Type == TokenType::XMLIdentifier);
			auto valueToken = lexer.NextToken();
			if(valueToken.Type == TokenType::XMLString)
				n->Attributes[attrToken.Lexeme] = valueToken;
			else if(valueToken.Lexeme == "{") {
				while(true) {
					auto t = lexer.NextToken();
					if(t.Type == TokenType::ExpressionClose)
						break;
					n->Attributes[attrToken.Lexeme].Lexeme += t.Lexeme;
				}
			}
			else
				VOLCANICORE_ASSERT(false, "Invalid attribute value");
		}

		// If not self close tag, parse children
		while(true) {
			auto t = lexer.NextToken();
			if(t.Type == TokenType::XMLEndTagOpen)
				break;
			Parse(lexer, t, node);
		}

		// Parse end tag
		auto endToken = lexer.NextToken();
		VOLCANICORE_ASSERT(endToken.Lexeme == nameToken.Lexeme,
			std::format("{0}", endToken.Lexeme).c_str());
		VOLCANICORE_ASSERT(lexer.NextToken().Type == TokenType::XMLTagClose);
	}

	else if(token.Type == TokenType::ExpressionOpen) {
		node = new ASXScriptNode();
		while(true) {
			auto t = lexer.NextToken();
			if(t.Type == TokenType::ExpressionClose)
				break;
			Parse(lexer, t, node);
		}
	}

	parent->Children.Add(node);
}

void ASXCompiler::Emit(ASXNode* node, File& script) {
	if(node->Type == ASXNodeType::Root) {
		auto n = node->As<ASXRootNode>();
		printf("ASXRootNode\n");
	}
	else if(node->Type == ASXNodeType::Block) {
		auto n = node->As<ASXBlockNode>();
		printf("ASXBlockNode\n");
	}
	else if(node->Type == ASXNodeType::Script) {
		auto n = node->As<ASXScriptNode>();
		printf("ASXScriptNode: %s\n", n->Script.c_str());
	}
	else if(node->Type == ASXNodeType::Element) {
		auto n = node->As<ASXElementNode>();
		printf("ASXElementNode: %s\n", n->TypeName.c_str());
	}

	for(auto child : node->Children)
		Emit(child, script);
}

}
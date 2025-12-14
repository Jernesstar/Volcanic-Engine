#include "ASX.h"

#include <cctype>

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

Token ASXLexer::LexAngelScript() {
	char c = Advance();

	if(std::isspace(c))
		return NextToken();

	if(c == '(') {
		auto pos = m_Pos;
		if(Match('<')) {
			Advance();
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
	char c = Advance();

	if(c == '}') {
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

	char c = Advance();

	if(c == '{') {
		m_Pos--;
		m_State.Mode = LexerMode::Expression;
		return { TokenType::ExpressionOpen, "{", m_Line, m_Column };
	}

	auto pos = m_Pos;
	if(c == '>' && Match(')')) {
		Advance();
		m_State.Mode = LexerMode::AngelScript;
		return { TokenType::XMLBlockClose, ">)", m_Line, m_Column };
	}
	m_Pos = pos;

	if(c == '<') {
		if(Peek(1) == '/') {
			Advance();
			return { TokenType::XMLEndTagOpen, "</", m_Line, m_Column };
		}

		return { TokenType::XMLTagOpen, "<", m_Line, m_Column };
	}

	if(c == '>') {
		return { TokenType::XMLTagClose, ">", m_Line, m_Column };
	}

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

}
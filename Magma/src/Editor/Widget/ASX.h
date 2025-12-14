#pragma once

#include <VolcaniCore/Core/Defines.h>

namespace Magma::UI {

enum class LexerMode {
	AngelScript,
	XML,
	Expression
};

struct LexerState {
	LexerMode Mode = LexerMode::AngelScript;

	u64 ParentDepth = 0; // used to detect return ( ... )
	u64 BraceDepth = 0; // for expressions inside XML
	u64 XMLDepth = 0;   // nested tags
};

enum class TokenType {
	// AngelScript
	Identifier,        // functions, variables
	Keyword,           // return, include, import, for, if, etc
	Number,            // 123
	String,            // "string"
	Symbol,            // ;, :, *, /, }, ), etc

	// XML
	XMLTagOpen,        // <
	XMLTagClose,       // >
	XMLIdentifier,     // tag / attribute
	XMLString,         // string
	XMLSelfClose,      // />
	XMLEndTagOpen,     // </

	// ASX
	XMLBlockOpen,      // (<
	XMLBlockClose,     // >)

	// Expressions
	ExpressionOpen,    // {
	ExpressionClose,   // }

	EndOfFile
};

struct Token {
	TokenType Type;
	std::string Lexeme;
	u64 Line;
	u64 Column;
};

class ASXLexer {
public:
	ASXLexer(const std::string& src)
		: m_Source(src) {}

	Token NextToken();

private:
	char Peek(u32 offset = 0) const;
	char Advance();
	bool Match(char c);

	Token LexAngelScript();
	Token LexExpression();
	Token LexXML();

private:
	std::string m_Source;
	LexerState m_State;
	u64 m_Pos = 0;
	u64 m_Line = 1;
	u64 m_Column = 1;
};

class ASXCompiler {
public:
	static void Compile(const std::string& path);
};

}
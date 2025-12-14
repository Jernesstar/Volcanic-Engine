#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Magma::UI {

enum class LexerMode {
	AngelScript,
	XML,
	Expression
};

struct LexerState {
	LexerMode Mode = LexerMode::AngelScript;

	u64 ParentDepth = 0; // used to detect return ( ... )
	u64 BraceDepth = 0;  // for expressions inside XML
	u64 XMLDepth = 0;    // nested tags
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
	void Revert();

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

enum class ASXNodeType {
	Root,
	Script,    // AngelScript
	Block,     // XML
	Element,   // XML tag
	Expression // { ... }
};

class ASXNode : public Derivable<ASXNode> {
public:
	ASXNodeType Type = ASXNodeType::Script;
	List<ASXNode*> Children;

public:
	ASXNode(ASXNodeType type)
		: Type(type) { }
	virtual ~ASXNode() = default;
};

class ASXRootNode : public ASXNode {
public:
	ASXRootNode()
		: ASXNode(ASXNodeType::Root) { }
};

class ASXScriptNode : public ASXNode {
public:
	std::string Script;

public:
	ASXScriptNode()
		: ASXNode(ASXNodeType::Script) { }
};

class ASXBlockNode : public ASXNode {
public:
	ASXBlockNode()
		: ASXNode(ASXNodeType::Block) { }
};

class ASXElementNode : public ASXNode {
public:
	std::string TypeName;
	Map<std::string, std::string> Attributes;

public:
	ASXElementNode()
		: ASXNode(ASXNodeType::Element) { }
};

class ASXExpressionNode : public ASXNode {
public:
	std::string Expression;

public:
	ASXExpressionNode()
		: ASXNode(ASXNodeType::Expression) { }
};

class ASXCompiler {
public:
	static ASXNode* Compile(const std::string& path);

public:
	static void Parse(ASXLexer& lexer, Token token, ASXNode* parent);
};

}
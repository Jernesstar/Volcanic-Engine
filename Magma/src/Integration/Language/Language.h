#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/List.h>

#include "Graph.h"

using namespace VolcaniCore;

namespace Magma::Language {

enum class ASTType { Function, Class, Variable, Expression };

struct ASTNode {
	ASTType Type;
	std::string Value;
	List<ASTNode> Children;
};

class Parser : public Derivable<Parser> {
public:
	Parser() = default;
	virtual ~Parser() = default;

	virtual ASTNode* Parse(const std::string& code) = 0;

private:
	ASTNode* m_Root = nullptr;
};

}
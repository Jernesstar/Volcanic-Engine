#include "Language.h"

#include <VolcaniCore/Core/FileUtils.h>

namespace Magma::Language {

static Map<std::string, Ref<Parser>> s_Parsers;

static Map<std::string, List<std::string>> s_Extensions;

class CppParser : public Parser {
public:
	ASTNode* Parse(const std::string& code) override {

	}
};

void LanguageManager::Init() {
	s_Extensions = {
		{ "cpp", { ".h", ".hpp", ".cpp", ".cc", ".C" } },
		{ "c", { ".h", ".c" } },
		{ "lua", { ".lua" } }
	};

	s_Parsers["cpp"] = CreateRef<CppParser>();
}

void LanguageManager::Shutdown() {
	s_Parsers.clear();
}

Ref<Parser> LanguageManager::GetParser(const std::string& path) {
	auto ext = fs::path(path).extension().string();
	for(auto& [lang, exts] : s_Extensions) {
		if(exts.Find(ext))
			return s_Parsers[lang];
	}

	// VOLCANICORE_LOG_INFO("No parser for file %s", path.c_str());
	return nullptr;
}

}
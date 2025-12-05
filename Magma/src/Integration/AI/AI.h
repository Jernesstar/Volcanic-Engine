#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/List.h>

#include <rapidjson/rapidjson.h>

using namespace VolcaniCore;

namespace Magma::AI {

struct Document {
	std::string ID;
	std::string Type;
	std::string Content;
	std::string Title;
	std::string Source;
	std::string Path;
	std::string Metadata;
};

class EmbeddingStore {
public:
	static Ref<EmbeddingStore> Create();

public:
	EmbeddingStore() = default;
	virtual ~EmbeddingStore() = default;

	virtual List<f32> EmbedText(const std::string& text) = 0;
};

struct SearchResult {
	Document Doc;
	f32 Score;
};

class VectorStore {
public:
	static Ref<VectorStore> Create();

public:
	VectorStore() = default;
	virtual ~VectorStore() = default;

	virtual bool Add(const Document& doc, const List<f32>& v) = 0;
	virtual List<SearchResult> SearchByVector(const List<f32>& v, u32 topK) = 0;
	List<SearchResult> SearchByText(const std::string& text,
									Ref<EmbeddingStore> embedder, u32 topK)
	{
		auto v = embedder->EmbedText(text);
		return SearchByVector(v, topK);
	}
};

struct ModelInput {
	std::string SystemPrompt;
	std::string UserPrompt;
	u32 MaxTokens = 512;
	f32 Temperature = 0.2f;
};

class ModelProvider {
public:
	static Ref<ModelProvider> Create();

public:
	ModelProvider() = default;
	virtual ~ModelProvider() = default;

	virtual std::string Generate(const ModelInput& input) = 0;
	// virtual std::string GenerateStream(const ModelInput &input,
	// 	Func<void, const std::string& tokenPart> onToken) = 0;
};

class AIManager {
public:
	static void Init();
	static void Close();
	static void RunAnalysis();
};

}
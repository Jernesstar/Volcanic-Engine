#include "AI.h"

#include <thread>

#include <nlohmann/json.hpp>

#include "Networking/Networking.h"

using json = nlohmann::json;
using namespace Magma::Networking;

namespace Magma::AI {

class LlamaHttpEmbeddingStore : public EmbeddingStore {
public:
	LlamaHttpEmbeddingStore(u16 port = 8081)
		: m_Client("localhost", port)
	{
		std::string path;
#if defined(VOLCANIC_WINDOWS)
		path = "Magma/.vendor/llamacpp/bin/Win/cpu";
#elif defined(VOLCANIC_LINUX)
		path = "Magma/.vendor/llamacpp/bin/Linux/cpu";
#endif

		std::string command = path +
			"/llama-server --port 8081 --no-webui --embedding"
			" --hf-repo Triangle104/SmolLM2-360M-Q4_K_S-GGUF"
			" --hf-file smollm2-360m-q4_k_s.gguf -c 2048 &";

		system(command.c_str());

		std::this_thread::sleep_for(std::chrono::seconds(9));

		auto resp = m_Client.Get({ "/health", PayloadType::JSON, "" });
		if(resp.StatusCode < 200 || resp.StatusCode >= 300)
			throw std::runtime_error(
				"Llama model server health check failed: " +
				std::to_string(resp.StatusCode));

		VOLCANICORE_LOG_INFO("Health: %s", resp.Body.c_str());
	}
	~LlamaHttpEmbeddingStore() = default;

	List<f32> EmbedText(const std::string& text) override {
		json body = {
			{ "content", text },
			{ "model", "" }
		};
		auto resp =
			m_Client.Post({ "/v1/embeddings", PayloadType::JSON, body.dump() });

		if(resp.StatusCode < 200 || resp.StatusCode >= 300)
			throw std::runtime_error(
				"Embed request failed: " + std::to_string(resp.StatusCode));

		auto response = json::parse(resp.Body);
		if(!response.contains("embedding"))
			throw std::runtime_error("embed response missing 'embedding'");

		List<f32> vec;
		for(auto& v : response["embedding"])
			vec.Add(v.get<f32>());

		return vec;
	}

private:
	HttpClient m_Client;
};

Ref<EmbeddingStore> EmbeddingStore::Create() {
	return CreateRef<LlamaHttpEmbeddingStore>();
}

class QdrantVectorStore : public VectorStore {
public:
	QdrantVectorStore(const std::string& collection, u16 port = 6333)
		: m_Collection(collection), m_Client("127.0.0.1", port)
	{
		system("docker run -p 6333:6333 --name vector_store qdrant/qdrant &");
	}
	~QdrantVectorStore() {
		system("docker stop vector_store && docker rm vector_store");
	}

	bool Add(const Document& doc, const List<f32>& vector) override {
		// Build Qdrant upsert payload
		json point = {
			{ "id", doc.ID },
			{ "vector", vector },
			{
				"payload",
				{
					{ "title", doc.Title },
					{ "type", doc.Type },
					{ "path", doc.Path },
					{ "content", doc.Content }, // TODO(Optimize): Move to separate storage
					{ "metadata", doc.Metadata }
				}
			}
		};

		json body = { { "points", json::array({point}) } };
		std::string url = "/collections/" + m_Collection + "/points?wait=true";
		try {
			auto resp = m_Client.Post({ url, PayloadType::JSON, body.dump() });
			if(resp.StatusCode >= 200 && resp.StatusCode < 300)
				return true;
			else {
				VOLCANICORE_LOG_WARNING("Qdrant upsert failed for doc %s\n. \
					Error: %s", doc.Title.c_str(), resp.Body.c_str());
				return false;
			}
			return false;

		} catch(const std::exception& e) {
			// log e.what()
			return false;
		}
	}

	List<SearchResult> SearchByVector(const List<f32>& vector,
									  u32 topK) override
	{
		json body = {
			{ "vector", vector },
			{ "top", topK },
			{ "with_payload", true }
		};
		std::string url = "/collections/" + m_Collection + "/points/search";
		List<SearchResult> out;

		try {
			auto resp = m_Client.Post({ url, PayloadType::JSON, body.dump() });
			if(resp.StatusCode < 200 || resp.StatusCode >= 300)
				return out;

			auto response = json::parse(resp.Body);
			// Qdrant response format:
			// { "result": [ { "id":..., "payload": {...}, "score": ... }, ... ] }
			if(response.contains("result")) {
				for(auto& r : response["result"]) {
					SearchResult sr;
					sr.Score = r.value("score", 0.0f);

					if(r.contains("payload")) {
						auto p = r["payload"];
						sr.Doc.ID = r.value("id", "");
						sr.Doc.Title = p.value("title", "");
						sr.Doc.Type = p.value("type", "");
						sr.Doc.Path = p.value("path", "");
						sr.Doc.Content = p.value("content", "");
						sr.Doc.Metadata = p.value("metadata", "");
					}

					out.Add(sr);
				}
			}
		} catch (const std::exception &e) {
			// log
		}

		return out;
	}

private:
	std::string m_Collection;
	HttpClient m_Client;
};

Ref<VectorStore> VectorStore::Create() {
	return CreateRef<QdrantVectorStore>("Initial");
}

class LlamaModelProvider : public ModelProvider {
public:
	LlamaModelProvider(u16 port = 8080)
		: m_Client("127.0.0.1", port)
	{
		std::string path;
#if defined(VOLCANIC_WINDOWS)
		path = "Magma/.vendor/llamacpp/bin/Win/cpu";
#elif defined(VOLCANIC_LINUX)
		path = "Magma/.vendor/llamacpp/bin/Linux/cpu";
#endif

		std::string command = path +
			"/llama-server --port 8080 --no-webui"
			" --hf-repo Triangle104/SmolLM2-360M-Q4_K_S-GGUF"
			" --hf-file smollm2-360m-q4_k_s.gguf -c 2048 &";

		system(command.c_str());

		std::this_thread::sleep_for(std::chrono::seconds(9));

		auto resp = m_Client.Get({ "/health", PayloadType::JSON, "" });
		if(resp.StatusCode < 200 || resp.StatusCode >= 300)
			throw std::runtime_error(
				"Llama model server health check failed: " +
				std::to_string(resp.StatusCode));

		VOLCANICORE_LOG_INFO("Health: %s", resp.Body.c_str());
	}
	~LlamaModelProvider() = default;

	std::string Generate(const ModelInput& input) override {
		json body = {
			{ "prompt", input.SystemPrompt + "\n\n" + input.UserPrompt },
			{ "max_tokens", input.MaxTokens },
			{ "temperature", input.Temperature }
		};

		auto resp =
			m_Client.Post({ "/completion", PayloadType::JSON, body.dump() });
		if(resp.StatusCode < 200 || resp.StatusCode >= 300)
			throw std::runtime_error(
				"Generate request failed: " + std::to_string(resp.StatusCode));

		auto response = json::parse(resp.Body);
		if(!response.contains("content"))
			throw std::runtime_error("Generate response missing 'content' field"
				"\nResponse: " + resp.Body);

		return response["content"].get<std::string>();
	}

private:
	HttpClient m_Client;
};

Ref<ModelProvider> ModelProvider::Create() {
	return CreateRef<LlamaModelProvider>();
}

static Ref<EmbeddingStore> s_EmbeddingStore;
static Ref<VectorStore> s_VectorStore;
static Ref<ModelProvider> s_ModelProvider;

void AIManager::Init() {
	s_ModelProvider = ModelProvider::Create();
	s_EmbeddingStore = EmbeddingStore::Create();
	s_VectorStore = VectorStore::Create();
}

void AIManager::Close() {
	s_EmbeddingStore.reset();
	s_VectorStore.reset();
	s_ModelProvider.reset();
}

void AIManager::RunAnalysis() {
	// Example analysis
	ModelInput input;
	input.SystemPrompt = "You are an AI assistant that helps with code analysis.";
	input.UserPrompt = "Summarize the following code snippet:\n\n"
					   "def add(a, b):\n    return a + b";
	input.MaxTokens = 100;
	input.Temperature = 0.3f;

	std::string response = s_ModelProvider->Generate(input);
	VOLCANICORE_LOG_INFO("AI Analysis Response: %s", response.c_str());

	// Example vector and embedding store usage
	Document doc;
	doc.ID = "1";
	doc.Title = "Addition Function";
	doc.Type = "code";
	doc.Path = "/path/to/code.py";
	doc.Content = "def add(a, b): return a + b";
	doc.Metadata = "A simple addition function in Python.";
	List<f32> embedding = s_EmbeddingStore->EmbedText(doc.Content);
	s_VectorStore->Add(doc, embedding);
	List<SearchResult> results =
		s_VectorStore->SearchByText("function that adds two numbers",
									s_EmbeddingStore, 5);
	for(auto& res : results) {
		VOLCANICORE_LOG_INFO("Found Document: %s with score %f",
							res.Doc.Title.c_str(), res.Score);
	}
}

}
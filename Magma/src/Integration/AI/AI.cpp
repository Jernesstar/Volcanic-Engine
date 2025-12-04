#include "AI.h"

#include <thread>

#include <nlohmann/json.hpp>

#include "Networking/Networking.h"

using json = nlohmann::json;
using namespace Magma::Networking;

namespace Magma::AI {

class LlamaHttpEmbeddingStore : public EmbeddingStore {
public:
	LlamaHttpEmbeddingStore(u16 port = 8080)
		: m_Client("localhost", port) { }
	~LlamaHttpEmbeddingStore() = default;

	List<f32> EmbedText(const std::string& text) override {
		json body = { { "input", text } };
		auto resp = m_Client.Post({ "/embed", PayloadType::JSON, body.dump() });

		if(resp.StatusCode < 200 || resp.StatusCode >= 300)
			throw std::runtime_error(
				"Embed request failed: " + std::to_string(resp.StatusCode));

		auto response = json::parse(resp.Body);
		if(!response.contains("embedding"))
			throw std::runtime_error("embed response missing embedding");

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
		: m_Collection(collection), m_Client("localhost", port)
	{
		system("docker run -p 6333:6333 qdrant/qdrant &");
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

}
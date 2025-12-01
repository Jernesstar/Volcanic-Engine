#include "AI.h"

#include <thread>

#include "Networking/Networking.h"

using namespace Magma::Networking;

namespace Magma::AI {

static Ref<std::thread> s_LLMThread;
static std::thread s_HttpThread;

Assistant::Assistant() {
	s_LLMThread = CreateRef<std::thread>(
		[]()
		{
			system("llama-server --hf-repo ngxson/SmolLM2-360M-Instruct-Q8_0-GGUF --hf-file smollm2-360m-instruct-q8_0.gguf -c 2048 --main-gpu 1");
		});
	s_LLMThread->detach();
}

Assistant::~Assistant() {
	s_LLMThread.reset();
}
	
Response Assistant::HandleRequest(const std::string& request) {
	Bytes data(request.length());
	data.Set(request.data(), request.length());

	auto client = HttpClient("127.0.0.1", 8080);
	client.Post("/completion", std::move(data), PayloadType::Json,
		[](const ResponseResult res, Networking::Response response)
		{
		});
}
	

}
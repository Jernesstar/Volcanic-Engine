#pragma once

#include <drogon/drogon.h>
#include <jwt-cpp/jwt.h>
#include <openssl/sha.h>

#include <VolcaniCore/Core/List.h>

using namespace VolcaniCore;

namespace Magma::Server {

struct User {
	std::string Name;
	std::string Username;
	std::string Email;
	std::string Password;
	std::string PasswordHash;
};

class UserService {
public:
	static void Init();

private:
	inline static List<User> s_Users;
};

}
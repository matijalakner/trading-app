#pragma once
#include <string>

class ApiClient;

class AuthManager
{
public:
    explicit AuthManager(ApiClient& api);

    bool Login(const std::string& username, const std::string& password);
    void Logout();

    bool IsAuthenticated() const;
    const std::string& GetUsername() const;

private:
    ApiClient& api;
    std::string token;
    std::string username;
    bool authenticated = false;
};

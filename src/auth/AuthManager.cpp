#include "AuthManager.h"
#include "api/ApiClient.h"
#include <nlohmann/json.hpp>

AuthManager::AuthManager(ApiClient& api) : api(api) {}

bool AuthManager::Login(
    const std::string& requestedUsername,
    const std::string& password)
{
    const auto response = api.Post(
        "/login",
        nlohmann::json{
            {"username", requestedUsername},
            {"password", password}
        });

    if (!response.success ||
        !response.data.is_object() ||
        !response.data.contains("token") ||
        !response.data["token"].is_string())
    {
        return false;
    }

    token = response.data["token"].get<std::string>();
    username = requestedUsername;
    authenticated = true;
    api.SetToken(token);
    return true;
}

void AuthManager::Logout()
{
    token.clear();
    username.clear();
    authenticated = false;
    api.ClearToken();
}

bool AuthManager::IsAuthenticated() const
{
    return authenticated;
}

const std::string& AuthManager::GetUsername() const
{
    return username;
}

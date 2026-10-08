#pragma once
#include <nlohmann/json.hpp>
#include <string>

struct ApiResponse
{
    long statusCode = 0;
    bool success = false;
    nlohmann::json data;
    std::string error;
};

class ApiClient
{
public:
    explicit ApiClient(const std::string& baseUrl);

    ApiResponse Get(const std::string& endpoint);
    ApiResponse Post(const std::string& endpoint, const nlohmann::json& body);
    ApiResponse Put(const std::string& endpoint, const nlohmann::json& body);
    ApiResponse Delete(const std::string& endpoint);

    void SetToken(const std::string& token);
    void ClearToken();

private:
    ApiResponse Request(
        const std::string& method,
        const std::string& endpoint,
        const std::string& body = "");

    std::string baseUrl;
    std::string token;
};

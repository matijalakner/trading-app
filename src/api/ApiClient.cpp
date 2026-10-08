#include "ApiClient.h"
#include <curl/curl.h>
#include <iostream>
#include <mutex>

namespace
{
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    const size_t total = size * nmemb;
    static_cast<std::string*>(userp)->append(
        static_cast<char*>(contents), total);
    return total;
}

void EnsureCurlInitialized()
{
    static std::once_flag flag;
    std::call_once(flag, [] {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    });
}
}

ApiClient::ApiClient(const std::string& url) : baseUrl(url)
{
    EnsureCurlInitialized();
    while (!baseUrl.empty() && baseUrl.back() == '/')
        baseUrl.pop_back();
}

void ApiClient::SetToken(const std::string& value) { token = value; }
void ApiClient::ClearToken() { token.clear(); }

ApiResponse ApiClient::Get(const std::string& endpoint)
{
    return Request("GET", endpoint);
}

ApiResponse ApiClient::Post(const std::string& endpoint, const nlohmann::json& body)
{
    return Request("POST", endpoint, body.dump());
}

ApiResponse ApiClient::Put(const std::string& endpoint, const nlohmann::json& body)
{
    return Request("PUT", endpoint, body.dump());
}

ApiResponse ApiClient::Delete(const std::string& endpoint)
{
    return Request("DELETE", endpoint);
}

ApiResponse ApiClient::Request(
    const std::string& method,
    const std::string& endpoint,
    const std::string& body)
{
    ApiResponse response;
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        response.error = "Failed to initialize CURL";
        return response;
    }

    const std::string url = baseUrl + endpoint;
    std::string responseBody;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");

    if (!body.empty())
        headers = curl_slist_append(headers, "Content-Type: application/json");

    if (!token.empty())
    {
        const std::string h = "Authorization: Bearer " + token;
        headers = curl_slist_append(headers, h.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (!body.empty())
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    std::cerr << method << " " << url << "\n";
    if (!body.empty())
        std::cerr << "Request body: " << body << "\n";

    const CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        response.error = curl_easy_strerror(result);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return response;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (!responseBody.empty())
    {
        try
        {
            response.data = nlohmann::json::parse(responseBody);
        }
        catch (const nlohmann::json::parse_error&)
        {
            response.error = "Server returned invalid JSON";
            return response;
        }
    }
    else
    {
        response.data = nlohmann::json::object();
    }

    response.success =
        response.statusCode >= 200 && response.statusCode < 300;

    if (!response.success)
    {
        if (response.data.is_object() &&
            response.data.contains("detail") &&
            response.data["detail"].is_string())
        {
            response.error = response.data["detail"].get<std::string>();
        }
        else
        {
            response.error = "HTTP error " +
                std::to_string(response.statusCode);
        }
    }

    return response;
}

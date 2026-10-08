#pragma once
#include "StockPriceChart.h"
#include <nlohmann/json.hpp>
#include <string>

class ApiClient;
class AuthManager;

class MainWindow
{
public:
    MainWindow(ApiClient& api, AuthManager& auth);
    void Render();

private:
    void Refresh();
    void Logout();

    ApiClient& api;
    AuthManager& auth;
    nlohmann::json stockData;
    char symbol[32] = "AAPL";
    bool loading = false;
    std::string error;
    StockPriceChart chart;
};

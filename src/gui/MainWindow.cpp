#include "MainWindow.h"
#include "api/ApiClient.h"
#include "auth/AuthManager.h"

#include <imgui.h>
#include <algorithm>
#include <cctype>

MainWindow::MainWindow(ApiClient& api, AuthManager& auth)
    : api(api), auth(auth)
{
    Refresh();
}

void MainWindow::Refresh()
{
    loading = true;
    error.clear();

    std::string requested(symbol);
    std::transform(
        requested.begin(),
        requested.end(),
        requested.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });

    const auto response =
        api.Get("/stocks/" + requested + "/prices");

    loading = false;

    if (!response.success)
    {
        error = response.error.empty()
            ? "Failed to load stock data."
            : response.error;
        stockData = nlohmann::json::object();
        return;
    }

    if (!response.data.is_object() ||
        !response.data.contains("prices") ||
        !response.data["prices"].is_array())
    {
        error = "API returned an invalid stock-data response.";
        stockData = nlohmann::json::object();
        return;
    }

    stockData = response.data;
}

void MainWindow::Logout()
{
    auth.Logout();
    stockData = nlohmann::json::object();
}

void MainWindow::Render()
{
    ImGui::Begin("MyImGuiApp - Stock Dashboard");

    ImGui::Text("Welcome, %s", auth.GetUsername().c_str());
    ImGui::SameLine();

    if (ImGui::Button("Logout"))
        Logout();

    ImGui::Separator();

    ImGui::Text("Stock symbol");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);

    const bool enterPressed = ImGui::InputText(
        "##symbol",
        symbol,
        sizeof(symbol),
        ImGuiInputTextFlags_CharsUppercase |
        ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();

    if (ImGui::Button("Refresh") || enterPressed)
        Refresh();

    ImGui::SameLine();

    if (loading)
        ImGui::TextDisabled("Loading...");

    if (!error.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
            "%s",
            error.c_str());
        ImGui::Spacing();
    }

    if (!loading && error.empty())
    {
        std::string displayedSymbol = "UNKNOWN";
        if (stockData.contains("symbol") &&
            stockData["symbol"].is_string())
        {
            displayedSymbol =
                stockData["symbol"].get<std::string>();
        }

        size_t count = 0;
        if (stockData.contains("prices") &&
            stockData["prices"].is_array())
        {
            count = stockData["prices"].size();
        }

        ImGui::Text(
            "%s | %zu candles | demo OHLC data",
            displayedSymbol.c_str(),
            count);

        ImGui::Spacing();
        chart.Render(stockData);
    }

    ImGui::End();
}

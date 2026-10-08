#include "StockPriceChart.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
struct Candle
{
    std::string timestamp;
    double open = 0;
    double high = 0;
    double low = 0;
    double close = 0;
};

bool Number(const nlohmann::json& j, const char* key, double& value)
{
    if (!j.is_object() || !j.contains(key) || !j[key].is_number())
        return false;
    value = j[key].get<double>();
    return std::isfinite(value);
}
}

void StockPriceChart::Render(const nlohmann::json& data)
{
    ImGui::BeginChild(
        "StockChart",
        ImVec2(0, 520),
        true,
        ImGuiWindowFlags_NoScrollbar);

    if (!data.is_object() ||
        !data.contains("prices") ||
        !data["prices"].is_array())
    {
        ImGui::TextDisabled("No stock data available.");
        ImGui::EndChild();
        return;
    }

    std::vector<Candle> candles;

    for (const auto& item : data["prices"])
    {
        if (!item.is_object() ||
            !item.contains("timestamp") ||
            !item["timestamp"].is_string())
            continue;

        Candle c;
        c.timestamp = item["timestamp"].get<std::string>();

        if (!Number(item, "open", c.open) ||
            !Number(item, "high", c.high) ||
            !Number(item, "low", c.low) ||
            !Number(item, "close", c.close))
            continue;

        if (c.high < c.low)
            std::swap(c.high, c.low);

        candles.push_back(c);
    }

    if (candles.empty())
    {
        ImGui::TextDisabled("No valid OHLC candles were returned.");
        ImGui::EndChild();
        return;
    }

    const float width =
        std::max(800.0f, ImGui::GetContentRegionAvail().x);
    const float height = 470.0f;

    const float left = 70.0f;
    const float right = 25.0f;
    const float top = 30.0f;
    const float bottom = 55.0f;

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 plotMin(
        origin.x + left,
        origin.y + top);
    const ImVec2 plotMax(
        origin.x + width - right,
        origin.y + height - bottom);

    const float plotWidth = plotMax.x - plotMin.x;
    const float plotHeight = plotMax.y - plotMin.y;

    double minPrice = candles.front().low;
    double maxPrice = candles.front().high;

    for (const auto& c : candles)
    {
        minPrice = std::min(minPrice, c.low);
        maxPrice = std::max(maxPrice, c.high);
    }

    if (std::abs(maxPrice - minPrice) < 1e-9)
    {
        minPrice -= 1;
        maxPrice += 1;
    }

    const double padding = (maxPrice - minPrice) * 0.08;
    minPrice -= padding;
    maxPrice += padding;

    auto priceY = [&](double price)
    {
        const double normalized =
            (price - minPrice) / (maxPrice - minPrice);
        return plotMax.y -
            static_cast<float>(normalized * plotHeight);
    };

    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw->AddRectFilled(
        plotMin,
        plotMax,
        IM_COL32(17, 22, 29, 255));

    constexpr int gridLines = 6;

    for (int i = 0; i <= gridLines; ++i)
    {
        const float y =
            plotMin.y + plotHeight *
            static_cast<float>(i) / gridLines;

        draw->AddLine(
            ImVec2(plotMin.x, y),
            ImVec2(plotMax.x, y),
            IM_COL32(55, 65, 78, 150));

        const double price =
            maxPrice -
            (maxPrice - minPrice) * i / gridLines;

        char label[64];
        std::snprintf(label, sizeof(label), "%.2f", price);

        draw->AddText(
            ImVec2(origin.x + 8, y - 8),
            IM_COL32(200, 205, 215, 255),
            label);
    }

    const float slot =
        plotWidth / static_cast<float>(candles.size());

    const float candleWidth =
        std::max(3.0f, std::min(22.0f, slot * 0.62f));

    for (size_t i = 0; i < candles.size(); ++i)
    {
        const Candle& c = candles[i];
        const float x =
            plotMin.x + slot * (static_cast<float>(i) + 0.5f);

        const float yHigh = priceY(c.high);
        const float yLow = priceY(c.low);
        const float yOpen = priceY(c.open);
        const float yClose = priceY(c.close);

        const bool up = c.close >= c.open;
        const ImU32 color = up
            ? IM_COL32(80, 210, 140, 255)
            : IM_COL32(235, 90, 100, 255);

        draw->AddLine(
            ImVec2(x, yHigh),
            ImVec2(x, yLow),
            color,
            1.5f);

        const float topBody = std::min(yOpen, yClose);
        const float bottomBody = std::max(yOpen, yClose);

        draw->AddRectFilled(
            ImVec2(x - candleWidth * 0.5f, topBody),
            ImVec2(
                x + candleWidth * 0.5f,
                std::max(topBody + 2.0f, bottomBody)),
            color);
    }

    draw->AddRect(
        plotMin,
        plotMax,
        IM_COL32(100, 110, 125, 255));

    const size_t every =
        std::max<size_t>(1, candles.size() / 6);

    for (size_t i = 0; i < candles.size(); i += every)
    {
        const float x =
            plotMin.x + slot * (static_cast<float>(i) + 0.5f);

        std::string label = candles[i].timestamp;
        if (label.size() >= 16)
            label = label.substr(11, 5);

        draw->AddText(
            ImVec2(x - 18, plotMax.y + 10),
            IM_COL32(190, 195, 205, 255),
            label.c_str());
    }

    std::string symbol = "UNKNOWN";
    if (data.contains("symbol") && data["symbol"].is_string())
        symbol = data["symbol"].get<std::string>();

    draw->AddText(
        ImVec2(plotMin.x, origin.y + 5),
        IM_COL32(235, 235, 240, 255),
        symbol.c_str());

    ImGui::Dummy(ImVec2(width, height));
    ImGui::EndChild();
}

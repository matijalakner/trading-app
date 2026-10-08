#pragma once
#include <nlohmann/json.hpp>

class StockPriceChart
{
public:
    void Render(const nlohmann::json& data);
};

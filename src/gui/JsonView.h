#pragma once
#include <nlohmann/json.hpp>

class JsonView
{
public:
    static void Render(const nlohmann::json& value);
};

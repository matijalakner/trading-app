#include "JsonView.h"
#include <imgui.h>

void JsonView::Render(const nlohmann::json& value)
{
    const std::string text = value.dump(2);
    ImGui::BeginChild("JsonView", ImVec2(0, 250), true);
    ImGui::TextUnformatted(text.c_str());
    ImGui::EndChild();
}

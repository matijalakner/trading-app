#include "LoginWindow.h"
#include "auth/AuthManager.h"
#include <imgui.h>
#include <cstring>

LoginWindow::LoginWindow(AuthManager& auth) : auth(auth)
{
    std::strncpy(username, "admin", sizeof(username) - 1);
}

void LoginWindow::Render()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(
        viewport->GetCenter(),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.5f));

    ImGui::SetNextWindowSize(
        ImVec2(430.0f, 390.0f),
        ImGuiCond_Always);

    ImGui::Begin(
        "MyImGuiApp",
        nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse);

    ImGui::Spacing();
    ImGui::Text("Stock Dashboard");
    ImGui::TextDisabled("Sign in to continue");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Username");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##username", username, sizeof(username));

    ImGui::Spacing();
    ImGui::Text("Password");
    ImGui::SetNextItemWidth(-1.0f);

    const ImGuiInputTextFlags flags =
        (showPassword ? 0 : ImGuiInputTextFlags_Password) |
        ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputText(
        "##password", password, sizeof(password), flags))
    {
        loginFailed = !auth.Login(username, password);
    }

    ImGui::Checkbox("Show password", &showPassword);
    ImGui::Checkbox("Remember me", &rememberMe);

    ImGui::Spacing();

    if (ImGui::Button("Login", ImVec2(-1.0f, 42.0f)))
        loginFailed = !auth.Login(username, password);

    if (loginFailed)
    {
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
            "Login failed. Check your username and password.");
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Development account: admin / password");

    ImGui::End();
}

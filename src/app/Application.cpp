#include "Application.h"
#include "api/ApiClient.h"
#include "auth/AuthManager.h"
#include "gui/LoginWindow.h"
#include "gui/MainWindow.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/gl.h>
#include <iostream>

Application::Application() = default;

Application::~Application()
{
    Shutdown();
}

bool Application::Initialize()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(
        1280, 800, "MyImGuiApp - Stock Dashboard", nullptr, nullptr);

    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
        return false;
    if (!ImGui_ImplOpenGL3_Init("#version 330"))
        return false;

    apiClient = std::make_unique<ApiClient>("http://127.0.0.1:8000");
    authManager = std::make_unique<AuthManager>(*apiClient);
    loginWindow = std::make_unique<LoginWindow>(*authManager);
    mainWindow = std::make_unique<MainWindow>(*apiClient, *authManager);

    return true;
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (authManager->IsAuthenticated())
            mainWindow->Render();
        else
            loginWindow->Render();

        ImGui::Render();

        int w = 0;
        int h = 0;
        glfwGetFramebufferSize(window, &w, &h);

        glViewport(0, 0, w, h);
        glClearColor(0.055f, 0.065f, 0.080f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void Application::Shutdown()
{
    if (!window)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}

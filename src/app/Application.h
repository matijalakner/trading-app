#pragma once
#include <memory>
#include <GLFW/glfw3.h>

class ApiClient;
class AuthManager;
class LoginWindow;
class MainWindow;

class Application
{
public:
    Application();
    ~Application();
    bool Initialize();
    void Run();

private:
    void Shutdown();
    GLFWwindow* window = nullptr;
    std::unique_ptr<ApiClient> apiClient;
    std::unique_ptr<AuthManager> authManager;
    std::unique_ptr<LoginWindow> loginWindow;
    std::unique_ptr<MainWindow> mainWindow;
};

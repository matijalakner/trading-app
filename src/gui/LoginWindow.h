#pragma once
class AuthManager;

class LoginWindow
{
public:
    explicit LoginWindow(AuthManager& auth);
    void Render();

private:
    AuthManager& auth;
    char username[128]{};
    char password[128]{};
    bool rememberMe = false;
    bool showPassword = false;
    bool loginFailed = false;
};

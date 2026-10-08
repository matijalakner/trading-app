cmake_minimum_required(VERSION 3.20)
project(MyImGuiApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(CURL REQUIRED)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp")
    message(FATAL_ERROR "Dear ImGui missing. Run scripts/setup_dependencies.sh")
endif()
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/json/include/nlohmann/json.hpp")
    message(FATAL_ERROR "nlohmann/json missing. Run scripts/setup_dependencies.sh")
endif()

add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_executable(MyImGuiApp
    src/main.cpp
    src/app/Application.cpp
    src/api/ApiClient.cpp
    src/auth/AuthManager.cpp
    src/gui/LoginWindow.cpp
    src/gui/MainWindow.cpp
    src/gui/JsonView.cpp
    src/gui/StockPriceChart.cpp
)
target_include_directories(MyImGuiApp PRIVATE src external/json/include)
target_link_libraries(MyImGuiApp PRIVATE imgui CURL::libcurl)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(MyImGuiApp PRIVATE -Wall -Wextra -Wpedantic)
endif()

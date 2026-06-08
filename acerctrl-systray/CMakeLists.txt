set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
add_executable(acerctrl-systray main.cpp rgb-window.cpp hw-settings.cpp)
target_link_libraries(acerctrl-systray PRIVATE Qt6::Widgets)

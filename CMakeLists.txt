cmake_minimum_required(VERSION 3.28)
project(acerctrl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libacerhidhardware)
add_subdirectory(libacerhidrgb)
add_subdirectory(acerctrld)
add_subdirectory(acerctrl-cli)
add_subdirectory(acerctrl-systray)

install(TARGETS acerctrld acerctrl-cli acerctrl-systray DESTINATION bin)
install(TARGETS AcerHIDHardware AcerHIDRGB LIBRARY DESTINATION lib)

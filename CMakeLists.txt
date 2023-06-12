cmake_minimum_required(VERSION 3.15)

project(wkwebviewtest
    LANGUAGES CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(
    WEBKIT2GTK
    REQUIRED
    IMPORTED_TARGET webkit2gtk-4.1
)

add_executable(wkwebviewtest main.cpp)
target_link_libraries(wkwebviewtest PRIVATE PkgConfig::WEBKIT2GTK)

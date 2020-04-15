cmake_minimum_required(VERSION 3.16)

project(WallpaperService)

set(CMAKE_CXX_STANDARD 17)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES
        cli11/1.3.0@bincrafters/stable
        restinio/0.6.5@stiffstream/stable
        BASIC_SETUP
        CMAKE_TARGETS
        BUILD missing
)

add_subdirectory(src)

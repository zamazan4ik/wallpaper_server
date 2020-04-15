set(SRC
        main.cpp
)

add_executable(WallpaperService ${SRC})
target_link_libraries(WallpaperService
    PRIVATE
        CONAN_PKG::cli11
        CONAN_PKG::restinio
)

install(TARGETS WallpaperService RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/logs")

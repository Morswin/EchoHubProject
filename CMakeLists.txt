cmake_minimum_required(VERSION 4.2)
project(EchoHubProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# SDL
add_subdirectory(external/SDL)

# ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

# Asio
FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-30-2
)
FetchContent_MakeAvailable(asio)
FetchContent_GetProperties(asio SOURCE_DIR ASIO_SOURCE_DIR)
add_library(asio_lib INTERFACE)
add_library(asio::asio ALIAS asio_lib)
target_include_directories(asio_lib INTERFACE ${ASIO_SOURCE_DIR}/asio/include)
target_compile_definitions(asio_lib INTERFACE ASIO_STANDALONE)

# Opus
FetchContent_Declare(
        opus
        GIT_REPOSITORY https://github.com/xiph/opus.git
        GIT_TAG v1.5.2
)
set(OPUS_BUILD_PROGRAMS OFF CACHE INTERNAL "")
set(OPUS_BUILD_TESTING OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(opus)

# Our executable
add_executable(echo_hub
        src/main.cpp
        src/message.cpp
        src/users.cpp
)
target_include_directories(echo_hub PRIVATE
        src
)

# Linking
target_link_libraries(imgui PRIVATE SDL3::SDL3-shared)
find_package(Threads REQUIRED)
target_link_libraries(echo_hub PRIVATE
        imgui
        SDL3::SDL3-shared
        asio::asio
        opus
        Threads::Threads
)

# Copying dynamic libraries
if(WIN32)
    add_custom_command(TARGET echo_hub POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:echo_hub>
    )
endif()


add_custom_command(TARGET echo_hub POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:echo_hub>/assets/fonts

        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/assets/fonts/Roboto-Bold.ttf
        $<TARGET_FILE_DIR:echo_hub>/assets/fonts/

        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/assets/fonts/Roboto-Black.ttf
        $<TARGET_FILE_DIR:echo_hub>/assets/fonts/

        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:echo_hub>/data/

        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/data/users.txt
        $<TARGET_FILE_DIR:echo_hub>/data/
)

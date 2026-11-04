cmake_minimum_required(VERSION 3.21)
project(SoldierSkirmish VERSION 0.1.0 LANGUAGES CXX)
find_package(SDL2 2.0.18 CONFIG REQUIRED)
find_package(SDL2_image 2.0 CONFIG REQUIRED)
find_package(SDL2_ttf 2.0 CONFIG REQUIRED)
find_package(Threads REQUIRED)
add_executable(soldier-skirmish
    src/main.cpp src/runtimeAssets.cpp src/attackBehavior.cpp src/base.cpp
    src/chessboard.cpp src/controller.cpp src/model.cpp src/soldier.cpp
    src/soldier_move.cpp src/view.cpp src/vision.cpp)
target_compile_features(soldier-skirmish PRIVATE cxx_std_17)
set_target_properties(soldier-skirmish PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(soldier-skirmish PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(soldier-skirmish PRIVATE
    SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf Threads::Threads)
if(MSVC)
    target_compile_options(soldier-skirmish PRIVATE /W4 /utf-8)
else()
    target_compile_options(soldier-skirmish PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_custom_target(copy-assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:soldier-skirmish>/assets")
add_dependencies(soldier-skirmish copy-assets)
if(WIN32)
    add_custom_command(TARGET soldier-skirmish POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:soldier-skirmish> $<TARGET_FILE_DIR:soldier-skirmish>
        COMMAND_EXPAND_LISTS)
endif()
include(CTest)
if(BUILD_TESTING)
    add_test(NAME assets COMMAND soldier-skirmish --check-assets)
    add_test(NAME startup COMMAND soldier-skirmish --smoke-test)
    set_tests_properties(startup PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy" TIMEOUT 15)
endif()

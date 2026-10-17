cmake_minimum_required(VERSION 3.16)
project(ImpulseTrackerModern VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(IT_CONSOLE "Build with a console window on Windows (for debugging / CLI rendering)" OFF)
option(IT_FETCH_SDL "Download and build SDL3 if it is not installed" ON)

# ---------------------------------------------------------------- SDL3
find_package(SDL3 CONFIG QUIET)
if(NOT SDL3_FOUND)
  if(IT_FETCH_SDL)
    message(STATUS "SDL3 not found - fetching it from GitHub")
    include(FetchContent)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL3
      GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
      GIT_TAG release-3.2.24
      GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(SDL3)
  else()
    message(FATAL_ERROR "SDL3 is required")
  endif()
endif()

# ---------------------------------------------------------------- embedded assets
include(${CMAKE_SOURCE_DIR}/cmake/embed.cmake)
set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)
embed_file(${CMAKE_SOURCE_DIR}/assets/fonts/JetBrainsMono-Regular.ttf font_regular ${GEN_DIR}/font_regular.c)
embed_file(${CMAKE_SOURCE_DIR}/assets/fonts/JetBrainsMono-Bold.ttf    font_bold    ${GEN_DIR}/font_bold.c)

# ---------------------------------------------------------------- sources
set(IT_SOURCES
  src/main.c
  src/song.c
  src/util.c
  src/fmt_it.c
  src/fmt_s3m.c
  src/fmt_mod.c
  src/fmt_xm.c
  src/fmt_wav.c
  src/player.c
  src/mixer.c
  src/audio.c
  src/config.c
  src/gfx.c
  src/ui.c
  src/widgets.c
  src/dialog.c
  src/help_text.c
  src/page_help.c
  src/page_pattern.c
  src/page_orders.c
  src/page_samples.c
  src/page_instruments.c
  src/page_info.c
  src/page_files.c
  src/page_songvars.c
  src/page_settings.c
  src/midi.c
  src/midi_profiles.c
  src/midi_learn.c
  src/page_message.c
  src/page_wizard.c
  src/demo_songs.c
  src/export.c
  ${GEN_DIR}/font_regular.c
  ${GEN_DIR}/font_bold.c
)

if(WIN32)
  list(APPEND IT_SOURCES res/impulse.rc)
endif()

if(WIN32 AND NOT IT_CONSOLE)
  add_executable(impulse-tracker WIN32 ${IT_SOURCES})
elseif(APPLE)
  add_executable(impulse-tracker MACOSX_BUNDLE ${IT_SOURCES})
  set_target_properties(impulse-tracker PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Impulse Tracker"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.impulsetracker.modern"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/res/Info.plist.in)
else()
  add_executable(impulse-tracker ${IT_SOURCES})
endif()

target_include_directories(impulse-tracker PRIVATE src third_party/stb)
target_compile_definitions(impulse-tracker PRIVATE IT_VERSION_STR="${PROJECT_VERSION}")
target_link_libraries(impulse-tracker PRIVATE SDL3::SDL3)

if(WIN32)
  target_link_libraries(impulse-tracker PRIVATE winmm)
endif()

if(MSVC)
  target_compile_options(impulse-tracker PRIVATE /W3 /D_CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(impulse-tracker PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -fno-lto)
  if(NOT APPLE)
    target_link_libraries(impulse-tracker PRIVATE m)
  endif()
  if(MINGW)
    target_link_options(impulse-tracker PRIVATE -static-libgcc -fno-lto)
  endif()
endif()

# copy SDL3 runtime next to the executable on Windows
if(WIN32 AND TARGET SDL3::SDL3-shared)
  add_custom_command(TARGET impulse-tracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:impulse-tracker>)
endif()

install(TARGETS impulse-tracker RUNTIME DESTINATION bin BUNDLE DESTINATION .)

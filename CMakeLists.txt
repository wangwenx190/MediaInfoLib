cmake_minimum_required(VERSION 4.0)
project(MediaInfoLib
    VERSION 26.5.0
    DESCRIPTION "Convenient unified display of the most relevant technical and tag data for video and audio files"
    LANGUAGES C CXX
)
if(WIN32)
    enable_language(RC)
endif()
if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release")
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
foreach(__lang C CXX)
    if(NOT "x${CMAKE_${__lang}_FLAGS}" STREQUAL "x")
        if(MSVC)
            string(REGEX REPLACE " [/-]W[01234] " " " CMAKE_${__lang}_FLAGS ${CMAKE_${__lang}_FLAGS})
        else()
            string(REGEX REPLACE " -W(all)?(extra)? " " " CMAKE_${__lang}_FLAGS ${CMAKE_${__lang}_FLAGS})
            string(REGEX REPLACE " -[W]?pedantic " " " CMAKE_${__lang}_FLAGS ${CMAKE_${__lang}_FLAGS})
        endif()
    endif()
    string(APPEND CMAKE_${__lang}_FLAGS " -w ")
endforeach()
set(__top_level ON)
if("x${CMAKE_SOURCE_DIR}" STREQUAL "x${PROJECT_SOURCE_DIR}")
    message("MediaInfoLib: Building as a top-level project.")
else()
    set(__top_level OFF)
    message("MediaInfoLib: Building as a subproject.")
endif()
option(MEDIAINFOLIB_BUILD_SHARED "Build shared library edition of MediaInfoLib." ON)
option(MEDIAINFOLIB_BUILD_STATIC "Build static library edition of MediaInfoLib." ON)
option(MEDIAINFOLIB_INSTALL "Enable installation for MediaInfoLib." OFF)
option(MEDIAINFOLIB_USE_BUNDLED_ZLIB "Use bundled zlib." ON)
if(NOT MEDIAINFOLIB_BUILD_SHARED AND NOT MEDIAINFOLIB_BUILD_STATIC)
    message(SEND_ERROR "You MUST build either the shared library or the static library or both of them!")
endif()
set(__targets)
if(MEDIAINFOLIB_BUILD_SHARED)
    add_library(${PROJECT_NAME}_shared SHARED)
    add_library(${PROJECT_NAME}::${PROJECT_NAME}_shared ALIAS ${PROJECT_NAME}_shared)
    set(__dllname ${PROJECT_NAME})
    if(NOT MSVC)
        set(__dllname lib${PROJECT_NAME})
    endif()
    set_target_properties(${PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME ${__dllname})
    target_compile_definitions(${PROJECT_NAME}_shared PRIVATE
        MEDIAINFOLIBLITE_BUILD_LIBRARY
    )
    if(WIN32)
        target_sources(${PROJECT_NAME}_shared PRIVATE mediainfolib.rc)
    endif()
    list(APPEND __targets ${PROJECT_NAME}_shared)
endif()
if(MEDIAINFOLIB_BUILD_STATIC)
    add_library(${PROJECT_NAME}_static STATIC)
    add_library(${PROJECT_NAME}::${PROJECT_NAME}_static ALIAS ${PROJECT_NAME}_static)
    target_compile_definitions(${PROJECT_NAME}_static PUBLIC
        MEDIAINFOLIBLITE_BUILD_STATIC
    )
    list(APPEND __targets ${PROJECT_NAME}_static)
endif()
if(MEDIAINFOLIB_BUILD_SHARED AND NOT MEDIAINFOLIB_BUILD_STATIC)
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_shared)
endif()
if(MEDIAINFOLIB_BUILD_STATIC AND NOT MEDIAINFOLIB_BUILD_SHARED)
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_static)
endif()
if(MEDIAINFOLIB_USE_BUNDLED_ZLIB)
    file(GLOB __zlib_sources LIST_DIRECTORIES false CONFIGURE_DEPENDS
        "./zlib_src/*.h" "./zlib_src/*.c"
    )
    add_library(${PROJECT_NAME}_zlib STATIC ${__zlib_sources})
    target_include_directories(${PROJECT_NAME}_zlib PUBLIC ./zlib_src)
else()
    add_library(${PROJECT_NAME}_zlib INTERFACE)
    target_include_directories(${PROJECT_NAME}_zlib INTERFACE ./zlib_hdr)
endif()
include(GNUInstallDirs)
file(GLOB_RECURSE __sources LIST_DIRECTORIES false CONFIGURE_DEPENDS
    "./Source/*.h" "./Source/*.hh" "./Source/*.hpp" "./Source/*.hxx"
    "./Source/*.cpp" "./Source/*.cxx"
    "./Source/*.c" "./Source/*.cc"
)
foreach(__target IN LISTS __targets)
    target_sources(${__target} PRIVATE ${__sources})
    target_include_directories(${__target} PUBLIC
        ./Source
        ./Source/MediaInfo
    )
    target_include_directories(${__target} PRIVATE
        ./Source/MediaInfo/ZenLib
        ./Source/MediaInfo/ZenLib/Format
        ./Source/MediaInfo/ZenLib/HTTP_Client
        ./Source/MediaInfo/ThirdParty
        ./Source/MediaInfo/ThirdParty/aes-gladman
        ./Source/MediaInfo/ThirdParty/base64
        ./Source/MediaInfo/ThirdParty/fmt
        ./Source/MediaInfo/ThirdParty/hmac-gladman
        ./Source/MediaInfo/ThirdParty/jni
        ./Source/MediaInfo/ThirdParty/md5
        ./Source/MediaInfo/ThirdParty/sha1-gladman
        ./Source/MediaInfo/ThirdParty/sha2-gladman
        ./Source/MediaInfo/ThirdParty/tfsxml
        ./Source/MediaInfo/ThirdParty/tinyxml2
    )
    target_compile_definitions(${__target} PRIVATE
        MEDIAINFO_LIBMMS_NO
        MEDIAINFO_LIBCURL_NO
        MEDIAINFO_GRAPHVIZ_NO
    )
    if(__top_level)
        if(WIN32)
            target_compile_definitions(${__target} PRIVATE
                UNICODE _UNICODE
                NOMINMAX
                _USE_MATH_DEFINES
                WIN32_LEAN_AND_MEAN
            )
        endif()
        if(MSVC)
            target_compile_options(${__target} PRIVATE
                /utf-8 /Zc:__cplusplus
                $<$<NOT:$<CONFIG:Debug>>:/Gw /Gy /Zc:inline>
            )
            target_link_options(${__target} PRIVATE
                $<$<NOT:$<CONFIG:Debug>>:/OPT:REF /OPT:ICF>
            )
        else()
            target_compile_options(${__target} PRIVATE
                $<$<NOT:$<CONFIG:Debug>>:-ffunction-sections -fdata-sections>
            )
            target_link_options(${__target} PRIVATE
                $<$<NOT:$<CONFIG:Debug>>:-Wl,--gc-sections>
            )
        endif()
    endif()
    set_target_properties(${__target} PROPERTIES
        VERSION ${PROJECT_VERSION}
    )
    set(__dep_visibility PRIVATE)
    get_target_property(__type ${__target} TYPE)
    if("x${__type}" STREQUAL "xSTATIC_LIBRARY")
        set(__dep_visibility PUBLIC)
    else()
        set_target_properties(${__target} PROPERTIES
            SOVERSION ${PROJECT_VERSION_MAJOR}
        )
    endif()
    target_link_libraries(${__target} ${__dep_visibility}
        ${PROJECT_NAME}_zlib
    )
    if(WIN32)
        target_link_libraries(${__target} ${__dep_visibility}
            ws2_32
        )
    endif()
    if(MEDIAINFOLIB_INSTALL)
        install(TARGETS ${__target}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
        install(
            FILES
                ./Source/MediaInfo/MediaInfoLibLite.h
                ./Source/MediaInfo/MediaInfoLibLite_global.h
                ./Source/MediaInfo/File__Analyse_Automatic.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
endforeach()

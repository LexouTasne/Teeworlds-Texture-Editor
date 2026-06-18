cmake_minimum_required(VERSION 3.20)

project(TeeworldsTextureEditor VERSION 0.1.0 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TTE_AUTHOR "Lex")
set(TTE_COPYRIGHT "Lex copyright 2026")
set(TTE_COMPANY_NAME "Lex")
set(TTE_PRODUCT_NAME "Teeworlds Texture Editor")

set(TTE_ICON_PATH "")
if(EXISTS "${CMAKE_SOURCE_DIR}/app.ico")
    set(TTE_ICON_PATH "${CMAKE_SOURCE_DIR}/app.ico")
else()
    file(GLOB TTE_ICON_CANDIDATES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/assets/icons/*.ico"
        "${CMAKE_SOURCE_DIR}/*.ico"
    )
    if(TTE_ICON_CANDIDATES)
        list(GET TTE_ICON_CANDIDATES 0 TTE_ICON_PATH)
    endif()
endif()

set(TTE_ICON_RESOURCE_LINE "")
if(TTE_ICON_PATH)
    file(TO_CMAKE_PATH "${TTE_ICON_PATH}" TTE_ICON_RESOURCE_PATH)
    set(TTE_ICON_RESOURCE_LINE "IDI_APP_ICON ICON \"${TTE_ICON_RESOURCE_PATH}\"")
    message(STATUS "Using Windows icon: ${TTE_ICON_PATH}")
else()
    message(STATUS "No .ico found; build will use the default executable icon.")
endif()

set(TTE_RESOURCE_FILE "")
if(WIN32)
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")
    configure_file(cmake/windows_app.rc.in "${CMAKE_BINARY_DIR}/generated/tte.rc" @ONLY)
    set(TTE_RESOURCE_FILE "${CMAKE_BINARY_DIR}/generated/tte.rc")
endif()

if(WIN32)
    add_executable(tte WIN32 src/main.cpp ${TTE_RESOURCE_FILE})
else()
    add_executable(tte src/main.cpp ${TTE_RESOURCE_FILE})
endif()

target_compile_definitions(tte PRIVATE
    TTE_VERSION="${PROJECT_VERSION}"
    TTE_AUTHOR="${TTE_AUTHOR}"
    TTE_COPYRIGHT="${TTE_COPYRIGHT}"
)

if(WIN32)
    target_link_libraries(tte PRIVATE gdiplus comdlg32 winmm)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(tte PRIVATE -municode)
    endif()
endif()

function(tte_copy_if_exists dll_name)
    string(MAKE_C_IDENTIFIER "${dll_name}" dll_var)
    find_file(TTE_${dll_var}_PATH
        NAMES "${dll_name}"
        PATHS
            "${CMAKE_CXX_COMPILER}/../"
            "C:/msys64/ucrt64/bin"
            "C:/msys64/mingw64/bin"
        NO_DEFAULT_PATH
    )
    if(TTE_${dll_var}_PATH)
        add_custom_command(TARGET tte POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${TTE_${dll_var}_PATH}"
                "$<TARGET_FILE_DIR:tte>/${dll_name}"
            COMMENT "Copying runtime DLL ${dll_name}"
        )
    endif()
endfunction()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    tte_copy_if_exists("libgcc_s_seh-1.dll")
    tte_copy_if_exists("libstdc++-6.dll")
    tte_copy_if_exists("libwinpthread-1.dll")
endif()

set(TTE_PACKAGE_DIR "${CMAKE_BINARY_DIR}/dist/${TTE_PRODUCT_NAME}-${PROJECT_VERSION}")
string(REPLACE " " "" TTE_PACKAGE_DIR "${TTE_PACKAGE_DIR}")

add_custom_target(package-local
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${TTE_PACKAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${TTE_PACKAGE_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:tte>" "${TTE_PACKAGE_DIR}/tte.exe"
    COMMAND "${CMAKE_COMMAND}" -DINPUT_DIR="$<TARGET_FILE_DIR:tte>" -DOUTPUT_DIR="${TTE_PACKAGE_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/copy_runtime_dlls.cmake"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/data" "${TTE_PACKAGE_DIR}/data"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/icons" "${TTE_PACKAGE_DIR}/icons"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${TTE_PACKAGE_DIR}/IA-TRAIN"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/IA-TRAIN/README.md" "${TTE_PACKAGE_DIR}/IA-TRAIN/README.md"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/scripts" "${TTE_PACKAGE_DIR}/scripts"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/requirements.txt" "${TTE_PACKAGE_DIR}/requirements.txt"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/README.md" "${TTE_PACKAGE_DIR}/README.md"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/LICENSE" "${TTE_PACKAGE_DIR}/LICENSE"
    DEPENDS tte
    COMMENT "Creating local runnable package at ${TTE_PACKAGE_DIR}"
)

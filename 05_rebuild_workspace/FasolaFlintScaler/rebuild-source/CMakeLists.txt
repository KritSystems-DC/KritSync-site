cmake_minimum_required(VERSION 3.24)

project(FasolaFlintScaler VERSION 1.0.0 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Krit-Faslo must be built as a 64-bit DLL.")
endif()

if(NOT DEFINED ENV{ASA_API_ROOT})
    message(FATAL_ERROR "Set ASA_API_ROOT to the ArkServerAPI / ASA API folder before configuring.")
endif()

set(ASA_API_ROOT "$ENV{ASA_API_ROOT}" CACHE PATH "ArkServerAPI / ASA API root")
if(DEFINED ENV{ASA_API_LIB_ROOT})
    set(ASA_API_LIB_ROOT "$ENV{ASA_API_LIB_ROOT}" CACHE PATH "ArkServerAPI / ASA API library/runtime root")
else()
    set(ASA_API_LIB_ROOT "${ASA_API_ROOT}" CACHE PATH "ArkServerAPI / ASA API library/runtime root")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(AsaApi REQUIRED)

add_library(FasolaFlintScaler SHARED
    src/ArkApiBridge.cpp
    src/Config.cpp
    src/HarvestScaler.cpp
    src/LicenseValidator.cpp
    src/Logger.cpp
    src/PluginMain.cpp
    src/PluginModule.cpp
    src/Version.rc
)

target_include_directories(FasolaFlintScaler
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
        ${ASAAPI_INCLUDE_DIRS}
)

target_link_libraries(FasolaFlintScaler
    PRIVATE
        "${ASAAPI_LIBRARY}"
        bcrypt
        crypt32
)

target_compile_definitions(FasolaFlintScaler
    PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        FASOLA_WITH_ASA_API=1
)

if(MSVC)
    target_compile_options(FasolaFlintScaler PRIVATE /permissive- /W4 /EHsc /Zc:__cplusplus)
    target_link_options(FasolaFlintScaler PRIVATE /NXCOMPAT /DYNAMICBASE)
endif()

set_target_properties(FasolaFlintScaler PROPERTIES
    OUTPUT_NAME "Krit-Faslo"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Krit-Faslo"
)

add_custom_command(TARGET FasolaFlintScaler POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
        "$<TARGET_FILE_DIR:FasolaFlintScaler>/config.json"
)

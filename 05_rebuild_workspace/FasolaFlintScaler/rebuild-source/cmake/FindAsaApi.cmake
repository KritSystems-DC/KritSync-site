find_path(ASAAPI_INCLUDE_DIR
    NAMES AsaApi.h ArkApi.h API/ARK/Ark.h AsaApi/Core/Public/IHooks.h
    PATHS
        "${ASA_API_ROOT}"
        "${ASA_API_ROOT}/include"
        "${ASA_API_ROOT}/ArkApi"
        "${ASA_API_ROOT}/AsaApi"
        "${ASA_API_ROOT}/AsaApi/Core/Public"
        "${ASA_API_ROOT}/AsaApi/Core/Private"
        "${ASA_API_ROOT}/AsaApi/AsaApi/Core/Public"
        "${ASA_API_ROOT}/AsaApi/AsaApi/Core/Private"
        "${ASA_API_ROOT}/AsaApi/AsaApi"
    NO_DEFAULT_PATH
)

set(_ASAAPI_INCLUDE_CANDIDATES
    "${ASA_API_ROOT}"
    "${ASA_API_ROOT}/include"
    "${ASA_API_ROOT}/ArkApi"
    "${ASA_API_ROOT}/AsaApi"
    "${ASA_API_ROOT}/AsaApi/Core/Public"
    "${ASA_API_ROOT}/AsaApi/Core/Private"
    "${ASA_API_ROOT}/AsaApi/AsaApi/Core/Public"
    "${ASA_API_ROOT}/AsaApi/AsaApi/Core/Private"
    "${ASA_API_ROOT}/AsaApi/AsaApi"
)

set(ASAAPI_INCLUDE_DIRS "")
foreach(_ASAAPI_INCLUDE_CANDIDATE IN LISTS _ASAAPI_INCLUDE_CANDIDATES)
    if(EXISTS "${_ASAAPI_INCLUDE_CANDIDATE}")
        list(APPEND ASAAPI_INCLUDE_DIRS "${_ASAAPI_INCLUDE_CANDIDATE}")
    endif()
endforeach()

list(REMOVE_DUPLICATES ASAAPI_INCLUDE_DIRS)

find_library(ASAAPI_LIBRARY
    NAMES AsaApi ArkApi
    PATHS
        "${ASA_API_LIB_ROOT}"
        "${ASA_API_LIB_ROOT}/lib"
        "${ASA_API_LIB_ROOT}/Lib"
        "${ASA_API_LIB_ROOT}/Binaries/Win64"
        "${ASA_API_LIB_ROOT}/ArkApi"
        "${ASA_API_LIB_ROOT}/ArkApi/Lib"
        "${ASA_API_LIB_ROOT}/AsaApi/x64/Release"
        "${ASA_API_LIB_ROOT}/AsaApi/Lib"
        "${ASA_API_LIB_ROOT}/Win64"
        "${ASA_API_ROOT}"
        "${ASA_API_ROOT}/lib"
        "${ASA_API_ROOT}/Lib"
        "${ASA_API_ROOT}/Binaries/Win64"
        "${ASA_API_ROOT}/AsaApi/x64/Release"
        "${ASA_API_ROOT}/AsaApi/Lib"
        "${ASA_API_ROOT}/Win64"
    NO_DEFAULT_PATH
)

find_file(ASAAPI_RUNTIME_DLL
    NAMES AsaApi.dll ArkApi.dll
    PATHS
        "${ASA_API_LIB_ROOT}"
        "${ASA_API_LIB_ROOT}/ArkApi"
        "${ASA_API_LIB_ROOT}/Binaries/Win64"
        "${ASA_API_LIB_ROOT}/Win64"
        "${ASA_API_ROOT}"
        "${ASA_API_ROOT}/ArkApi"
        "${ASA_API_ROOT}/Binaries/Win64"
        "${ASA_API_ROOT}/Win64"
    NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AsaApi
    REQUIRED_VARS ASAAPI_INCLUDE_DIR ASAAPI_INCLUDE_DIRS ASAAPI_LIBRARY
)

if(AsaApi_FOUND)
    message(STATUS "AsaApi headers: ${ASAAPI_INCLUDE_DIR}")
    message(STATUS "AsaApi include dirs: ${ASAAPI_INCLUDE_DIRS}")
    message(STATUS "AsaApi import library: ${ASAAPI_LIBRARY}")
    if(ASAAPI_RUNTIME_DLL)
        message(STATUS "AsaApi runtime DLL: ${ASAAPI_RUNTIME_DLL}")
    else()
        message(WARNING "AsaApi runtime DLL was not found below ASA_API_LIB_ROOT. The DLL is not required to configure, but it is required on the server.")
    endif()
endif()

mark_as_advanced(ASAAPI_INCLUDE_DIR ASAAPI_INCLUDE_DIRS ASAAPI_LIBRARY ASAAPI_RUNTIME_DLL)

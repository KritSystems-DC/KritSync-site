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

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AsaApi
    REQUIRED_VARS ASAAPI_INCLUDE_DIR ASAAPI_LIBRARY
)

mark_as_advanced(ASAAPI_INCLUDE_DIR ASAAPI_LIBRARY)

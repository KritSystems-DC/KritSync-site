find_path(ASAAPI_INCLUDE_DIR
    NAMES AsaApi.h ArkApi.h API/ARK/Ark.h
    PATHS "${ASA_API_ROOT}" "${ASA_API_ROOT}/include" "${ASA_API_ROOT}/ArkApi"
    NO_DEFAULT_PATH
)

find_library(ASAAPI_LIBRARY
    NAMES AsaApi ArkApi
    PATHS "${ASA_API_ROOT}" "${ASA_API_ROOT}/lib" "${ASA_API_ROOT}/Lib" "${ASA_API_ROOT}/Binaries/Win64"
    NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AsaApi
    REQUIRED_VARS ASAAPI_INCLUDE_DIR ASAAPI_LIBRARY
)

mark_as_advanced(ASAAPI_INCLUDE_DIR ASAAPI_LIBRARY)

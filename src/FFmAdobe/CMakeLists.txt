set(LIBRARY_NAME "FFmAdobe")

# ============================
# Source Files
# ============================
set(Source_Files
    "FFmAdobe.cpp"
    "FFmAdobe_Params.cpp"
)

set(Header_Files
    "FFmAdobe.h"
    "FFmAdobe_Params.h"
)

set(Utility_Files
    "${ADOBE_PP_SDK}/Examples/Utils/SDK_File.cpp"
    "${ADOBE_PP_SDK}/Examples/Utils/SDK_File.h"
    "${ADOBE_PP_SDK}/Examples/Utils/SDK_Segment_Utils.cpp"
    "${ADOBE_PP_SDK}/Examples/Utils/SDK_Segment_Utils.h"
)

set(Resource_Files "FFmAdobe.rc")

source_group("Source Files"           FILES ${Source_Files})
source_group("Header Files"           FILES ${Header_Files})
source_group("Adobe SDK Utility Files" FILES ${Utility_Files})
source_group("Resource Files"          FILES ${Resource_Files})

# ============================
# Build shared library (.prm)
# ============================
add_library(${LIBRARY_NAME} SHARED
    ${Source_Files}
    ${Header_Files}
    ${Utility_Files}
    ${Resource_Files}
)

set_target_properties(${LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# ============================
# Include Directories
# ============================
target_include_directories(${LIBRARY_NAME} PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${ADOBE_PP_SDK}/Examples/Utils"
    "${ADOBE_PP_SDK}/Examples/Headers"
)

# ============================
# MSVC link options
# ============================
if (MSVC)
    set_target_properties(${LIBRARY_NAME} PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
    target_link_options(${LIBRARY_NAME} PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
        $<$<CONFIG:Release>:/DEBUG:FULL;/OPT:REF;/OPT:ICF>
        /MACHINE:X64
    )
endif()

# ============================
# Export header
# ============================
include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header(${LIBRARY_NAME} EXPORT_FILE_NAME "${_export_file}")

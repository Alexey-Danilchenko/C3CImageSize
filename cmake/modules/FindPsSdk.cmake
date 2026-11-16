#
# FindPsSdk
# ---------
#
# Find the Adobe Photoshop SDK (plugin API, header only).
#
# The SDK is located by looking for the sample code helper headers,
# e.g. <sdk>/pluginsdk/samplecode/common/includes/PIDefines.h.
#
# Imported Targets
# ----------------
#
# This module provides the following imported target, if found:
#    PsSdk::PsSdk
#
# Result Variables
# ----------------
#
# This will define the following variables:
#
#   PsSdk_FOUND        - True if the system has the Photoshop SDK.
#   PsSdk_INCLUDE_DIRS - Include directories needed to use the Photoshop SDK.
#
# Cache Variables
# ---------------
#
# The following cache variables may also be set:
#
#   PsSdk_INCLUDE_DIR  - The directory containing PIDefines.h.
#
# Copyright (c) 2026 Alexey Danilchenko
#
# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#

find_path(PsSdk_INCLUDE_DIR
    NAME PIDefines.h
    PATH_SUFFIXES pluginsdk/samplecode/common/includes
)

if(PsSdk_INCLUDE_DIR)
    # <sdk root>/pluginsdk/samplecode/common/includes
    get_filename_component(_PsSdk_root "${PsSdk_INCLUDE_DIR}/../../../.." ABSOLUTE)
    set(PsSdk_INCLUDE_DIRS
        "${_PsSdk_root}/pluginsdk/photoshopapi/photoshop"
        "${_PsSdk_root}/pluginsdk/photoshopapi/pica_sp"
        "${PsSdk_INCLUDE_DIR}"
    )
    # Sample sources (e.g. PIUSuites.cpp with the sPS* suite globals)
    set(PsSdk_SAMPLE_SOURCES_DIR "${_PsSdk_root}/pluginsdk/samplecode/common/sources")
endif()

# Handle standard arguments and any REQUIRED findings
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PsSdk
                                  FOUND_VAR PsSdk_FOUND
                                  REQUIRED_VARS PsSdk_INCLUDE_DIR)

# set plural vars and mark as advanced
if(PsSdk_FOUND)
    mark_as_advanced(PsSdk_INCLUDE_DIR
                     PsSdk_INCLUDE_DIRS)
endif()

# Create imported target PsSdk::PsSdk
if(PsSdk_FOUND AND NOT TARGET PsSdk::PsSdk)
    add_library(PsSdk::PsSdk INTERFACE IMPORTED)
    set_target_properties(PsSdk::PsSdk PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES "${PsSdk_INCLUDE_DIRS}"
    )
endif()

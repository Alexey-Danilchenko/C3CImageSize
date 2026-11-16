# MacOS X specific parts of the C3C plugins

# Photoshop plugin bundle setup
macro(setup_ps_bundle _plugin _info_plist)
    set_target_properties(${_plugin} PROPERTIES
        MACOSX_BUNDLE                      TRUE
        MACOSX_BUNDLE_BUNDLE_NAME          ${_plugin}
        MACOSX_BUNDLE_BUNDLE_VERSION       "${C3C_PLUGINS_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT            "© 2026 Alexey Danilchenko, Sergey Sverdlov"
        MACOSX_BUNDLE_GUI_IDENTIFIER       com.C3C.${_plugin}
        MACOSX_BUNDLE_INFO_STRING          "Photoshop plugin"
        MACOSX_BUNDLE_LONG_VERSION_STRING  "${C3C_PLUGINS_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${C3C_PLUGINS_VERSION}"
        MACOSX_BUNDLE_INFO_PLIST           ${CMAKE_SOURCE_DIR}/${_info_plist}
        # Since the dependencies are bundled as part of the build stages (and not at install-time), build with the install rpath.
        # The plugin is dlopen'ed by Photoshop, so @executable_path would resolve to
        # Photoshop's own MacOS directory and the bundled dependencies would never be
        # found; @loader_path resolves to the plugin's own MacOS directory
        # (Contents/MacOS). Standalone dylibs live in Contents/PlugIns, frameworks
        # in Contents/Frameworks.
        BUILD_WITH_INSTALL_RPATH           TRUE
        INSTALL_RPATH                      "@loader_path/../Frameworks;@loader_path/../PlugIns"
    )

    # Modern Photoshop plugins carry their PiPL as
    # Contents/Resources/PiPLs.json inside the bundle
    # (TARGET_DIRECTORY is required: the property must be attached to the
    # file instance in the target's directory scope, not this one)
    set_source_files_properties(${CMAKE_SOURCE_DIR}/${_plugin}/PiPLs.json
        TARGET_DIRECTORY ${_plugin}
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(${_plugin} PRIVATE ${CMAKE_SOURCE_DIR}/${_plugin}/PiPLs.json)

    # The plugins are loaded by Photoshop, not launched: link as a bundle
    # (MH_BUNDLE, no main() required)
    set_target_properties(${_plugin} PROPERTIES
        LINK_OPTIONS "-Wl,-bundle"
    )
endmacro()

setup_ps_bundle(C3CImageSize mac/Info.plist)
setup_ps_bundle(C3CImageSizeAutomate mac/Info.plistAutomate)

# For static QT builds - import plugins
qt6_import_plugins(C3CImageSize
    INCLUDE Qt6::QCocoaIntegrationPlugin
)
qt6_import_plugins(C3CImageSizeAutomate
    INCLUDE Qt6::QCocoaIntegrationPlugin
)

# Note: with dynamic Qt the cocoa platform plugin (Qt6::QCocoaIntegrationPlugin)
# is a MH_BUNDLE and cannot be linked into the plugin; it is copied to
# Contents/PlugIns/platforms by copy_qt_plugins() and loaded at runtime.

# PkgInfo (CFBundlePackageType + CFBundleSignature, 8 chars, no newline)
# goes to Contents/PkgInfo (MACOSX_PACKAGE_LOCATION is relative to Contents).
# The file must be named exactly "PkgInfo" in the bundle: codesign treats
# any other file at the Contents/ level as an unsigned code object and
# macdeployqt's ad-hoc signing would fail.
set_source_files_properties(${CMAKE_SOURCE_DIR}/mac/C3CImageSize/PkgInfo
    TARGET_DIRECTORY C3CImageSize
    PROPERTIES MACOSX_PACKAGE_LOCATION "."
)
target_sources(C3CImageSize PRIVATE ${CMAKE_SOURCE_DIR}/mac/C3CImageSize/PkgInfo)
set_source_files_properties(${CMAKE_SOURCE_DIR}/mac/C3CImageSizeAutomate/PkgInfo
    TARGET_DIRECTORY C3CImageSizeAutomate
    PROPERTIES MACOSX_PACKAGE_LOCATION "."
)
target_sources(C3CImageSizeAutomate PRIVATE ${CMAKE_SOURCE_DIR}/mac/C3CImageSizeAutomate/PkgInfo)

# Plug-in bundle icon (Contents/Resources/C3CImageSize.icns, referenced by
# CFBundleIconFile in the Info.plist files)
foreach(_plugin C3CImageSize C3CImageSizeAutomate)
    set_source_files_properties(${CMAKE_SOURCE_DIR}/mac/C3CImageSize.icns
        TARGET_DIRECTORY ${_plugin}
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(${_plugin} PRIVATE ${CMAKE_SOURCE_DIR}/mac/C3CImageSize.icns)
endforeach()

# Shared libs
macro(copy_imported_shared_lib _target _shared_target)
    get_target_property(_framework_target_imported "${_shared_target}" IMPORTED)
    if(_framework_target_imported)
        get_target_property(_framework_target_type "${_shared_target}" TYPE)
        if (_framework_target_type STREQUAL "SHARED_LIBRARY")
            # Resolve the library location for the current configuration,
            # falling back to any available configuration (the macOS Qt
            # frameworks and this TBB build ship a single binary for all
            # configurations, so Debug builds reuse it)
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                get_target_property(_shared_target_location "${_shared_target}" LOCATION_DEBUG)
            else()
                get_target_property(_shared_target_location "${_shared_target}" LOCATION_RELEASE)
            endif()
            if(NOT _shared_target_location OR _shared_target_location MATCHES "^NOTFOUND"
               OR NOT EXISTS "${_shared_target_location}")
                get_target_property(_shared_target_location "${_shared_target}" LOCATION_RELEASE)
            endif()
            if(NOT _shared_target_location OR _shared_target_location MATCHES "^NOTFOUND"
               OR NOT EXISTS "${_shared_target_location}")
                get_target_property(_shared_target_location "${_shared_target}" LOCATION)
            endif()
            if(EXISTS "${_shared_target_location}")
                # Frameworks go to Contents/Frameworks; standalone dylibs
                # (e.g. TBB's libtbb.12.17.dylib) go to Contents/PlugIns,
                # mirroring the classic Photoshop plugin layout (the old Xcode
                # build kept libomp.dylib in Contents/Plugins).
                if("${_shared_target_location}" MATCHES "\\.framework")
                    set(_shared_dest "Frameworks")
                else()
                    set(_shared_dest "PlugIns")
                endif()
                # Copy plugin to bundle *now* (don't wait for CMake install)
                set_source_files_properties(
                    ${_shared_target_location} PROPERTIES
                        MACOSX_PACKAGE_LOCATION "${_shared_dest}"
                )
                target_sources(${_target} PRIVATE ${_shared_target_location})
                # create symlink if needed
                if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                    get_target_property(_shared_imp_soname "${_shared_target}" IMPORTED_SONAME_DEBUG)
                else()
                    get_target_property(_shared_imp_soname "${_shared_target}" IMPORTED_SONAME_RELEASE)
                endif()
                if(NOT _shared_imp_soname OR _shared_imp_soname MATCHES "^NOTFOUND")
                    get_target_property(_shared_imp_soname "${_shared_target}" IMPORTED_SONAME_RELEASE)
                endif()
                get_filename_component(_shared_soname "${_shared_imp_soname}" NAME)
                get_filename_component(_shared_file "${_shared_target_location}" NAME)
                if(NOT ${_shared_soname} STREQUAL ${_shared_file})
                    add_custom_command(TARGET ${_target} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E rm -f $<TARGET_FILE_DIR:${_target}>/../${_shared_dest}/${_shared_soname}
                        COMMAND ${CMAKE_COMMAND} -E create_symlink
                            ${_shared_file}
                            $<TARGET_FILE_DIR:${_target}>/../${_shared_dest}/${_shared_soname}
                    )
                endif()
            else()
                message(FATAL_ERROR "Imported framework ${_shared_target} not found at: ${_shared_target_location}")
            endif()
        endif()
    else()
        message(FATAL_ERROR "Framework ${_shared_target} is not an IMPORTED CMake target")
    endif()
endmacro()

function(copy_libraries _target)
    # Handling non QT6
    foreach(_lib ${ARGN})
        if(NOT ${_lib} MATCHES "^Qt6")
            copy_imported_shared_lib(${_target} ${_lib})
        endif()
    endforeach()

    # Retrieve the absolute path to uic and then use that path to find
    # the binaries
    get_target_property(_uic_executable Qt6::uic IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_uic_executable}" DIRECTORY)
    find_program(_macdeployqt macdeployqt HINTS "${_qt_bin_dir}")
    # macdeployqt must run after all bundle content is in place. The
    # "OS X content" copy rules are prerequisites of the target's build
    # step whose order relative to the link is not guaranteed, so it is
    # scheduled as a custom target that depends on the plugin target.
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_target(${_target}_macdeployqt ALL
            COMMAND "${_macdeployqt}"
                "$<TARGET_BUNDLE_DIR:${_target}>"
                -always-overwrite -no-plugins -no-strip
            DEPENDS ${_target}
            COMMENT "Running macdeployqt..."
        )
    else()
        add_custom_target(${_target}_macdeployqt ALL
            COMMAND "${_macdeployqt}"
                "$<TARGET_BUNDLE_DIR:${_target}>"
                -always-overwrite -no-plugins
            DEPENDS ${_target}
            COMMENT "Running macdeployqt..."
        )
    endif()
endfunction()

# Qt6 plugins
# based on code from CMake's QtDialog/CMakeLists.txt
macro(copy_qt6_plugin _target _qt_plugin_name)
    get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
    if(EXISTS "${_qt_plugin_path}")
        get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
        get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
        get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
        set(_qt_plugin_dest "PlugIns/${_qt_plugin_type}")
        # Copy plugin to bundle *now* (don't wait for CMake install)
        set_source_files_properties(
            ${_qt_plugin_path} PROPERTIES
                MACOSX_PACKAGE_LOCATION "${_qt_plugin_dest}"
        )
        target_sources(${_target} PRIVATE ${_qt_plugin_path})
    else()
        message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found at: ${_qt_plugin_path}")
    endif()
endmacro()

function(copy_qt_plugins _target)
    copy_qt6_plugin(${_target} "Qt6::QCocoaIntegrationPlugin")
endfunction()

# Extras
function(copy_extras _target)
    foreach(_extraDir ${ARGN})
        # Copy plugin to bundle *now* (don't wait for CMake install)
        get_filename_component(dirName "${_extraDir}" NAME)
        add_custom_command(TARGET ${_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${_target}>/../${dirName}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${_extraDir} $<TARGET_FILE_DIR:${_target}>/../${dirName}
        )
    endforeach()
endfunction()

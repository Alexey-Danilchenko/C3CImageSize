# C3CPlugins — Developer Notes

This is a structural document for developers working on the C3C Photoshop
plugins (version 3.2): what each part of the project is for, how the CMake
build is wired, and how the current build relates to the legacy build
artifacts.

The plugins:

| Plugin | Kind | Entry point |
|---|---|---|
| **C3CImageSize** | Image resize filter (hidden from the menu, invoked from Photoshop scripts) | `ENTRYPOINT` |
| **C3CImageSizeAutomate** | "C3C Image Size..." action in the *C3C Geometry* category, scriptable via its own terminology dictionary | `AutoPluginMain` |

Both plugins are C++ / Qt 6 applications using the modern Adobe Photoshop SDK (JSON PiPL) and Intel TBB.

Build and installation instructions are in [INSTALL.md](INSTALL.md).

## Requirements

| Dependency | Notes |
|---|---|
| CMake ≥ 3.17 | Tested with CMake 4.4.3 (Makefiles generator) |
| Qt 6 (Core, Gui, Widgets, Network, Xml) | Tested with 6.12.0 |
| Intel TBB | Tested with 12.17. On macOS, TBB 12 is arm64-only and requires macOS 15.0+. The build is pinned to `arm64` / `CMAKE_OSX_DEPLOYMENT_TARGET 15.0` |
| Adobe Photoshop SDK (2025+) | Header-only. Located via `pluginsdk/samplecode/common/includes/PIDefines.h` (see `cmake/modules/FindPsSdk.cmake`) |
| macOS | Xcode / Apple clang, Apple Silicon |
| Windows | Visual Studio, best effort (see INSTALL.md) |

## Project layout

| Path | Role |
|---|---|
| `CMakeLists.txt` | Top level: target definitions, Qt/SDK/TBB discovery, output to `bin/<type>`, macdeployqt, `.app` to `.plugin` rename |
| `cmake/modules/FindPsSdk.cmake` | Locates the Photoshop SDK, provides the `PsSdk::PsSdk` target |
| `common/` | Sources shared by both plugins (`C3CImageSizeScripting`) |
| `C3CImageSize/` | Filter plugin sources + `PiPLs.json` |
| `C3CImageSizeAutomate/` | Actions plugin sources + `PiPLs.json` |
| `mac/` | macOS bundle setup: `Info.plist*`, `PkgInfo`, macdeployqt, dependency copying |
| `win/` | Windows setup: `.rc` resources (embed `PiPLs.json` as `JSON_PIPL`), `qwindows.dll` copying |

The build compiles the Photoshop SDK's `PIUSuites.cpp` (suite globals such as `sPSActionDescriptor`, `sPSUIHooks`) into both plugins.

## Legacy PiPL (`.r`) files

The repository still contains the original Carbon/Rez resource scripts from the pre-CMake (Xcode / Visual Studio 2008) build:

| File | Role in the legacy build |
|---|---|
| `C3CImageSize/C3CImageSize.r` | PiPL for the hidden filter plugin (`**Hidden**` category, `ENTRYPOINT`) |
| `C3CImageSizeAutomate/C3CImageSizeAuto.r` | PiPL for the *C3C Geometry* actions plugin (`AutoPluginMain`) |
| `common/C3CImageSizeTerminology.r` | Shared `'aete'` terminology dictionary, pulled in by both plugins via `#include` |

These files are **legacy and no longer supported**: the current CMake build does not compile them. The PiPL is now carried by `PiPLs.json` — on macOS the file is copied to `Contents/Resources/PiPLs.json` inside the bundle, and on Windows it is embedded in the `.8bf` as a `JSON_PIPL` resource (see `win/*.rc`). The JSON values (versions, entry points, supported modes, terminology) are kept aligned with what the `.r` files define, so the two remain equivalent.

If the plugins ever need to run in a non-Photoshop, Photoshop-compatible host that only understands the legacy binary PiPL, the `.r` files can be re-enabled. For posterity, here is how they were compiled before:

### macOS (Xcode, Rez)

Each Xcode project had a Rez build phase (`PBXRezBuildPhase`) that compiled the plugin's `.r` together with the SDK's `PIUtilities.r`:

- `C3CImageSize.xcodeproj` → `C3CImageSize.r` + `PIUtilities.r`
- `C3CImageSizeAutomate.xcodeproj` → `C3CImageSizeAuto.r` + `PIUtilities.r`

with `REZ_PREFIX_FILE` set to the SDK's `pluginsdk/samplecode/common/includes/MachOMacrezXcode.h` (which defines `Macintosh` / `TARGET_API_MAC_CARBON` / `TARGET_MAC_OS` and `#include <Carbon.r>`). The `.r` files in turn `#include` the SDK's `Types.r`, `SysTypes.r`, `PIGeneral.r` and `PIUtilities.r`, plus the plugin's `C3CImageSizeScripting.h` and `C3CImageSizeTerminology.r`. The Rez output was a standalone `Contents/Resources/<name>.rsrc` (Apple HFS resource-fork file) containing the legacy binary `'PIPL'` resource (`8BIM` records, with `8LIZ` for the actions plugin) and the `'aete'` terminology resource.

### Windows (MSVC, Cnvtpipl)

The Visual Studio projects used a custom build step per `.r` file:

1. Preprocess the `.r` with the C compiler (preprocessor only):
   `cl /I<include dirs> /EP /DMSWindows=1 /DWIN32=1 /Tc "<name>.r" > "$(IntDir)<name>.rr"`
2. Convert to the binary PiPL with the SDK tool:
   `$(PSDK)\samplecode\resources\cnvtpipl.exe "$(IntDir)<name>.rr" "$(IntDir)<name>.pipl"`
3. Delete the intermediate `.rr`.

The include directories were the project dir, `..\common`, and the SDK's `samplecode\common\includes`, `PhotoshopAPI\Photoshop`, `PhotoshopAPI\PICA_SP` and `samplecode\common\Resources`. The `.rc` file then `#include`d the generated `<name>.pipl`, and the resource compiler was given `AdditionalIncludeDirectories="$(IntDir)"` so `rc.exe` could find it; the binary PiPL was embedded as a Windows resource in the `.8bf`.

## Notes

- The macOS build works in Release and Debug. Codesign is clean and all dependencies are bundled.
- The Windows path is best-effort. The IIQRemap reference app it mirrors is unverified on Windows too.
- `C3CImageSize/C3CImageSize2FilterScripting.jsx` is a manual-install Photoshop script (placed in the Photoshop Scripts folder); the build copies it to `bin/<BuildType>/` alongside the plugins. The release workflow reorganizes the archive into a `C3CImageSize/` folder (plugins, plus the dependent DLLs on Windows) and a `Scripts/` folder (the .jsx).
- Legacy files not part of the CMake build: the `.r` PiPL scripts (see [Legacy PiPL (`.r`) files](#legacy-pipl-r-files)).
- `build/`, `build-debug/` and `bin/` are git-ignored.

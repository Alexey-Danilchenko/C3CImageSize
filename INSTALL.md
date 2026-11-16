# Installing C3CPlugins

The build produces self-contained artifacts in `bin/<BuildType>` (Release or Debug).
See [DEVELOPER_NOTES.md](DEVELOPER_NOTES.md) for the dependency requirements.

## macOS

Copy the bundle(s) from `bin/Release/` (or `bin/Debug/`) into the per-user plug-in folder:

```
~/Library/Application Support/Adobe/Photoshop/Plug-Ins/
```

(Or the application-level `Adobe Photoshop.app/Contents/Plug-Ins/` for an
all-users install.) Then restart Photoshop.

- **C3CImageSizeAutomate.** The "C3C Image Size..." action becomes available in the
  Actions panel and via scripts (terminology dictionary `c3C2`, category *C3C Geometry*).
- **C3CImageSize.** The plugin registers the filter in the *Hidden* category, so it does
  not appear in the Filter menu. Photoshop scripts invoke it (see
  `C3CImageSize/C3CImageSize2FilterScripting.jsx`).

## Windows

Copy the entire contents of `bin/<BuildType>/` (the `.8bf` files, the Qt/TBB DLLs and the
`platforms/` directory) into Photoshop's `Plug-Ins` folder (or `Plug-PerUser` for a
per-user install), keeping the layout intact, then restart Photoshop.

## Installing/building from sources

### macOS (verified)

```sh
# Release
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/Users/dalex/Qt/6.11.0/macos:/Users/dalex/Development/TBB:/Users/dalex/Development/photoshopsdk"
cmake --build build -j 8

# Debug (separate build directory)
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="/Users/dalex/Qt/6.11.0/macos:/Users/dalex/Development/TBB:/Users/dalex/Development/photoshopsdk"
cmake --build build-debug -j 8
```

`CMAKE_PREFIX_PATH` must point to the Qt, TBB and Photoshop SDK roots. On non-Windows
platforms you can pass it shell-style, colon-separated; the top-level `CMakeLists.txt`
normalizes it to CMake's `;`-separated list. On Windows, pass a regular `;`-separated
list.

### Windows (best effort)

Same commands on a Windows machine. The result is `C3CImageSize.8bf` and
`C3CImageSizeAutomate.8bf` with the Qt and TBB DLLs and `platforms/qwindows.dll` placed
alongside, ready to drop into Photoshop's Plug-Ins folder. The Windows build is
unverified.

### Output

- **macOS.** `bin/Release/C3CImageSize.plugin` and `bin/Release/C3CImageSizeAutomate.plugin`
  (Release; `bin/Debug/...` for Debug). Each is a self-contained bundle:
  - `Contents/Resources/PiPLs.json` (JSON PiPL)
  - `Contents/PkgInfo`
  - `Contents/Frameworks/` with the Qt frameworks and TBB
  - `Contents/PlugIns/platforms/libqcocoa.dylib`
  - macdeployqt signs the bundle ad-hoc
- **Windows.** `bin/<BuildType>/C3CImageSize.8bf` and `C3CImageSizeAutomate.8bf`, with the
  Qt/TBB DLLs and `platforms/qwindows.dll` alongside.

Debug builds reuse the same Qt/TBB binaries as Release, since Qt and TBB ship a single
binary for all configurations. Debug keeps the full symbol table (`-no-strip`).

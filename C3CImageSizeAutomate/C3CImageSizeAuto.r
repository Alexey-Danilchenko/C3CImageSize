//-------------------------------------------------------------------------------
//
//    File:
//        C3CImageSizeAuto.r
//
//    Description:
//        This file contains the resource definitions (PiPL and 'aete'
//        terminology) for the C3CImageSizeAutomate actions plugin.
//
//    LEGACY - NO LONGER SUPPORTED
//        This file is a leftover from the original Xcode build and is NOT
//        compiled by the current CMake build. The plugin's PiPL is now defined
//        by PiPLs.json (macOS: Contents/Resources/PiPLs.json; Windows: embedded
//        as the JSON_PIPL resource). The JSON values are kept aligned with what
//        this file defines. See the "Legacy PiPL (.r) files" section of
//        README.md for how this file was compiled on each platform.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//    Definitions -- Required by include files.
//-------------------------------------------------------------------------------

// The About box and resources are created in PIUtilities.r.
// You can easily override them, if you like.

#define plugInName          "C3C Image Size"
#define plugInVendor        "C3C Geometry"
#define plugInDescription   "C3CImageSize plugin"

//#define PIPL_DEEP_MODES

#define plugInAETEComment   "C3C image resize automation plugin."
#define plugInSuiteID       'c3C2'
#define plugInClassID       plugInSuiteID
#define plugInEventID       plugInClassID
#define plugInUniqueID      "5f3d6843-d895-44c2-bbb7-b71ca0cb8c01"


//-------------------------------------------------------------------------------
//    Set up included files for Macintosh and Windows.
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "PITerminology.h"
#include "PIActions.h"

#if __PIMac__
    #include "Types.r"
    #include "SysTypes.r"
    #include "PIGeneral.r"
    #include "PIUtilities.r"
#elif defined(__PIWin__)
    #define Rez
    #include "PIGeneral.h"
    #include "PIUtilities.r"
#endif

#include "C3CImageSizeScripting.h"

resource 'PiPL' ( 16000, plugInName, purgeable)
{
    {
        Kind { Actions },
        Name { plugInName"..." },
        Category { plugInVendor },
        Version { (latestActionsPlugInVersion << 16) | latestActionsPlugInSubVersion },

        #ifdef __PIMac__

            #if (defined(__i386__))

                CodeMacIntel32 { "AutoPluginMain" },

                /* If your plugin uses Objective-C, Cocoa, for UI it should not be
                   unloaded, this flag is valid for 32 bit plug-ins only and
                   should not be used in any windows section */

                // off for now as this plug-in has no Objective-C Cocoa {},

            #endif
            #if (defined(__x86_64__))
                CodeMacIntel64 { "AutoPluginMain" },
            #endif
        #else
            #if defined(_WIN64)
                CodeWin64X86 { "AutoPluginMain" },
            #else
                CodeWin32X86 { "AutoPluginMain" },
            #endif
        #endif

        EnableInfo { "in (PSHOP_ImageMode, GrayScaleMode, RGBMode, RGB48Mode) ||"
                           "PSHOP_ImageDepth == 16"},

        HasTerminology
        {
            plugInClassID,
            plugInEventID,
            16000,
            plugInUniqueID
        }
    }
};

#include "C3CImageSizeTerminology.r"

//-------------------------------------------------------------------------------

// end C3CImageSizeAuto.r

//-------------------------------------------------------------------------------
//
//    File:
//        C3CImageSize.r
//
//    Description:
//        This file contains the resource definitions (PiPL and 'aete'
//        terminology) for the C3CImageSize filter.
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

#define plugInName          "C3C Image Size"
//#define plugInVendor        "C3C Filters"
#define plugInVendor        "**Hidden**"
#define plugInDescription   "C3C image resize plugin."

#define plugInAETEComment       "C3C image resize plugin"
#define plugInSuiteID           'c3C1'
#define plugInClassID           plugInSuiteID
#define plugInEventID           plugInClassID

//#define PIPL_DEEP_MODES


//-------------------------------------------------------------------------------
//    Set up included files for Macintosh and Windows.
//-------------------------------------------------------------------------------

#include "PIDefines.h"

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

#define C3C_IMAGE_SIZE_FILTER

resource 'PiPL' ( 16000, plugInName, purgeable)
{
    {
        Kind { Filter },
        Name { plugInName },
        Category { plugInVendor },
        Version { (latestFilterVersion << 16) | latestFilterSubVersion },

        #ifdef __PIMac__

            #if (defined(__i386__))

                CodeMacIntel32 { "ENTRYPOINT" },

                /* If your plugin uses Objective-C, Cocoa, for UI it should not be
                   unloaded, this flag is valid for 32 bit plug-ins only and
                   should not be used in any windows section */

                // off for now as this plug-in has no Objective-C Cocoa {},

            #endif
            #if (defined(__x86_64__))
                CodeMacIntel64 { "ENTRYPOINT" },
            #endif
        #else
            #if defined(_WIN64)
                CodeWin64X86 { "ENTRYPOINT" },
            #else
                CodeWin32X86 { "ENTRYPOINT" },
            #endif
        #endif

        SupportedModes
        {
            noBitmap, doesSupportGrayScale,
            noIndexedColor, doesSupportRGBColor,
            noCMYKColor, noHSLColor,
            noHSBColor, noMultichannel,
            noDuotone, noLABColor
            #ifdef PIPL_DEEP_MODES
        ,    doesSupportGray16, doesSupportRGB48,
          noLab48, noCMYK64,
          noDeepMultichannel, noDuotone16
            #endif
        },

        HasTerminology
        {
            plugInClassID,
            plugInEventID,
            16000,
            c3cImageSizeUniqueID
        },

        EnableInfo { "in (PSHOP_ImageMode, GrayScaleMode, RGBMode, RGB48Mode) ||"
                           "PSHOP_ImageDepth == 16"},

        PlugInMaxSize { 2000000, 2000000 },

        FilterLayerSupport {doesSupportFilterLayers},

        FilterCaseInfo
        {
            {
                /* Straight data, no selection */
                inStraightData, outStraightData,
                doNotWriteOutsideSelection,
                filtersLayerMasks, worksWithBlankData,
                copySourceToDestination,

                /* Straight data, no selection */
                inStraightData, outStraightData,
                doNotWriteOutsideSelection,
                filtersLayerMasks, worksWithBlankData,
                copySourceToDestination,

                /* Straight data, no selection */
                inStraightData, outStraightData,
                doNotWriteOutsideSelection,
                filtersLayerMasks, worksWithBlankData,
                copySourceToDestination,

                /* Straight data, no selection */
                inStraightData, outStraightData,
                doNotWriteOutsideSelection,
                filtersLayerMasks, worksWithBlankData,
                copySourceToDestination,

                /* Straight data, no selection */
                inStraightData, outStraightData,
                doNotWriteOutsideSelection,
                filtersLayerMasks, worksWithBlankData,
                copySourceToDestination,

                /* Straight data, no selection */
                inStraightData, outStraightData,
                doNotWriteOutsideSelection,
                filtersLayerMasks, worksWithBlankData,
                copySourceToDestination,

                /* Straight data, no selection */
                inStraightData, outStraightData,
                doNotWriteOutsideSelection,
                filtersLayerMasks, worksWithBlankData,
                copySourceToDestination
            }
        }
    }
};

#include "C3CImageSizeTerminology.r"

//-------------------------------------------------------------------------------

// end C3CImageSize.r

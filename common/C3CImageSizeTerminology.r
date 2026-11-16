//-------------------------------------------------------------------------------
//
//    File:
//        C3CImageSizeTerminology.r
//
//    Description:
//        This file contains the shared 'aete' terminology resource for the
//        C3C image resize plugins. It was pulled into each plugin's PiPL by the
//        #include in C3CImageSize.r and C3CImageSizeAuto.r.
//
//    LEGACY - NO LONGER SUPPORTED
//        This file is a leftover from the original Xcode build and is NOT
//        compiled by the current CMake build. Terminology is now declared in
//        PiPLs.json (the "Terminology" / "HasTerminology" sections). See the
//        "Legacy PiPL (.r) files" section of README.md for how this file was
//        compiled on each platform.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//    Definitions -- Required by include files.
//-------------------------------------------------------------------------------

resource 'aete' (16000, "C3CImageSize dictionary", purgeable)
{
    1, 0, english, roman,                                    /* aete version and language specifiers */
    {
        plugInVendor,                                        /* vendor suite name */
        plugInDescription,                                   /* optional description */
        plugInSuiteID,                                       /* suite ID */
        1,                                                   /* suite code, must be 1 */
        1,                                                   /* suite level, must be 1 */
        {                                                    /* structure for filters */
            plugInName,                                      /* unique filter name */
            plugInAETEComment,                               /* optional description */
            plugInClassID,                                   /* class ID, must be unique or Suite ID */
            plugInEventID,                                   /* event ID, must be unique to class ID */

            NO_REPLY,                                        /* never a reply */
            IMAGE_DIRECT_PARAMETER,                          /* direct parameter, used by Photoshop */
            {                                                /* parameters here, if any */
#ifdef C3C_IMAGE_SIZE_FILTER
                "Pixel Width",                               /* parameter name */
                keyPixelWidth,                               /* parameter key ID */
                typeInteger,                                 /* parameter type ID */
                "resizes width in pixels",                   /* optional description */
                flagsSingleParameter,                        /* parameter flags */

                "Pixel Height",                              /* parameter name */
                keyPixelHeight,                              /* parameter key ID */
                typeInteger,                                 /* parameter type ID */
                "resizes height in pixels",                  /* optional description */
                flagsSingleParameter,                        /* parameter flags */
#endif
                "Width",                                     /* parameter name */
                keyWidth,                                    /* parameter key ID */
                typeInteger,                                 /* parameter type ID */
                "resize width",                              /* optional description */
                flagsSingleParameter,                        /* parameter flags */

                "Height",                                    /* parameter name */
                keyHeight,                                   /* parameter key ID */
                typeInteger,                                 /* parameter type ID */
                "resize height",                             /* optional description */
                flagsSingleParameter,                        /* parameter flags */

                "Sharpness",                                 /* parameter name */
                keySharpnessIndex,                           /* parameter key ID */
                typeInteger,                                 /* parameter type ID */
                "sharpness index",                           /* optional description */
                flagsSingleParameter,                        /* parameter flags */

                "Resize Units",                              // second parameter
                keyResizeUnits,                              // parameter key ID
                typeResizeUnits,                             // parameter type ID
                "resize units",                              // optional description
                flagsEnumeratedParameter,                    // parameter flags

                "Constrain Proportions",                     /* parameter name */
                keyConstrProportions,                        /* parameter key ID */
                typeBoolean,                                 /* parameter type ID */
                "whether to constrain proportions",          /* optional description */
                flagsSingleParameter,                        /* parameter flags */

                "Resize Method",                             // second parameter
                keyResizeMethod,                             // parameter key ID
                typeResizeMethod,                            // parameter type ID
                "resize algorithm",                          // optional description
                flagsEnumeratedParameter                     // parameter flags
            }
        },
        {                                                    /* non-filter plug-in class here */
        },
        {                                                    /* comparison ops (not supported) */
        },
        {                                                    /* any enumerations */
            typeResizeUnits,                                 /* type resize types */
            {
                "pixels",                                    /* first value */
                runitsPixels,
                "pixel units",                               /* optional description */

                "%",                                         /* second value */
                runitsPercents,
                "percents"                                   /* optional description */
            },
            typeResizeMethod,                                /* type resize types */
            {
                "linear",                                    /* first value */
                resizeLinear,
                "C3C linear",                                /* optional description */

                "linear with contrast",                      /* second value */
                resizeLinearContrast,
                "C3C linear with contrast"                   /* optional description */
            }
        }
    }
};

//-------------------------------------------------------------------------------

// end C3CImageSizeTerminology.r

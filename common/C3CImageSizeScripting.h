//-------------------------------------------------------------------------------
// Scripting parameters
//-------------------------------------------------------------------------------
#ifndef _C3C_IMAGE_SIZE_SCRIPTING_H
#define _C3C_IMAGE_SIZE_SCRIPTING_H

#include "PIDefines.h"
#include "PIActions.h"
#include "PITerminology.h"

#define c3cImageSizeUniqueID    "dcc0c3c0-8e6e-4cb9-98e7-7531bf0266c4"

//#define plugInCopyrightYear  "2013"

#define keyResizeMethod      'mc3S'
#define typeResizeMethod     'tc3M'
#define resizeLinear         'rc30'
#define resizeLinearContrast 'rc31'

#define keyConstrProportions 'cC3P'
#define keyFitTo             'cC3F'

#define keySharpnessIndex    'pC3S'

#define keyResizeUnits       'uc3S'
#define typeResizeUnits      'tc3U'
#define runitsPixels         'uc30'
#define runitsPercents       'uc31'

#define keyPixelWidth        'pC3W'
#define keyPixelHeight       'pC3H'

#ifndef Rez

struct C3CResizeParameters
{
    int32 resizeWidth;
    int32 resizeHeight;
    DescriptorEnumID resizeUnits;
    Boolean constrainProportions;
     Boolean fitTo;
    DescriptorEnumID resizeMethod;
    int32 sharpnessIndex;

    // passed back for canvas resizing
    int32 pixelResizeWidth;
    int32 pixelResizeHeight;

    bool paramsOk;

    C3CResizeParameters(): resizeWidth(0),
                           resizeHeight(0),
                           resizeUnits(runitsPixels),
                           constrainProportions(true),
                           fitTo(true),
                           resizeMethod(resizeLinear),
                           sharpnessIndex(0),
                           paramsOk(false),
                           pixelResizeWidth(0),
                           pixelResizeHeight(0) {}

    void Init()
    {
        resizeWidth = 0;
        resizeHeight = 0;
        resizeUnits = runitsPixels;
        constrainProportions = true;
        fitTo = true;
        resizeMethod = resizeLinear;
        sharpnessIndex = 0;
        paramsOk = false;
        pixelResizeWidth = 0;
        pixelResizeHeight = 0;
    }

    bool ParamsOk()
    {
        // normalise
        if (resizeWidth <= 0)
            resizeWidth = resizeHeight;
        else if (resizeHeight <= 0)
            resizeHeight = resizeWidth;

        return paramsOk = (resizeWidth != 0 && resizeHeight != 0);
    }

    SPErr ReadScriptParameters(PIActionDescriptor actionDesc);
    SPErr WriteScriptParameters(PIActionDescriptor actionDesc);
};

#endif
#endif
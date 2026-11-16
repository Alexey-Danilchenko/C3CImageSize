// Scripting parameters
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "PITypes.h"
#include "PIFilter.h"

#include "C3CImageSizeScripting.h"
#include "PIUSuites.h"

//-------------------------------------------------------------------------------
//
// ReadScriptParameters
//
// See if we were called by the Photoshop scripting system and return the value
// in showUI if the user wants to see our dialog.
//
//-------------------------------------------------------------------------------
SPErr C3CResizeParameters::ReadScriptParameters(PIActionDescriptor actionDesc)
{
    Boolean hasKey = false;
    DescriptorEnumTypeID enumType;
    SPErr error = kSPNoError;

    if (actionDesc)
    {
        // getting width
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keyWidth, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetInteger(actionDesc, keyWidth, &resizeWidth);

        // getting height
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keyHeight, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetInteger(actionDesc, keyHeight, &resizeHeight);

        // getting constrain proportions
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keyConstrProportions, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetBoolean(actionDesc, keyConstrProportions, &constrainProportions);

        // getting Fit To // c3c
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keyFitTo, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetBoolean(actionDesc, keyFitTo, &fitTo);

          // getting resize units
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keyResizeUnits, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetEnumerated(actionDesc, keyResizeUnits, &enumType, &resizeUnits);

        // getting resize method
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keyResizeMethod, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetEnumerated(actionDesc, keyResizeMethod, &enumType, &resizeMethod);

        // getting sharpness
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keySharpnessIndex, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetInteger(actionDesc, keySharpnessIndex, &sharpnessIndex);


        // getting width
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keyPixelWidth, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetInteger(actionDesc, keyPixelWidth, &pixelResizeWidth);

        // getting height
        if (!error)
            error = sPSActionDescriptor->HasKey(actionDesc, keyPixelHeight, &hasKey);

        if (!error && hasKey)
            error = sPSActionDescriptor->GetInteger(actionDesc, keyPixelHeight, &pixelResizeHeight);
    }

    // set paramsOk
    if (!error)
    {
        ParamsOk();
    }

    return error;
}

//-------------------------------------------------------------------------------
//
// WriteScriptParameters
//
// Write our parameters to the Photoshop scripting system in case we are being
// recorded in the actions pallete.
//
//-------------------------------------------------------------------------------
SPErr C3CResizeParameters::WriteScriptParameters(PIActionDescriptor actionDesc)
{
    SPErr error = kSPNoError;

    if (actionDesc)
    {
        // write parameters
        if (!error)
            error = sPSActionDescriptor->PutInteger(actionDesc,
                                                    keyWidth,
                                                    resizeWidth);
        if (!error)
            error = sPSActionDescriptor->PutInteger(actionDesc,
                                                    keyHeight,
                                                    resizeHeight);
        if (!error)
            error = sPSActionDescriptor->PutBoolean(actionDesc,
                                                    keyConstrProportions,
                                                    constrainProportions);
        if (!error)
            error = sPSActionDescriptor->PutBoolean(actionDesc,
                                                    keyFitTo,
                                                    fitTo);
          if (!error)
            error = sPSActionDescriptor->PutEnumerated(actionDesc,
                                                       keyResizeUnits,
                                                       typeResizeUnits,
                                                       resizeUnits);
        if (!error)
            error = sPSActionDescriptor->PutEnumerated(actionDesc,
                                                       keyResizeMethod,
                                                       typeResizeMethod,
                                                       resizeMethod);
        if (!error && sharpnessIndex >= 0)
            error = sPSActionDescriptor->PutInteger(actionDesc,
                                                    keySharpnessIndex,
                                                    sharpnessIndex);
        if (!error && pixelResizeWidth > 0)
            error = sPSActionDescriptor->PutInteger(actionDesc,
                                                    keyPixelWidth,
                                                    pixelResizeWidth);
        if (!error && pixelResizeHeight > 0)
            error = sPSActionDescriptor->PutInteger(actionDesc,
                                                    keyPixelHeight,
                                                    pixelResizeHeight);
    }

    return error;
}

// end C3CImageSizeScripting.cpp

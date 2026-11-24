//-------------------------------------------------------------------------------
//    Includes
//-------------------------------------------------------------------------------

#include "C3CImageSizeAuto.h"
#include "C3CImageSizeScripting.h"
#include "PITerminology.h"
#include "QtPart.h"

//-------------------------------------------------------------------------------
//    Globals
//-------------------------------------------------------------------------------

// everyone needs access to the sPSBasic pointer
// new and delete operators for example
SPBasicSuite * sSPBasic = nullptr;

// there is a problem with the SuspendHistory() so we
// need a global error value to solve the problem
SPErr gError = kSPNoError;

//-------------------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------------------

DLLExport SPAPI SPErr AutoPluginMain(const char* caller,   // who is calling
                                     const char* selector, // what do they want
                                     void* message);       // what is the message

SPErr PlayeventFilter(PIDialogPlayOptions playInfo,
                      C3CResizeParameters& resizeParams,
                      C3CResizeParameters& returnParams);
SPErr PlayeventCanvasSize(int width, int height);
SPErr Execute(void * parameters);
SPErr ExecuteAbout(void);
SPErr DoIt(PSActionsPlugInMessage * message);

//-------------------------------------------------------------------------------
//
//    AutoPluginMain
//
//    All calls to the plug-in module come through this routine.
//    It must be placed first in the resource.  To achieve this,
//    most development systems require this be the first routine
//    in the source.
//
//    The entrypoint will be "pascal void" for Macintosh,
//    "void" for Windows.
//
//-------------------------------------------------------------------------------
DLLExport SPAPI SPErr AutoPluginMain(
    const char* caller,   // who is calling
    const char* selector, // what do they want
    void* message)        // what is the message
{
    SPErr error = kSPNoError;

    try
    {
        //all messages contain a SPMessageData*
        SPMessageData* basicMessage;
        basicMessage = (SPMessageData*) message;
        sSPBasic = basicMessage->basic;

        // check for SP interface callers
        if (sSPBasic->IsEqual(caller, kSPInterfaceCaller))
        {
            // pop an about box
            if (sSPBasic->IsEqual(selector, kSPInterfaceAboutSelector))
            {
                error = ExecuteAbout();
            }
        }

        // Photoshop is calling us
        if (sSPBasic->IsEqual(caller, kPSPhotoshopCaller))
        {
            // the one and only message
            if (sSPBasic->IsEqual(selector, kPSDoIt))
            {
                //now that we know more we can cast the message to a PSActionsPlugInMessage*
                PSActionsPlugInMessage* actionsMessage = (PSActionsPlugInMessage*) message;

                error = DoIt(actionsMessage);
            }
        }
    }
    catch(...)
    {
        if (error == 0)
            error = kSPBadParameterError;
    }

    return error;
}

SPErr PlayeventFilter(PIDialogPlayOptions playInfo,
                      C3CResizeParameters& resizeParams,
                      C3CResizeParameters& returnParams)
{
    PIActionDescriptor c3cImageSizeDesc = nullptr;
    DescriptorTypeID c3cFilterEventID;

    PIActionDescriptor result = nullptr;
    SPErr error = kSPNoError;

    error = sPSActionDescriptor->Make(&c3cImageSizeDesc);

    // get the filter plugin by ID
    if (!error)
        error = sPSActionControl->StringIDToTypeID(c3cImageSizeUniqueID, &c3cFilterEventID);

    // write parameters for it
    if (!error)
        resizeParams.WriteScriptParameters(c3cImageSizeDesc);

    // invoke filter
    if (!error)
        error = sPSActionControl->Play(&result,
                                       c3cFilterEventID,
                                       c3cImageSizeDesc,
                                       playInfo);

    // getting resized
    if (result)
    {
        // read results from result descriptor
        returnParams.ReadScriptParameters(result);
    }

    if (result != nullptr)
        sPSActionDescriptor->Free(result);
    if (c3cImageSizeDesc != nullptr)
        sPSActionDescriptor->Free(c3cImageSizeDesc);

    return error;
}

SPErr PlayeventCanvasSize(int width, int height)
{
    // Move this to the top of the routine!
    PIActionDescriptor imageResizeDesc = nullptr;
    PIActionDescriptor result = nullptr;
    SPErr error = kSPNoError;

    error = sPSActionDescriptor->Make(&imageResizeDesc);

    if (!error)
        error = sPSActionDescriptor->PutUnitFloat(imageResizeDesc, keyWidth, unitPixels, width);

    if (!error)
        error = sPSActionDescriptor->PutUnitFloat(imageResizeDesc, keyHeight, unitPixels, height);

    if (!error)
        error = sPSActionDescriptor->PutEnumerated(imageResizeDesc, keyHorizontal, typeHorizontalLocation, enumLeft);

    if (!error)
        error = sPSActionDescriptor->PutEnumerated(imageResizeDesc, keyVertical, typeVerticalLocation, enumTop);

    if (!error)
        error = sPSActionControl->Play(&result, eventCanvasSize, imageResizeDesc, plugInDialogSilent);

    if (result != nullptr)
        sPSActionDescriptor->Free(result);

    if (imageResizeDesc != nullptr)
        sPSActionDescriptor->Free(imageResizeDesc);

    return error;
}

SPErr DoIt(PSActionsPlugInMessage* message)
{
    SPErr error = kSPNoError;

    Auto_Ref reference;
    PIActionParameters* actionParams = message ? message->actionParameters : 0;

    ASZString nameAsZString = nullptr;

    sPSUIHooks->GetPluginName(message->d.self, &nameAsZString);

    sPSActionReference->PutEnumerated(reference.get(),
                                      classDocument,
                                      typeOrdinal,
                                      enumTarget);

    // If your execute routine error's you will not get the
    // error value from SuspendHistory for version 5.5 of Photoshop.
    // This was fixed for Photoshop 6.0. I am going to use a global error value
    // to work around this problem.
    sPSActionControl->SuspendHistory(reference.get(),
                                     ::Execute,
                                     actionParams,
                                     nameAsZString);

    error = gError;
    gError = kSPNoError;

    sASZString->Release(nameAsZString);

    return (error);
}

// actual execution of the plugin action
SPErr Execute(void* parameters)
{
    PIActionParameters* actionParams = (PIActionParameters*)parameters;
    PIActionDescriptor actionDesc = actionParams ? actionParams->descriptor : 0;

    C3CResizeParameters resizeParams;
    C3CResizeParameters returnParams;

    // read scripting params if we were called from action
    resizeParams.ReadScriptParameters(actionDesc);

     bool scripting = actionDesc ? true : false;
     WriteScripting(scripting);

    // invoke filter plugin and read results
    gError = PlayeventFilter(actionParams->playInfo, resizeParams, returnParams);

    if (!gError)
    {
        // if scripting results failed to be passed in - get it from registry
        if (returnParams.pixelResizeWidth <= 0 || returnParams.pixelResizeHeight <= 0)
            gError = ReadRegister(returnParams.pixelResizeWidth, returnParams.pixelResizeHeight);

        // invoke resize
        if (!gError)
            gError = PlayeventCanvasSize(returnParams.pixelResizeWidth, returnParams.pixelResizeHeight);
    }

    // write parameters back into action system
    if (!gError)
    {
        PIActionDescriptor writeBackDesc = 0;    // token to write our parameters to

        gError = sPSActionDescriptor->Make(&writeBackDesc);

        if (!gError && writeBackDesc)
            gError = returnParams.WriteScriptParameters(writeBackDesc);

        // now put our descriptor to actions params for return to the host
        if (!gError && actionParams)
        {
            if (actionParams->descriptor)
                sPSActionDescriptor->Free(actionParams->descriptor);

            actionParams->descriptor = writeBackDesc;
            actionParams->recordInfo = plugInDialogNone;
        }
    }

    return gError;
}

SPErr ExecuteAbout(void)
{
    DoAbout();
    return kSPNoError;
}

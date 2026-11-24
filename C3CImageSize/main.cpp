#include "C3CImageSize.h"

#include <QObject>
#include <QApplication>
#include <QSettings>
#include <tbb/tbb.h>

#include "PIDefines.h"
#include "PITypes.h"
#include "PIAbout.h"
#include "PIFilter.h"
#include "PIUtilities.h"
#include "PIColorSpaceSuite.h"

#if defined(Q_OS_MACOS)
#define BUNDLE_ID CFSTR("C3CImageSize")
// For static QT builds - import the platform plugin. With dynamic Qt
// the plugin is loaded at runtime from Contents/PlugIns/platforms
// inside the plugin bundle
#if defined(_QT_STATIC_) && QT_VERSION >= 0x060000
#include <QtPlugin>
Q_IMPORT_PLUGIN (QCocoaIntegrationPlugin);
#endif
#endif

#if defined(Q_OS_WIN)
#include "psnative.h"
#if defined(_QT_STATIC_) && QT_VERSION >= 0x050000
#include <QtPlugin>
Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif
#endif

#include "definitions.h"
#include "global.h"
#include "C3CConversion.h"
#include "scaling.h"
#include "statistics.h"

#include "C3CImageSizeScripting.h"

// resultCode
// -30102 - RGB
// -30103 - CMYK
// -30104 - HSL
// -30105 - HSB
// -30106 - Multichannel
// -30107 - Lab
// -30108 - RGB 48
// -30109 - Lab 48
// -30110 - CMYK 64
// -30111 - program error
// -30905 - incompatible with Photoshop version
// -30910 - interface problems
// -30910 - does not support services


//-------------------------------------------------------------------------------
// global variables
//-------------------------------------------------------------------------------

int16 from = 0;

SPBasicSuite * sSPBasic = nullptr;

void SetupAppStyle(QApplication& a)
{
#if defined(Q_OS_MACOS)
#if QT_VERSION >= 0x050000
    a.setStyle("fusion");
#endif
#endif
}

//-------------------------------------------------------------------------------
// local routines
//-------------------------------------------------------------------------------
// the six main routines of the plug in

void DoAbout(void);
void DoParameters(void);
void DoPrepare(void);
void DoStart(void);
void DoContinue(void);
void DoFinish(void);
void DoUI(void);
SPErr WriteScriptParameters();
SPErr ReadScriptParameters(bool& showUI);

void SetVRect(VRect* r, int32 top, int32 left, int32 bottom, int32 right)
{
    r->top = top;
    r->left = left;
    r->bottom = bottom;
    r->right = right;
}

//-------------------------------------------------------------------------------
//
//    PluginMain
//
//    All calls to the plug in module come through this routine.
//
//    Inputs:
//        const int16 selector        Host provides selector indicating what
//                                    command to do.
//
//    Inputs && Outputs:
//        FilterRecord *filterRecord    Host provides a pointer to parameter block
//                                    containing pertinent data && callbacks.
//                                    See PIFilter.h
//
//        intptr_t *data                Use this to store a handle or pointer to our global
//                                    data structure, which is maintained by the
//                                    host between calls to the plug in.
//
//    Outputs:
//        int16 *result                Returns error result. Some errors are handled
//                                    by the host, some are silent, && some you
//                                    must handle. See PIGeneral.h.
//
//-------------------------------------------------------------------------------
DLLExport MACPASCAL void ENTRYPOINT(const int16 selector,
                                    FilterRecordPtr filterRecord,
                                    intptr_t *data,
                                    int16 *result)
{
    // update our global parameters
    param = filterRecord;
    param->bigDocumentData->PluginUsing32BitCoordinates = 1;

    resultCode = noErr;
    if (selector == filterSelectorAbout)
    {
        sSPBasic = ((AboutRecord*)filterRecord)->sSPBasic;
    }
    else
    {
        sSPBasic = filterRecord->sSPBasic;
    }

    switch (selector)
    {
        case filterSelectorAbout:
            DoAbout();
            break;
        case filterSelectorParameters:
            DoParameters();
            break;
        case filterSelectorPrepare:
            DoPrepare();
            break;
        case filterSelectorStart:
            DoStart();
            break;
        case filterSelectorContinue:
            DoContinue();
            break;
        case filterSelectorFinish:
            DoFinish();
            break;
        default:
            break;
    }
    *result = resultCode;
}

int16 CheckParameters()
{
    if (param->imageMode != plugInModeRGBColor &&
        param->imageMode != plugInModeRGB48 &&
        param->imageMode != plugInModeGrayScale &&
        param->imageMode != plugInModeGray16)
    {
        resultCode = filterBadMode;
        return resultCode;
    }
    //else if (param->documentInfo->imageMode == plugInModeMultichannel)
    //{
    //    resultCode = getResultCode("cannot resize multichannel image");
    //    return resultCode;
    //}
    //else if (param->documentInfo->alphaChannels)
    //{
    //    resultCode = getResultCode("image has extra alpha channels");
    //    return resultCode;
    //}
    //else if (param->documentInfo->layersDescriptor)
    //{
    //    resultCode = getResultCode("image has several layers");
    //    return resultCode;
    //}
    //else if (param->imageMode != param->documentInfo->imageMode)
    //{
    //    resultCode = getResultCode("cannot resize a single channel");
    //    return resultCode;
    //}
    //else if (param->documentInfo->selection)
    //{
    //    resultCode = getResultCode("image has a selection");
    //    return resultCode;
    //}
    else
        return noErr;
}

//-------------------------------------------------------------------------------
//
// DoParameters
//
// Makes sure we have valid Data && Parameters handle(s). Locks && initializes
// these items.
//
// NOTE:
// This routine is NOT guaranteed to be called by Photoshop. If a user enters the
// CTRL-F keyboard shortcut to invoke the last filter command this routine will
// NOT be called. If the filter is ran by the actions pallete or an automation
// plug in this routine will NOT be called.
//
// NOTE:
// The fields in the gFilterRecord are not all valid at this stage.
//-------------------------------------------------------------------------------
void DoParameters(void)
{
    // our plugin always executes from automation so this won't be called at all
    resultCode = CheckParameters();
}

//-------------------------------------------------------------------------------
//
// DoPrepare
//
// Almost identical to DoParameters. Make sure we have valid Data && Parameters
// handle(s) && lock && initialize as necessary. Sets the bufferSpace &&
// maxSpace variables in the gFilterRecord so memory is used at an optimum.
//
// NOTE:
// The fields in the gFilterRecord are not all valid at this stage. We will take a
// guess at the actual tile size information.
//
//-------------------------------------------------------------------------------

void DoPrepare(void)
{
    const void* suite;

    // Acquire ColorSpaceSuite
    int Err = param->sSPBasic->AcquireSuite(kPSColorSpaceSuite, kPSColorSpaceSuiteVersion1, &suite);

    if (Err != 0)
        resultCode = errPlugInHostInsufficient;
    else
    {
        ColorSpaceSuite = (PSColorSpaceSuite1*)suite;

        int16 rL, ra, rb;
        int16 gL, ga, gb;
        int16 bL, ba, bb;

        InitConversion();

        GetLab(255, 0, 0, rL, ra, rb);
        GetLab(0, 255, 0, gL, ga, gb);
        GetLab(0, 0, 255, bL, ba, bb);

        if (rL == 160 && ra == 218 && rb == 206 &&
            gL == 212 && ga ==   0 && gb == 215 &&
            bL ==  77 && ba == 197 && bb ==  14)
            glSpace = AdobeRGB;
        else if (rL == 138 && ra == 209 && rb == 198 &&
                 gL == 224 && ga ==  49 && gb == 209 &&
                 bL ==  75 && ba == 196 && bb ==  16)
            glSpace = sRGB;
        else
            glSpace = unknownSpace;

        if (glSpace == AdobeRGB)
        {
            Lr = 0.29734;
            Lg = 0.62736;
            Lb = 0.07530; // Lb = 0.07529 in the standard, but then the sum = 0.99999
        }
        else
        {
            Lr = 0.2126;
            Lg = 0.7152;
            Lb = 0.0722;
        }

        param->bufferSpace = 0;
        param->maxSpace = 0;
    }
}

//-------------------------------------------------------------------------------
//
// DoStart
//
// The main filtering routine for this plug in. See if(we have any registry
// parameters from the last time we ran. Determine if(the UI needs to be
// displayed by reading the script parameters. Save the last dialog parameters
// in case something goes wrong or the user cancels.
//
//-------------------------------------------------------------------------------
void DoStart(void)
{
    SetVRect(&(param->bigDocumentData->inRect32),
             0,
             0,
             param->bigDocumentData->wholeSize32.v,
             param->bigDocumentData->wholeSize32.h);
    param->bigDocumentData->outRect32 = param->bigDocumentData->inRect32;
    param->inLoPlane = 0;
    param->inHiPlane = param->planes - 1;
    param->outLoPlane = 0;
    param->outHiPlane = param->planes - 1;

    param->haveMask = 1;

    from  = filterSelectorStart;
    resultCode = CheckParameters();
}

//-------------------------------------------------------------------------------
//
// DoContinue
//
// If we get here we probably did something wrong. This selector was needed
// before advanceState() was in the FilterRecord*. Now that we use advanceState()
// there is nothing for us to do but set all the rectangles to 0 && return.
//
//-------------------------------------------------------------------------------

void DoContinue(void)
{
    resultCode = CheckParameters();

    if (resultCode == noErr)
    {
        InitData(param);

        // our plugin will always be called by automation
        // so startUI flag always needs to be setup here
        bool showUI = true;

        SPErr error = ReadScriptParameters(showUI);
        if (!error && !resultCode)
        {
            if (showUI)
            {
                DoUI();
                CalculateNewSize(glWidth, glHeight, pluginParams, &pluginParams.pixelResizeWidth, &pluginParams.pixelResizeHeight);
            }
            else
                SetupFromParams();

            if (!resultCode)
            {
                int newWidth = pluginParams.pixelResizeWidth;
                int newHeight = pluginParams.pixelResizeHeight;

                if (newWidth > MAXSIZE)
                    resultCode = getResultCode("result image row is too long");//MAXSIZE_Error;
                else
                {
                    switch (glMode)
                    {
                    case plugInModeRGBColor:
                        Scale(glMethod, glSharpness, inData, outData, glWidth, glHeight, newWidth, newHeight);
                        break;
                    case plugInModeGrayScale:
                        Scale(glMethod, glSharpness, inData, outData, glWidth, glHeight, newWidth, newHeight);
                        break;
                    case plugInModeRGB48:
                        Scale(glMethod, glSharpness, inData16, outData16, glWidth, glHeight, newWidth, newHeight);
                        break;
                    case plugInModeGray16:
                        Scale(glMethod, glSharpness, inData16, outData16, glWidth, glHeight, newWidth, newHeight);
                        break;
                    default:
                        resultCode = filterBadMode;
                        break;
                    }
                }
            }

            if (!resultCode)
                WriteScriptParameters();
        }
        else if (!resultCode)
            resultCode = filterBadParameters;
    }

    SetVRect(&(param->bigDocumentData->inRect32), 0, 0, 0, 0);
    param->bigDocumentData->outRect32 = param->bigDocumentData->inRect32;

    from = filterSelectorContinue;
}

//-------------------------------------------------------------------------------
//
// DoFinish
//
// Everything went as planned && the pixels have been modified. Now record
// scripting parameters && put our information in the Photoshop Registry for the
// next time we get called. The Registry saves us from keeping a preferences file.
//
//-------------------------------------------------------------------------------
void DoFinish(void)
{
    //resultCode = CheckParameters();
    if (from != filterSelectorContinue)
        resultCode = getResultCode("finish error");
    else if (resultMessage)
        resultCode = getResultCode(resultMessage);
}

//-------------------------------------------------------------------------------
//
// DoAbout
//
// Pop a simple about box for this plug in.
//
// NOTE:    The global gFilterRecord is NOT a FilterRecord*. You must cast it to
//            an AboutRecord*. See PIAbout.h
//
//-------------------------------------------------------------------------------

void DoAbout(void)
{
    from = filterSelectorAbout;
}

//-------------------------------------------------------------------------------
//
// DoUI
//
// Pop main UI and request user parameters.
//
//-------------------------------------------------------------------------------
void DoUI(void)
{
    int argc = 0;
    char* argv[1];

#if defined(Q_OS_MACOS)
    // MacOS X specific application attributes set

    // This is needed if QT compiled as frameworks
    // QCoreApplication::addLibraryPath(GetBundlePath(BUNDLE_ID) + "/PlugIns");

    QCoreApplication::setAttribute(Qt::AA_PluginApplication, true);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, true);

#endif
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName(APPLICATION);

    SetupAppStyle(a);

#if defined(Q_OS_MACOS)

    C3CImageSize w(0);

    w.setWindowFlags(Qt::Window | Qt::WindowTitleHint
                                | Qt::CustomizeWindowHint
                                | Qt::WindowCloseButtonHint);

#else
    PlatformData* platform = (PlatformData*)param->platformData;

    C3CImageSize w;

    w.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint
                                | Qt::CustomizeWindowHint
                                | Qt::WindowCloseButtonHint
                                | Qt::MSWindowsFixedSizeDialogHint);

    // Embed the dialog in Photoshop's window (Qt 6 replacement for QWinWidget)
    QWindow* nativeParent = embedInNativeWindow(w, (HWND)platform->hwnd);
#endif
    w.show();

#if defined(Q_OS_MACOS)
    if (w.layout())
        w.layout()->setSizeConstraint(QLayout::SetFixedSize);
    w.setFixedSize(w.size());
#endif

#if defined(Q_OS_WIN)
    // Keep Photoshop's window disabled while the dialog is modal
    // (the legacy QWinWidget did the same via EnableWindow)
    EnableWindow((HWND)platform->hwnd, FALSE);
#endif
    resultCode = (w.exec() == QDialog::Accepted) ? 0 : userCanceledErr;
#if defined(Q_OS_WIN)
    EnableWindow((HWND)platform->hwnd, TRUE);
    // Safe: a foreign QWindow never destroys the wrapped native window
    delete nativeParent;
#endif
}

//-------------------------------------------------------------------------------
//
// ReadScriptParameters
//
// See if we were called by the Photoshop scripting system and return the value
// in showUI if the user wants to see our dialog.
//
//-------------------------------------------------------------------------------
SPErr ReadScriptParameters(bool& showUI)
{
    PIActionDescriptor actionDesc = nullptr;
    SPErr error = kSPNoError;

    pluginParams.Init();

    if (!param->descriptorParameters)
        return error;

    PIDialogPlayOptions dlgPlay = param->descriptorParameters->playInfo;

    PIDescriptorHandle descHandle = param->descriptorParameters->descriptor;

    error = sPSActionDescriptor->HandleToDescriptor(descHandle, &actionDesc);

    if (!error)
        error = pluginParams.ReadScriptParameters(actionDesc);

    if (actionDesc != nullptr)
        sPSActionDescriptor->Free(actionDesc);

    // set up showUI
    showUI = true;
    if (param->descriptorParameters->playInfo == plugInDialogSilent)
    {
        showUI = false;
        if (!pluginParams.paramsOk)
        {
        resultCode = filterBadParameters;
        error = kSPBadParameterError;
    }
    }
    else if (param->descriptorParameters->playInfo == plugInDialogDontDisplay &&
             pluginParams.paramsOk)
        showUI = false;

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
SPErr WriteScriptParameters(void)
{
    PIActionDescriptor actionDesc = nullptr;
    SPErr error = kSPNoError;

    error = sPSActionDescriptor->Make(&actionDesc);

    // write parameters
    error = pluginParams.WriteScriptParameters(actionDesc);

    // persist new descriptor
    if (param->descriptorParameters &&
        param->descriptorParameters->descriptor &&
        param->handleProcs)
    {
        HandleProcs * handleProcs = param->handleProcs;
        handleProcs->disposeProc(param->descriptorParameters->descriptor);
        param->descriptorParameters->descriptor = nullptr;
    }

    if (!error && param->descriptorParameters)
        error = sPSActionDescriptor->AsHandle(actionDesc, &param->descriptorParameters->descriptor);

    if (actionDesc)
        sPSActionDescriptor->Free(actionDesc);

    return error;
}


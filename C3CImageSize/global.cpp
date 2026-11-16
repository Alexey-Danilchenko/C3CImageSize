#include "C3CUtils.h"
#include "CurveUnit.h"
#include "sRGB.h"

int16 resultCode;
const char* resultMessage = nullptr;
FilterRecordPtr param = nullptr;

PSColorSpaceSuite1* ColorSpaceSuite;

uint8 *inData, *outData;
uint16 *inData16, *outData16;

int glWidth, glHeight;
int glPlanes;
int glInRowBytes;
int glAlignWidth;
int glMode;
int glSpace = sRGB;

int newWidth, newHeight;
int glMethod;
int glSharpness;

int glMAX;

C3CResizeParameters pluginParams;

float sRGB2LinCurve[SIZE16];
int Lin2sRGBCurve[SIZE16];

float RowFactor[MAXSIZE];
float LineFactor[MAXSIZE];

float ScaleCurve[SIZE16];

double Lr;
double Lg;
double Lb;

void CalcFixedCurves(int MAX)
{
    MakesRGB2LinCurve(sRGB2LinCurve, MAX);
    MakeLin2sRGBCurve(Lin2sRGBCurve, MAX);
}

void InitData(FilterRecordPtr param)
{
    glMode = param->imageMode;

    inData = (uint8*)param->inData;
    outData = (uint8*)param->outData;
    inData16 = (uint16*)param->inData;
    outData16 = (uint16*)param->outData;

    glWidth = param->bigDocumentData->wholeSize32.h;
    glHeight = param->bigDocumentData->wholeSize32.v;

    glPlanes = param->planes;
    glInRowBytes = param->inRowBytes;

    switch (glMode)
    {
        case plugInModeRGBColor:
            glMAX = MAX8;
            glAlignWidth = glInRowBytes/glPlanes;
            CalcFixedCurves(glMAX);
            break;
        case plugInModeGrayScale:
            glMAX = MAX8;
            glAlignWidth = glInRowBytes/glPlanes;
            CalcFixedCurves(glMAX);
            break;
        case plugInModeRGB48:
            glMAX = MAX16;
            glAlignWidth = glInRowBytes/glPlanes/2;
            CalcFixedCurves(glMAX);
            break;
        case plugInModeGray16:
            glMAX = MAX16;
            glAlignWidth = glInRowBytes/glPlanes/2;
            CalcFixedCurves(glMAX);
            break;
        default:
            resultCode = filterBadMode;
    }
}

int16 getResultCode(const char* inErrorString)
{
    resultMessage = inErrorString;

    resultCode = errReportString;

    char *pErrorString = (char*)param->errorString;
    if (pErrorString != nullptr && (strlen(inErrorString) < 256))
    {
        *pErrorString = (char)strlen(inErrorString);
        memcpy(pErrorString+1, inErrorString, (unsigned char)(*pErrorString));
    }
    return resultCode;
}

void CalculateNewSize(
    int32 imageWidth,
    int32 imageHeight,
    C3CResizeParameters params,
    int32* resWidth,
    int32* resHeight)
{
    int pixelWidth, pixelHeight;

    // Convert to pixels
    if (params.resizeUnits == runitsPixels)
    {
        pixelWidth = params.resizeWidth;
        pixelHeight = params.resizeHeight;
    }
    else
    {
        pixelWidth = roundi((float)params.resizeWidth/100.0f*imageWidth);
        pixelHeight = roundi((float)params.resizeHeight/100.0f*imageHeight);
    }

    if (imageWidth <= 0 || imageHeight <= 0 || pixelWidth <= 0 || pixelHeight <= 0)
    {
        *resWidth = 0;
        *resWidth = 0;
    }
    else if (pixelWidth > imageWidth && pixelHeight > imageHeight)
    {
        *resWidth = imageWidth;
        *resHeight = imageHeight;
    }
    else if (!params.constrainProportions )
    {
        if (pixelWidth > imageWidth)
            *resWidth = imageWidth;
        else
            *resWidth = pixelWidth;

        if (pixelHeight > imageHeight)
            *resHeight = imageHeight;
        else
            *resHeight = pixelHeight;
    }
    else if (params.fitTo)
    {
        if (pixelWidth*imageHeight < pixelHeight*imageWidth)
        {
            // Fit to Width
            *resWidth = pixelWidth;
            *resHeight = ROUNDI((float)pixelWidth/imageWidth*imageHeight);
        }
        else
        {
            // Fit to Height
            *resHeight = pixelHeight;
            *resWidth = ROUNDI((float)pixelHeight/imageHeight*imageWidth);
        }
    }
    else
    {
        *resWidth = pixelWidth;
        *resHeight = pixelHeight;
    }
}

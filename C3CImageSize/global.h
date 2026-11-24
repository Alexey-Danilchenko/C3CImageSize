#ifndef GLOBAL_H
#define GLOBAL_H

#include "PITypes.h"
#include "PIFilter.h"
#include "PIColorSpaceSuite.h"
//#include <PIChannelPortsSuite.h>

#include "C3CImageSizeScripting.h"

#define ORGANIZATION "Egor&Gleb Software"

#define MAXSIZE 32768

#define   MAX8 255
#define   MAX16 32768
#define   SIZE8 (MAX8 + 1)
#define   SIZE16 (MAX16 + 1)

#define AdobeRGB 0
#define sRGB 1
#define unknownSpace 2

extern int16 resultCode;
extern const char* resultMessage;
extern FilterRecordPtr param;
extern PSColorSpaceSuite1* ColorSpaceSuite;

extern uint8 *inData, *outData;
extern uint16 *inData16, *outData16;

extern C3CResizeParameters pluginParams;

extern int glWidth, glHeight;
extern int glPlanes;
extern int glInRowBytes;
extern int glAlignWidth;

extern int glMode;
extern int glSpace;

extern int newWidth, newHeight;
extern int glMethod;
extern int glSharpness;

extern int glMAX;

extern float sRGB2LinCurve[SIZE16];
extern int Lin2sRGBCurve[SIZE16];

extern float RowFactor[MAXSIZE];
extern float LineFactor[MAXSIZE];

extern float ScaleCurve[SIZE16];

void InitData(FilterRecordPtr param);

int16 getResultCode(const char* message);
void CalculateNewSize(int32 sourceWidth, int32 sourceHeight, C3CResizeParameters params, int32* resWidth, int32* resHeight);

#endif

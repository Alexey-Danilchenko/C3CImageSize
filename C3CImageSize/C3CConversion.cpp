#include "C3CConversion.h"
#include "global.h"

ColorServicesInfo RGB2Lab_info;

void InitConversion(void)
{
    RGB2Lab_info.infoSize = sizeof(RGB2Lab_info);
    RGB2Lab_info.selector = plugIncolorServicesConvertColor;
    RGB2Lab_info.reservedSourceSpaceInfo = nullptr;
    RGB2Lab_info.reservedResultSpaceInfo = nullptr;
    RGB2Lab_info.colorComponents[3] = 0;
    RGB2Lab_info.resultGamutInfoValid = true;
    RGB2Lab_info.reserved = nullptr;
    RGB2Lab_info.sourceSpace = plugIncolorServicesRGBSpace;
    RGB2Lab_info.resultSpace = plugIncolorServicesLabSpace;
}

void GetLab(int16 red, int16 green, int16 blue, int16 &L, int16 &a, int16 &b)
{
    RGB2Lab_info.colorComponents[0] = red;
    RGB2Lab_info.colorComponents[1] = green;
    RGB2Lab_info.colorComponents[2] = blue;

    param->colorServices(&RGB2Lab_info);

    L = RGB2Lab_info.colorComponents[0];
    a = RGB2Lab_info.colorComponents[1];
    b = RGB2Lab_info.colorComponents[2];
}


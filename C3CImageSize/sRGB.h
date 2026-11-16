#ifndef _sRGB_H
#define _sRGB_H

#include "C3CUtils.h"
#include "global.h"

extern double Lr;
extern double Lg;
extern double Lb;

// AdobeRGB (1998)
// Color Image Encoding
// Version 2005-05
// May 2005
// 0.57667 0.18556 0.18823
// 0.29734 0.62736 0.07529
// 0.02703 0.07069 0.99134

inline float sRGB2Lin(float c)
{
    if (glSpace == AdobeRGB)
        return pow(c, 2.19921875f);
    else
    {
        if (c<=0.04045f)
            return c/12.92f;
        else
            return pow((c+0.055f)/1.055f, 2.4f);
    }
}

inline float Lin2sRGB(float c)
{
    if (glSpace == AdobeRGB)
        return pow(c, 1.0f/2.19921875f);
    else
    {
        if (c<=0.0031308f)
            return 12.92f*c;
        else
            return 1.055f*pow(c, 1.0f/2.4f)-0.055f;
    }
}

inline float Lf2f(float R, float G, float B)
{
    return Lr*R + Lg*G + Lb*B;
}

#endif

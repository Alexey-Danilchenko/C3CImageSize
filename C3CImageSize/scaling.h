#ifndef SCALING_H
#define SCALING_H

#include <PIGeneral.h>

#include "global.h"
#include "statistics.h"
#include "C3CUtils.h"
#include "sRGB.h"

#include <math.h>
#include <tbb/tbb.h>

template <typename T>
void ScaleChannel(const int chan, T inData, float** arr, int width, int height, int newWidth, int newHeight)
{
    const int CHANNELS = glPlanes;

    int n = width;
    int m = newWidth;

    tbb::parallel_for(size_t(0), size_t(height), [&](size_t i)
    {
        float y[MAXSIZE];

        for (int j = 0; j<m; j++)
            y[j] = 0;

        int d = m-n;
        int k = 0;

        int offset = CHANNELS*glAlignWidth*i;

        for (int j = 0; j<n; j++)
        {
            float x = ScaleCurve[inData[chan + offset]];
            offset+=CHANNELS;

            if (d < 0)
            {
                y[k] += x;
                d += m;
            }
            else if (d == 0)
            {
                y[k++] += x;
                d = m-n;
            }
            else
            {
                float r = x*LineFactor[d];
                y[k++] += x - r;
                y[k] += r;
                d += m-n;
            }

        }

        for(int j=0; j<m; j++)
            arr[i][j] = y[j];
    });

    n = height;
    m = newHeight;

    tbb::parallel_for(size_t(0), size_t(newWidth), [&](size_t j)
    {
        float y[MAXSIZE];

        for (int i = 0; i<m; i++)
            y[i] = 0;

        int d = m-n;
        int k = 0;

        for (int i = 0; i<n; i++)
        {
            float x = arr[i][j];

            if (d < 0)
            {
                y[k] += x;
                d += m;
            }
            else if (d == 0)
            {
                y[k++] += x;
                d = m-n;
            }
            else
            {
                float r = x*RowFactor[d];
                y[k++] += x - r;
                y[k] += r;
                d += m-n;
            }
        }

        for (int i=0; i<m; i++)
            arr[i][j] = y[i];
    });
}

template <typename T>
void Out(int chan, float **arr, T outData, int AlignWidth, int newWidth, int newHeight)
{
    const int CHANNELS = glPlanes;

    tbb::parallel_for(size_t(0), size_t(newHeight), [&](size_t i)
    {
        for (int j=0; j<newWidth; j++)
            outData[CHANNELS*(AlignWidth*i + j) + chan] = Lin2sRGBCurve[ROUND(arr[i][j])];
    });
}

void Out2arr(float **arr1, float **arr2, int newWidth, int newHeight)
{
    tbb::parallel_for(size_t(0), size_t(newHeight), [&](size_t i)
    {
        for (int j=0; j<newWidth; j++)
            arr2[i][j] = arr1[i][j];
    });
}

void CorrectContrast(float fmin, float fmax, float Min, float Max, float** arr, int width, int height)
{
    float a = (fmax - fmin)/(Max - Min);
    float b = fmin - a*Min;

    tbb::parallel_for(size_t(0), size_t(height), [&](size_t i)
    {
        for (int j=0; j<width; j++)
            arr[i][j] = a*arr[i][j] + b;
    });
}

inline void Sharp(float** arr, int w, int h, float sharpness, float MAX)
{
    float percent;

    Assert(w <= MAXSIZE, "image row is too long");

    if (sharpness != 0.0f && h > 2 && w > 2)
    {
        float SQRT = sqrt(sqrt(2.0f));
        float alpha = pow(SQRT, sharpness);

        tbb::parallel_for(size_t(1), size_t(h-1), [&](size_t i)
        {
            float x = arr[i][0];
            for(int j = 1; j < w-1; j++)
            {
                float a = arr[i][j-1];
                float b = arr[i][j];
                float c = arr[i][j+1];
                float y = alpha*b + (1-alpha)*(a+b+c)/3.0f;
                arr[i][j-1] = x;
                x = y;
            }
            arr[i][w-2] = x;
        });

        tbb::parallel_for(size_t(1), size_t(w-1), [&](size_t j)
        {
            float x = arr[0][j];
            for (int i = 1; i < h-1; i++)
            {
                float a = arr[i-1][j];
                float b = arr[i][j];
                float c = arr[i+1][j];
                float y = alpha*b + (1-alpha)*(a+b+c)/3.0f;
                if (x<0)
                {
                    arr[i-1][j] = 0;
                }
                else if (x > MAX)
                {
                    arr[i-1][j] = MAX;
                }
                else
                    arr[i-1][j] = x;
                x = y;
            }
            if (x<0)
            {
                arr[h-2][j] = 0;
            }
            else if (x > MAX)
            {
                arr[h-2][j] = MAX;
            }
            else
                arr[h-2][j] = x;
        });
    }
}

template <typename T>
void Scale(int method, int sharpness, T inData, T outData, int width, int height, int newWidth, int newHeight)
{
    int mainPlanes;

    if (glMode == plugInModeRGBColor || glMode == plugInModeRGB48)
        mainPlanes = 3;
    else if (glMode == plugInModeGrayScale || glMode == plugInModeGray16)
        mainPlanes = 1;
    else {
        resultCode = filterBadMode;
        return;
    }

    float** arr = newArray(height, newWidth);
    if (arr == NULL)
        return;

    float **arr0 = newArray(newHeight, newWidth);
    if (arr0 == NULL)
    {
        freeArray(arr, newHeight);
        return;
    }

    float **arr1 = NULL;
    float **arr2 = NULL;

    if (mainPlanes == 3 )
    {
        arr1 = newArray(newHeight, newWidth);
        if (arr1 == NULL)
        {
            freeArray(arr, newHeight);
            freeArray(arr0, newHeight);
            return;
        }
        arr2 = newArray(newHeight, newWidth);
        if (arr2 == NULL)
        {
            freeArray(arr, newHeight);
            freeArray(arr0, newHeight);
            freeArray(arr1, newHeight);
            return;
        }
    }

    for(int i=0; i<newWidth; i++)
    {
        LineFactor[i] = (float)i/newWidth;
    }

    for (int i=0; i<newHeight; i++)
    {
        RowFactor[i] = (float)i/newHeight;
    }

    float norm = (float)MAX16*newWidth*newHeight/(width*height);

    for(int i=0; i<=glMAX; i++)
        ScaleCurve[i] = norm*sRGB2LinCurve[i];

    for (int ch = mainPlanes; ch < glPlanes; ch++)
    {
        ScaleChannel(ch, inData, arr, width, height, newWidth, newHeight);
        Out(ch, arr, outData, glAlignWidth, newWidth, newHeight);
    }

    float fmin;
    float fmax;

    if (method == LINEAR_CONTRAST_METHOD ||
        method == COMMON_CONTRAST_METHOD)
    {
        int min, max;
        MaxAndMin(inData, 0, glAlignWidth, height, max, min);

        for (int ch = 1; ch < mainPlanes; ch++)
        {
            int min2, max2;
            MaxAndMin(inData, ch, glAlignWidth, height, max2, min2);
            if (min2 < min)
                min = min2;
            if (max2 > max)
                max = max2;
        }

        fmin = MAX16*sRGB2LinCurve[min];
        fmax = MAX16*sRGB2LinCurve[max];
    }

    for (int ch = 0; ch < mainPlanes; ch++)
    {
        ScaleChannel(ch, inData, arr, width, height, newWidth, newHeight);
        if (ch == 0)
            Out2arr(arr, arr0, newWidth, newHeight);
        else if (ch == 1)
            Out2arr(arr, arr1, newWidth, newHeight);
        else if (ch == 2)
            Out2arr(arr, arr2, newWidth, newHeight);
    }

    freeArray(arr, height);

    if (sharpness != 0)
    {
        if (mainPlanes == 1)
            Sharp(arr0, newWidth, newHeight, sharpness, MAX16);
        else
        {
            float **Lightness = newArray(newHeight, newWidth);
            if (Lightness == NULL) return;

            tbb::parallel_for(size_t(0), size_t(newHeight), [&](size_t i)
            {
                for (int j = 0; j<newWidth; j++)
                    Lightness[i][j] = Lf2f(arr0[i][j], arr1[i][j], arr2[i][j]);
            });

            Sharp(Lightness, newWidth, newHeight, sharpness, MAX16);

            //int count = 0;
            tbb::parallel_for(size_t(0), size_t(newHeight), [&](size_t i)
            {
                for (int j = 0; j<newWidth; j++)
                {
                    float L = Lf2f(arr0[i][j], arr1[i][j], arr2[i][j]);

                    if (L > 0)
                    {
                        float r = Lightness[i][j]/L*arr0[i][j];
                        if (r > MAX16)
                            arr0[i][j] = MAX16;
                        else
                            arr0[i][j] = r;

                        float g = Lightness[i][j]/L*arr1[i][j];
                        if (g > MAX16)
                            arr1[i][j] = MAX16;
                        else
                            arr1[i][j] = g;

                        float b = Lightness[i][j]/L*arr2[i][j];
                        if (b > MAX16)
                            arr2[i][j] = MAX16;
                        else
                            arr2[i][j] = b;

                    }
                }
            });

            freeArray(Lightness, newHeight);
        }

    }

    if (method == LINEAR_CONTRAST_METHOD ||
        method == COMMON_CONTRAST_METHOD)
    {

        float Min, Max;

        MaxAndMin(arr0, newWidth, newHeight, Max, Min);

        for (int ch = 1; ch < mainPlanes; ch++)
        {
            float Min2, Max2;
            if (ch == 1)
                MaxAndMin(arr1, newWidth, newHeight, Max2, Min2);
            else if ( ch == 2)
                MaxAndMin(arr2, newWidth, newHeight, Max2, Min2);
            if (Min2 < Min) Min = Min2;
            if (Max2 > Max) Max = Max2;
        }

        for (int ch = 0; ch < mainPlanes; ch++)
        {
            if (ch == 0)
                CorrectContrast(fmin, fmax, Min, Max, arr0, newWidth, newHeight);
            else if (ch == 1)
                CorrectContrast(fmin, fmax, Min, Max, arr1, newWidth, newHeight);
            else if (ch == 2)
                CorrectContrast(fmin, fmax, Min, Max, arr2, newWidth, newHeight);
        }

    }

    for (int ch = 0; ch < mainPlanes; ch++)
    {
        if (ch == 0)
        {
            Out(ch, arr0, outData, glAlignWidth, newWidth, newHeight);
            freeArray(arr0, newHeight);
        }
        else if (ch == 1)
        {
            Out(ch, arr1, outData, glAlignWidth, newWidth, newHeight);
            freeArray(arr1, newHeight);
        }
        else if (ch == 2)
        {
            Out(ch, arr2, outData, glAlignWidth, newWidth, newHeight);
            freeArray(arr2, newHeight);
        }

    }
}

#endif

#ifndef _STATISTICS_H
#define _STATISTICS_H

#include "global.h"
#include <tbb/tbb.h>

template <typename T>
void MaxAndMin(T inData, int channel, int width, int height, int &max, int &min)
{
    min = inData[channel];
    max = min;

    tbb::parallel_for(size_t(0), size_t(height), [&](size_t i)
    {
        for (int j = channel; j < glPlanes * glAlignWidth; j += glPlanes)
        {
            int x = inData[glPlanes * glAlignWidth * i + j];

            if (x < min)
                min = x;
            else if (x > max)
                max = x;
        }
    });
}

void MaxAndMin(float** a, int width, int height, float &max, float &min)
{
    min = a[0][0];
    max = min;

    tbb::parallel_for(size_t(0), size_t(height), [&](size_t i)
    {
        for (int j = 0; j < width; j++)
        {
            float x = a[i][j];

            if (x < min)
                min = x;
            else if (x > max)
                max = x;
        }
    });
}

#endif
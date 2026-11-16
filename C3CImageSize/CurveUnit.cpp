#include <math.h>

#include "CurveUnit.h"
#include "C3CUtils.h"

#include "sRGB.h"
#include "global.h"
#include <tbb/tbb.h>

void MakesRGB2LinCurve(float curve[], int MAX)
{
    tbb::parallel_for(size_t(0), size_t(MAX+1), [&](size_t i)
    {
        curve[i] = sRGB2Lin(i/(float)MAX);
    });
}

void MakeLin2sRGBCurve(int curve[], int MAX)
{
    tbb::parallel_for(size_t(0), size_t(MAX16+1), [&](size_t i)
    {
        curve[i] = roundi(MAX*Lin2sRGB(i/(float)MAX16));
    });
}

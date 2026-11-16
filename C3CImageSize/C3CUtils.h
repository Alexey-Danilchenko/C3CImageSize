#ifndef _C3CUtils_H
#define _C3CUtils_H

#include <math.h>
#include <stdlib.h>
#include <PITypes.h>
#include <QCoreApplication>
#include <new>
#include "global.h"

using namespace std;

// prototype
inline void freeArray(float** &a, int n);

inline int ROUND(float x)
{
   return (int)(x+0.5f);
}

inline int ROUND(double x)
{
   return (int)(x+0.5);
}

inline int ROUNDI(float x)
{
   return (int)(x+0.5f);
}

inline int ROUNDI(double x)
{
   return (int)(x+0.5);
}

inline int roundi(float x)
{
   return (int)floor(x+0.5f);
}

inline int roundi(double x)
{
   return (int)floor(x+0.5);
}

inline void Assert(bool B, const char* message)
{
    if (!B)
    {
        resultCode = getResultCode(message);
        QCoreApplication::exit(resultCode/*code*/);
    }
}

inline float** newArray(int n, int m)
{
    float** a = new float*[n];

    for (int i=0; i<n; i++)
    {
        a[i] = nullptr;
        try
        {
            a[i] = new float[m];
        }
        catch (bad_alloc &ba)
        {
            Assert(false, "memory allocation error (float array)");
            return nullptr;
        }
    }

    return a;
}

inline void freeArray(float** &a, int n)
{
    for(int i=0; i<n; i++)
        if (a[i])
            delete [] a[i];
    delete [] a;
    a = NULL;
}

#endif

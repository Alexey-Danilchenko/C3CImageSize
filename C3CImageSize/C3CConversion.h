#ifndef _C3CConversion_H
#define _C3CConversion_H

#include "PITypes.h"

void InitConversion(void);
void GetLab(int16 red, int16 green, int16 blue, int16 &L, int16 &a, int16&b);

#endif
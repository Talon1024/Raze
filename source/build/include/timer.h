#pragma once

#ifndef timer_h__
#define timer_h__

#include "compat.h"

// for compatibility
#define timerUninit()

int      timerInit(int const tickspersecond);
void     timerUpdateClock(void);
int      timerGetClockRate(void);
uint64_t timerGetTicksU64(void);
uint64_t timerGetFreqU64(void);
double   timerGetHiTicks(void);
uint32_t timerGetTicks(void);

void (*timerSetCallback(void (*callback)(void)))(void);

inline void timerUpdate() // to avoid merge conflicts when the other games get updated.
{
	timerUpdateClock();
}

#endif // timer_h__

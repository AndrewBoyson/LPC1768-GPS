#include <stdbool.h>
#include <time.h>

extern bool     PpsIsStable(void);

extern int      PpsMsSinceLastPulse;
extern uint32_t PpsLastPulseMs;
extern bool     PpsLastPulseIsSet;

extern void     PpsMain(void);
extern void     PpsInit(void);
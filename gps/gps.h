#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

extern bool GpsTrace;
extern bool GpsVerbose;
extern int  GpsLog(const char *fmt, ...);
extern int  GpsLogVerbose(const char *fmt, ...);

extern void GpsHadPps(void);

extern void GpsInit(void);
extern void GpsMain(void);


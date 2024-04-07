#include <stdbool.h>

#include "lpc1768/tm/time64.h"

//Nmea module
extern void NmeaMain(void);
extern void NmeaInit(void);
extern void NmeaSerialMain(void);
extern void NmeaSerialInit(void);
extern bool NmeaSerialMessagesStopped;

//Nmea messages and commands
extern bool NmeaMsgTrace;
extern bool NmeaMsgMain(void);
extern void NmeaCmdSend(const char* command);
extern bool NmeaCmdTrace;
extern int  NmeaCmdAckItem;
extern int  NmeaCmdAckStatus;

//Nmea position declarations
extern bool NmeaFixIsStable(void);
extern int  NmeaDop;
extern int  NmeaSatelliteCount;
extern int  NmeaFixQuality;
#define NMEA_FIX_INVALID 0 // fix not available
#define NMEA_FIX_GPS     1 // GPS fix (SPS)
#define NMEA_FIX_DGPS    2 // DGPS fix

extern int  NmeaLat;
extern int  NmeaLng;
extern int  NmeaHgt;
extern int  NmeaHeightDevAboveMsl;
extern int  NmeaHeightMslAboveWgs;
#define NMEA_DEG_UNIT 10000000
#define NMEA_HGT_UNIT  1000000

extern void NmeaFixReceived(void);
extern int  NmeaLatAv;
extern int  NmeaLngAv;
extern int  NmeaHgtAv;
extern int  NmeaHeightDevAboveMslAv;

extern void NmeaDegToString(char plus, char minus, int lat, char *text);
extern void NmeaHgtToString(char plus, char minus, int hgt, char *text);
extern void NmeaDopToString(int dop, char* text);

extern void NmeaFixMain(void);
extern void NmeaFixInit(void);
extern bool NmeaFixMessagesStopped;

//Nmea time declarations
extern bool     NmeaTimeIsStable(void);
extern time64   NmeaTimeGetSeconds(void);
extern uint32_t NmeaTimeGetMs(void);
extern void     NmeaTimeMain(void);
extern void     NmeaTimeInit(void);
extern void     NmeaTimeReceived(time64 nmeaTime, int nmeaMs);
extern bool     NmeaTimeMessagesStopped;

//Nmea time bucket declarations
extern void     NmeaTimeBucketEnumerateStart(void);
extern bool     NmeaTimeBucketEnumerate(int size, char *text); //returns false if there is no more information
extern int      NmeaTimeBucketGetSize(void);
extern uint32_t NmeaTimeBucketGetItem(int item);
extern void     NmeaTimeBucketAdd(int nmeaStartMs);

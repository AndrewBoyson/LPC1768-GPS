#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "gps/gps.h"
#include "lpc1768/mstimer/mstimer.h"

#define AVERAGE_SHIFT_DIVISOR 10
#define TIMEOUT_MS         10000
#define CONFIDENCE_OK         15

static int confidence = 0;
static void confidenceReset()
{
    if (confidence == CONFIDENCE_OK) GpsLog("NMEA fix is not stable\r\n");
    confidence = 0;
}
static void confidenceIncrease()
{
    if (confidence == CONFIDENCE_OK) return;
    confidence++;
    if (confidence == CONFIDENCE_OK) GpsLog("NMEA fix is stable\r\n");
}
bool NmeaFixIsStable() { return confidence == CONFIDENCE_OK; }

int  NmeaLat = 0; // One degree is represented as 10 million (  90 ==   900,000,000) so one metre is represented by 100
int  NmeaLng = 0; // One degree is represented as 10 million ( 180 == 1,800,000,000) so one metre is represented by 100
int  NmeaHgt = 0; // One metre  is represented as  1 million (2100 == 2,100,000,000)

uint32_t nmeaLatAvLow = 0x80000000; //Holds the parts below the limits of an int
uint32_t nmeaLngAvLow = 0x80000000;
uint32_t nmeaHgtAvLow = 0x80000000;

int  NmeaLatAv = 0;
int  NmeaLngAv = 0;
int  NmeaHgtAv = 0;

int  NmeaHeightDevAboveMsl   = 0;
int  NmeaHeightDevAboveMslAv = 0;
int  NmeaHeightMslAboveWgs   = 0;

int  NmeaDop                 = 0; //In 100ths: dop < 2 is good; dop > 5 is poor.
int  NmeaSatelliteCount      = 0;
int  NmeaFixQuality          = 0;

void NmeaDegToString(char plus, char minus, int deg, char *text)
{
    int units    = labs(deg) / 10000000;
    int fraction = labs(deg) % 10000000;
    char dir = deg >= 0 ? plus : minus;
    sprintf(text, "%c%d.%05d", dir,   units,  fraction / 100);
}
void NmeaHgtToString(char plus, char minus, int hgt, char *text)
{
    int units    = labs(hgt) / 1000000;
    int fraction = labs(hgt) % 1000000;
    char dir = hgt >= 0 ? plus : minus;
    sprintf(text, "%c%d.%d", dir,   units,  fraction / 100000);
}
void NmeaDopToString(int dop, char* text)
{    //In 100ths
    int units    = dop / 100;
    int fraction = dop % 100;
    sprintf(text, "%d.%d", units, fraction);
}
static void addAv(int* pAccH, uint32_t* pAccL, int val, int shift)
{
    int64_t acc =  ((int64_t)*pAccH << 32) | *pAccL;
    int64_t diff = ((int64_t)val    << 32) - acc;
    int64_t add = diff >> shift;
    acc += add;
    *pAccH = acc >> 32;
    *pAccL = acc & 0xFFFFFFFF;
}

static uint32_t msTimerMsgWatchdog = (uint32_t)-TIMEOUT_MS;
bool NmeaFixMessagesStopped = true;
void NmeaFixReceived()
{
    static bool lastSet = false;
    if (NmeaFixMessagesStopped)
    {
        if (lastSet) GpsLog("NMEA fix messages have restarted after %d ms\r\n", MsTimerCount - msTimerMsgWatchdog);
        else         GpsLog("NMEA fix messages have started\r\n");
    }
    NmeaFixMessagesStopped = false;
    lastSet = true;
    msTimerMsgWatchdog = MsTimerCount; //Feed the watchdog whenever a fix message comes in
    
    if (NmeaFixQuality) confidenceIncrease();
    else                confidenceReset();

    NmeaHgt = NmeaHeightDevAboveMsl + NmeaHeightMslAboveWgs;
    
    if (confidence == CONFIDENCE_OK)
    {
        addAv(&NmeaLatAv, &nmeaLatAvLow, NmeaLat, AVERAGE_SHIFT_DIVISOR);
        addAv(&NmeaLngAv, &nmeaLngAvLow, NmeaLng, AVERAGE_SHIFT_DIVISOR);
        addAv(&NmeaHgtAv, &nmeaHgtAvLow, NmeaHgt, AVERAGE_SHIFT_DIVISOR);
    }
    
    NmeaHeightDevAboveMslAv = NmeaHgtAv - NmeaHeightMslAboveWgs;
}
void NmeaFixMain()
{
    if (MsTimerRelative(msTimerMsgWatchdog, TIMEOUT_MS))
    {
        if (!NmeaFixMessagesStopped) GpsLog("NMEA fix messages have stopped\r\n");
        NmeaFixMessagesStopped = true;
        confidenceReset();
    }
}
void NmeaFixInit()
{
    GpsLog("NMEA fix reset\r\n");
    confidenceReset();
    NmeaLat = 0;
    NmeaLng = 0;
    NmeaHgt = 0;
    NmeaDop = 0;
    NmeaSatelliteCount = 0;
    NmeaFixQuality     = 0;
}
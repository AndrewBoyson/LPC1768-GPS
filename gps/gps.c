#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "log/log.h"
#include "gps/nmea/nmea.h"
#include "pps.h"
#include "clock/clk/clkgov.h"

bool GpsTrace   = true;
bool GpsVerbose = false;

void GpsLog(const char *fmt, ...)
{
    if (!GpsTrace) return;
    va_list argptr;
    va_start(argptr, fmt);
    LogTimeF("GPS %04d ", PpsMsSinceLastPulse);
    LogV(fmt, argptr);
    va_end(argptr);
}
void GpsLogVerbose(const char *fmt, ...)
{
    if (!GpsTrace || !GpsVerbose) return;
    va_list argptr;
    va_start(argptr, fmt);
    LogTimeF("GPS %04d ", PpsMsSinceLastPulse);
    LogV(fmt, argptr);
    va_end(argptr);
}

#define SYNC_TYPE_NONE 0
#define SYNC_TYPE_PPS  1
#define SYNC_TYPE_TIME 2
static int syncType = SYNC_TYPE_NONE;

static void calculateSyncType()
{
    int newSyncType = SYNC_TYPE_NONE;
    if (PpsIsStable() && NmeaFixIsStable())
    {
        newSyncType = SYNC_TYPE_PPS;
        if (NmeaTimeIsStable()) newSyncType = SYNC_TYPE_TIME;
    }
    if (newSyncType != syncType)
    {
        switch (newSyncType)
        {
            case SYNC_TYPE_NONE:
                GpsLog("Sync stopped\r\n");
                break;
            case SYNC_TYPE_PPS:
                if (syncType == SYNC_TYPE_NONE) GpsLog("Sync started with PPS alone\r\n");
                else                            GpsLog("Syncing with PPS alone\r\n");
                break;
            case SYNC_TYPE_TIME:
                if (syncType == SYNC_TYPE_NONE) GpsLog("Sync started with PPS and NMEA time\r\n");
                else                            GpsLog("Syncing with PPS and NMEA time\r\n");
                break;
        }
        syncType = newSyncType;
    }
    ClkGovIsReceivingTime = syncType != SYNC_TYPE_NONE; //Tell the clock gov module if confident
}

void GpsHadPps()
{
    calculateSyncType();
    switch (syncType)
    {
        case SYNC_TYPE_PPS:
            ClkGovSyncPpsZ(); //Synchronise to nearest second of existing clock time
            break;
        case SYNC_TYPE_TIME:
        {
            //Check the last NMEA time message was not missed
            int msSinceLastNmeaTimeMsg = (int)(PpsLastPulseMs - NmeaTimeGetMs());
            if (msSinceLastNmeaTimeMsg > 50 && msSinceLastNmeaTimeMsg < 600) //This is the opposite way round to what you see on NMEA
            {
                ClkGovSyncPpsN(NmeaTimeGetSeconds() + 1); //Synchronise with Nmea time from the previous pulse
            }
            else
            {
                ClkGovSyncPpsZ(); //Synchronise to nearest second of existing clock time
            }
        }
    }
}

void GpsMain()
{
    NmeaMain();
    PpsMain();
    calculateSyncType();
}
void GpsInit()
{
    GpsLog("GPS reset\r\n");
    NmeaInit();
    PpsInit();
}

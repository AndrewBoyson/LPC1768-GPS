#include <stdint.h>


#include "nmea.h"
#include "gps/gps.h"
#include "gps/pps.h"
#include "log/log.h"
#include "lpc1768/mstimer/mstimer.h"
#include "clock/clk/clk.h"
#include "clock/clk/clkutc.h"
#include "clock/clk/clktime.h"

#define TIMEOUT_MS    10000
#define CONFIDENCE_OK_CLOCK_SET    20 * 60
#define CONFIDENCE_OK_CLOCK_NOT_SET 1 * 60

static time64   lastSeconds = 0;
static uint32_t lastMs      = 0;
uint32_t NmeaTimeGetMs()      { return lastMs; }
time64   NmeaTimeGetSeconds() { return lastSeconds; }

//Stability confidence
static int confidence =  0;
bool NmeaTimeIsStable()
{
    if (ClkTimeIsSet()) return confidence == CONFIDENCE_OK_CLOCK_SET;
    else                return confidence == CONFIDENCE_OK_CLOCK_NOT_SET;
}
static void confidenceReset()
{
    if (NmeaTimeIsStable()) GpsLog("NMEA time is not stable\r\n"); //Only show a message if WAS stable
    confidence = 0;
}
static void confidenceIncrease()
{
    if (NmeaTimeIsStable()) return;                            //Do nothing if already stable
    confidence++;
    if (NmeaTimeIsStable()) GpsLog("NMEA time is stable\r\n"); //Show a message if NOW stable
}


static uint32_t msTimerMsgTimeout = (uint32_t)-TIMEOUT_MS;
bool NmeaTimeMessagesStopped = true;
void NmeaTimeReceived(time64 nmeaMsgTime, int nmeaMs)
{   
    //Reset the msg timeout timer
    static bool lastSet = false;
    if (NmeaTimeMessagesStopped)
    {
        if (lastSet) GpsLog("NMEA time messages have restarted after %d ms\r\n", MsTimerCount - msTimerMsgTimeout);
        else         GpsLog("NMEA time messages have started\r\n");
    }
    NmeaTimeMessagesStopped = false;
    msTimerMsgTimeout = MsTimerCount;

    //Update confidence    
    if (lastSet)
    {
        time64 jump = nmeaMsgTime - lastSeconds - 1;
        if (jump)
        {
            GpsLog("NMEA time message has jumped %+lld seconds\r\n", jump);
            confidenceReset();
        }
        else
        {
            confidenceIncrease();
        }
    }
    
    //Save the timings
   lastSeconds = nmeaMsgTime;
   lastMs      = MsTimerCount;
   lastSet     = true;
}
void NmeaTimeMain()
{
    //Check if messages have stopped
    if (MsTimerRelative(msTimerMsgTimeout, TIMEOUT_MS))
    {
        if (!NmeaTimeMessagesStopped) GpsLog("NMEA time messages have stopped\r\n");
        NmeaTimeMessagesStopped = true;
        confidenceReset();
    }
}
void NmeaTimeInit()
{
    GpsLog("NMEA time reset\r\n");
    confidenceReset();
}

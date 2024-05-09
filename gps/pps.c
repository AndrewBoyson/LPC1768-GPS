#include <stdbool.h>
#include <time.h>

#include "log/log.h"
#include "clk/clkgov.h"
#include "lpc1768/mstimer/mstimer.h"
#include "lpc1768/hrtimer/hrtimer.h"
#include "lpc1768/register.h"
#include "lpc1768/gpio.h"
#include "lpc1768/bitband.h"
#include "gps.h"

#define     FIX_PIN FIO0PIN_ALIAS(23)
#define  ENABLE_DIR FIO0DIR_ALIAS(24)
#define  ENABLE_SET FIO0SET_ALIAS(24) = 1
#define  ENABLE_CLR FIO0CLR_ALIAS(24) = 1
#define     PPS_PIN FIO0PIN_ALIAS(17)
#define PPS_INT_ENR IO0INTENR_ALIAS(17) = 1
#define PPS_INT_CLR IO0INTCLR_ALIAS(17) = 1

static volatile uint32_t pulseHr = 0;        //Set after a PPS interrupt. Good for accurate interval between two pulses
static volatile uint32_t pulseMs = 0;        //Set after a PPS interrupt. Good for comparing between PPS and NMEA
static volatile bool     pulseValid = false; //Set after the first PPS interrupt
static volatile bool     hadPulse   = false; //Set after each PPS interrupt

int      PpsMsSinceLastPulse = 0;     //Calculated after a pulse from pulseHr
uint32_t PpsLastPulseMs      = 0;     //Copied     after a pulse from pulseMs
bool     PpsLastPulseIsSet   = false; //Copied     after a pulse from pulseValid

static void (*prevGpioHook)(void) = 0;

static void ppsHandler()
{
	if (prevGpioHook) prevGpioHook();
    PPS_INT_CLR;
    ClkGovSyncPpsI();
    pulseHr    = HrTimerCount();
    pulseMs    = MsTimerCount;
    pulseValid = true;
    hadPulse   = true;
}

#define CONFIDENCE_OK  10
#define TIMEOUT_MS   8000

static int confidence = 0;
static void confidenceReset()
{
    if (confidence == CONFIDENCE_OK) GpsLog("PPS is not stable\r\n");
    confidence = 0;
}
static void confidenceIncrease()
{
    if (confidence == CONFIDENCE_OK) return;
    confidence++;
    if (confidence == CONFIDENCE_OK) GpsLog("PPS is stable\r\n");
}
bool PpsIsStable() { return confidence == CONFIDENCE_OK; }

static bool pulseWasStopped = true;
static void checkForPulsesStoppingAndStarting()
{
    if (PpsMsSinceLastPulse > TIMEOUT_MS)
    {
        if (!pulseWasStopped)
        {
            GpsLog("PPS pulses have stopped\r\n");
            pulseWasStopped = true;
        }
        confidenceReset();
    }
    if (hadPulse)
    {
        static bool prevPulseValid = false;
        if (pulseWasStopped)
        {
            if (prevPulseValid) GpsLog("PPS pulses have restarted after %d ms\r\n", PpsMsSinceLastPulse);
            else                GpsLog("PPS pulses have started\r\n");
            pulseWasStopped = false;
        }
        prevPulseValid = pulseValid;
    }
}

static void buildConfidence()
{
    static uint32_t prevPulseHr = 0;
    static bool     prevPulseValid = false;
    if (prevPulseValid)
    {
        uint32_t hrSinceLastPulse        = pulseHr - prevPulseHr;
        uint32_t msSinceLastPulse        = hrSinceLastPulse / HR_TIMER_COUNT_PER_MS;
        uint32_t msSinceLastPulseMod1000 = msSinceLastPulse % 1000;

        if (msSinceLastPulseMod1000 == 999 || msSinceLastPulseMod1000 == 000)
        {
            confidenceIncrease();
        }
        else
        {
            GpsLog("PPS interval %03d is not 999 or 1000 ms\r\n", msSinceLastPulse);
            confidenceReset();
        }
    }
    prevPulseHr    = pulseHr;
    prevPulseValid = pulseValid;
}

void PpsMain()
{
    //Wait for 100ms after the start up in order to ignore initial glitches on the PPS line
    static bool afterStartup = false; //Need to store the timeout as hr timer rolls over every 44 seconds
    if (HrTimerCount() > HR_TIMER_COUNT_PER_SECOND / 10) afterStartup = true;
    if (!afterStartup)
    {
        hadPulse = false; //Cancel the pulse
        return;
    }
    
    checkForPulsesStoppingAndStarting(); //Do this before storing PPsMsSinceLastPulse to allow log to show when they started not zero
    
    PpsMsSinceLastPulse = HrTimerSinceMs(pulseHr);
    PpsLastPulseMs      = pulseMs;
    PpsLastPulseIsSet   = pulseValid;
    
    if (hadPulse)
    {
        buildConfidence();
        GpsHadPps();
    }

    hadPulse = false;
}
void PpsInit()
{
	prevGpioHook = GpioHook; //Hook into the GPIO interrupt handler
	GpioHook = ppsHandler;
	
    ENABLE_CLR;       //Set the enable pin to low
    ENABLE_DIR = 1;   //Set the enable pin direction to 1 == output
    GpsLog("PPS reset\r\n");
    confidenceReset();
    ENABLE_SET;       //Set the enable pin to high
    PPS_INT_ENR;      //Set the PPS pin to be interrupt on rise
}
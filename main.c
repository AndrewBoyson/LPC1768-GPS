
#include "lpc1768/crt.h"
#include "lpc1768/lpc1768.h"
#include "clk/clk.h"
#include "log/log.h"
#include "net/net.h"
#include "net/link/jack.h"
#include "net/udp/ntp/ntpserver.h"
#include "settings/settings.h"
#include "web/web.h"
#include "gps/gps.h"
#include "lpc1768/led.h"

int main()
{
	CrtInit();
	Lpc1768Init();
	SettingsInit();
	LogInit(ClkNowTmUtc, 115200);
	ClkInit();
	GpsInit();
	JackLinkLedDirPtr  = FIO2DIR_ALIAS_PTR(0); //P2.0 ==> p26 output
	JackLinkLedSetPtr  = FIO2SET_ALIAS_PTR(0);
	JackLinkLedClrPtr  = FIO2CLR_ALIAS_PTR(0);
	JackSpeedLedDirPtr = FIO2DIR_ALIAS_PTR(1); //P2.1 ==> p25 output
	JackSpeedLedSetPtr = FIO2SET_ALIAS_PTR(1);
	JackSpeedLedClrPtr = FIO2CLR_ALIAS_PTR(1);
	NetInit("GPS");
	NtpServerEnable = true;
	WebInit("GPS Clock");
      
    while (1)
    {   
		LogMain();
		ClkMain();
		GpsMain();
		NetMain();
		Lpc1768Main();
    }
}
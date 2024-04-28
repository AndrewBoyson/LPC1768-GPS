
#include "lpc1768/crt.h"
#include "lpc1768/lpc1768.h"
#include "clk/clk.h"
#include "log/log.h"
#include "net/net.h"
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
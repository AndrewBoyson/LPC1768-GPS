#include <stdint.h>

#include "lpc1768/startup.h"
#include "lpc1768-this/periphs.h"
#include "lpc1768/lpc1768.h"
#include "clock/clk/clk.h"
#include "log/log.h"
#include "net/net.h"
#include "settings/settings.h"
#include "web/web.h"
#include "gps/gps.h"
#include "lpc1768/led.h"
#include "net-this/net-this-leds.h"
#include "web-this/web-server-this.h"
#include "web-this/web-nav-this.h"

int main()
{
	     Startup();
     PeriphsInit();
	     LedInit();
     Lpc1768Init();
    SettingsInit();
         ClkInit();
         LogInit(ClkNowTmUtc, 115200);
         GpsInit();
         NetInit("GPS", NetThisLinkLed, NetThisSpeedLed, true);
		 
		WebHookDecideWhatToDo = WebServerThisDecideWhatToDo;
		WebHookHandleQuery    = WebServerThisHandleQuery;
		WebHookPost           = WebServerThisPost;
		WebHookReply          = WebServerThisReply;
		WebHookAddNav         = WebNavThis;
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
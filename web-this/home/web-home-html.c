
#include "web/http/http.h"
#include "web/web-add.h"
#include "web-this/web-nav-this.h"

void WebHomeHtml()
{
    HttpOk("text/html; charset=UTF-8", "no-cache", NULL, NULL);
    WebAddHeader3("Home", "settings.css", "ajax-class.js", "clock-class.js", "home.js");
    WebAddNav(HOME_PAGE);
    WebAddH1("Home");
    
    WebAddH2("GPS Messages");
    WebAddAjaxLed("Messages seen"        , "ajax-msgs-ok"      );
    WebAddAjaxLed("Fix messages seen"    , "ajax-fix-msgs-ok"  );
    WebAddAjaxLed("Time messages seen"   , "ajax-time-msgs-ok" );
    
    WebAddH2("GPS Time");
    WebAddAjaxLed("PPS is stable"        , "ajax-pps-stable"   );
    WebAddAjaxLed("NMEA time is stable"  , "ajax-nmea-stable"  );
    
    WebAddH2("Clock Status");
    WebAddAjaxLed("RTC is set"           , "ajax-rtc-set"      );
    WebAddAjaxLed("Clock is set"         , "ajax-clock-set"    );
    WebAddAjaxLed("External source is ok", "ajax-source-ok"    );
    WebAddAjaxLed("Rate synchronised"    , "ajax-rate-locked"  );
    WebAddAjaxLed("Time synchronised"    , "ajax-time-locked"  );

    WebAddH2("Server UTC time");
    HttpAddText("<div id='ajax-date-utc'></div>\r\n");

    WebAddH2("Server local time");
    HttpAddText("<div id='ajax-date-pc'></div>\r\n");
    
    WebAddH2("Server - PC (ms)");
    HttpAddText("<div id='ajax-date-diff'></div>\r\n");    
    
    WebAddEnd();
}

#include <stdio.h>

#include "web/http/http.h"
#include "web-this/web-nav-this.h"
#include "web/web-add.h"

void WebNmeaHtml()
{
    HttpOk("text/html; charset=UTF-8", "no-cache", NULL, NULL);
    WebAddHeader("NMEA", "settings.css", "nmea.js");
    WebAddNav(NMEA_PAGE);
    WebAddH1("NMEA");
    
    WebAddInputButton("Reset GPS module", "Reset", "/nmea", "reset");
        
    WebAddH2("Position");
    WebAddAjaxLabelled("WGS", "ajax-position");
    WebAddAjaxLabelled("Satellite count", "ajax-sat-count");
    WebAddAjaxLabelled("Dilution of Precision", "ajax-dop");
    
    WebAddH2("Height");
    WebAddAjaxLabelledSuffix("MSL above WGS",                 "ajax-msl-above-wgs",              "m");
    WebAddAjaxLabelledSuffix("Sensor height above MSL",       "ajax-hgt-above-msl",              "m");
    WebAddAjaxInputSuffix   ("Sensor height above ground", 2, "ajax-sensor-hgt", "sensorheight", "m");
    WebAddAjaxLabelledSuffix("Ground height above MSL",       "ajax-gnd-above-msl",              "m");
    
    WebAddH2("Trace");
    WebAddAjaxInputToggle("Trace GPS"    , "ajax-trace-gps"    , "gpstrace"     );
    WebAddAjaxInputToggle("Verbose GPS"  , "ajax-trace-verbose", "gpsverbose"   );
    WebAddAjaxInputToggle("Trace message", "ajax-trace-msg"    , "nmeamsgtrace" );
    WebAddAjaxInputToggle("Trace command", "ajax-trace-cmd"    , "nmeacmdtrace" );

    WebAddH2("ZDA timing bins");
    HttpAddText("<code id='ajax-buckets'></code>\r\n");
    
    WebAddEnd();
}

#include "web/http/http.h"

//Use http://tomeko.net/online_tools/cpp_text_escape.php   to convert from text to c-multiline
//Use http://tomeko.net/online_tools/cpp_text_unescape.php to convert from c-multiline to text

const char* WebHomeScriptDate = __DATE__;
const char* WebHomeScriptTime = __TIME__;

static const char* script =
#include "web/base/clock/web-clock-class.inc"
#include "web/common/web-ajax-class.inc"
#include "web-home-script.inc"
;
void WebHomeScript()
{
    HttpOk("application/javascript; charset=UTF-8", "max-age=3600", WebHomeScriptDate, WebHomeScriptTime);
    HttpAddText(script);
}

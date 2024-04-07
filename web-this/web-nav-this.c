#include "web/web-add.h"
#include "web-nav-this.h"

void WebNavThis(int page)
{
    WebAddNavItem(page == HOME_PAGE, "/",     "Home");
    WebAddNavItem(page == NMEA_PAGE, "/nmea", "Nmea");
}

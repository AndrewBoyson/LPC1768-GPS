#include "lpc1768/gpio.h"

#define LED_GR_L_DIR FIO2DIR(0) //P2.0 ==> p26 output
#define LED_GR_L_SET FIO2SET(0)
#define LED_GR_L_CLR FIO2CLR(0)

#define LED_YE_R_DIR FIO2DIR(1) //P2.1 ==> p25 output
#define LED_YE_R_SET FIO2SET(1)
#define LED_YE_R_CLR FIO2CLR(1)

void NetThisLinkLed(char on)
{
    LED_GR_L_DIR = 1; //Set the direction to 1 == output
	if (on) LED_GR_L_SET;
	else    LED_GR_L_CLR;
}
void NetThisSpeedLed(char on)
{
    LED_YE_R_DIR = 1; //Set the direction to 1 == output
	if (on) LED_YE_R_SET;
	else    LED_YE_R_CLR;
}

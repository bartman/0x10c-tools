#ifndef __included_0x10c_colours_h__
#define __included_0x10c_colours_h__

#define RSTCLR  "\033[0m"  /* reset */

#define CLR(CM,CC) \
	"\033["__CLR_MODE_##CM";"__CLR_COLOUR_##CC"m"

#define __CLR_MODE_N "0" /* normal */
#define __CLR_MODE_B "1" /* bold */
#define __CLR_MODE_F "2" /* faint */
#define __CLR_MODE_I "3" /* italic */
#define __CLR_MODE_U "4" /* underline */
#define __CLR_MODE_K "5" /* blink slow */
#define __CLR_MODE_k "6" /* blink rapid */
#define __CLR_MODE_R "7" /* inverse */
#define __CLR_MODE_S "9" /* strike-through */

#define __CLR_COLOUR_BLACK    "30"
#define __CLR_COLOUR_RED      "31"
#define __CLR_COLOUR_GREEN    "32"
#define __CLR_COLOUR_YELLOW   "33"
#define __CLR_COLOUR_BLUE     "34"
#define __CLR_COLOUR_MAGENTA  "35"
#define __CLR_COLOUR_CYAN     "36"
#define __CLR_COLOUR_WHITE    "37"



#endif // __included_0x10c_colours_h__

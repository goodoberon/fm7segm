#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

#define SEG_A				B
#define SEG_A_LINE			(1<<6)
#define SEG_B				D
#define SEG_B_LINE			(1<<6)
#define SEG_C				B
#define SEG_C_LINE			(1<<1)
#define SEG_D				C
#define SEG_D_LINE			(1<<1)
#define SEG_E				C
#define SEG_E_LINE			(1<<0)
#define SEG_F				B
#define SEG_F_LINE			(1<<7)
#define SEG_G				B
#define SEG_G_LINE			(1<<0)
#define SEG_P				B
#define SEG_P_LINE			(1<<2)

#define DIG_0				D
#define DIG_0_LINE			(1<<7)
#define DIG_1				D
#define DIG_1_LINE			(1<<5)
#define DIG_2				D
#define DIG_2_LINE			(1<<3)
#define DIG_3				D
#define DIG_3_LINE			(1<<4)

/* Encoder definitions */
#define ENCODER_A			B
#define ENCODER_A_LINE		(1<<4)
#define ENCODER_B			B
#define ENCODER_B_LINE		(1<<3)

/* Buttons definitions */
#define BUTTON_1			C
#define BUTTON_1_LINE		(1<<2)
#define BUTTON_2			D
#define BUTTON_2_LINE		(1<<0)
#define BUTTON_3			D
#define BUTTON_3_LINE		(1<<1)
#define BUTTON_4			D
#define BUTTON_4_LINE		(1<<2)

/* Temperature definitions */
#define TEMP_SENSOR			B
#define TEMP_SENSOR_LINE	(1<<5)

#endif /* PINS_H */

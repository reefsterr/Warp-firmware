#include <stdint.h>

/*
 *	config.h needs to come first
 */
#include "config.h"

#include "fsl_spi_master_driver.h"
#include "fsl_port_hal.h"

#include "SEGGER_RTT.h"
#include "gpio_pins.h"
#include "warp.h"
#include "devSSD1331.h"

volatile uint8_t	inBuffer[1];
volatile uint8_t	payloadBytes[1];


/*
 *	Override Warp firmware's use of these pins and define new aliases.
 */
enum
{
	kSSD1331PinMOSI		= GPIO_MAKE_PIN(HW_GPIOA, 8),
	kSSD1331PinSCK		= GPIO_MAKE_PIN(HW_GPIOA, 9),
	kSSD1331PinCSn		= GPIO_MAKE_PIN(HW_GPIOB, 11),
	kSSD1331PinDC		= GPIO_MAKE_PIN(HW_GPIOA, 12),
	kSSD1331PinRST		= GPIO_MAKE_PIN(HW_GPIOB, 0),
};

static int
writeCommand(uint8_t commandByte)
{
	spi_status_t status;

	/*
	 *	Drive /CS low.
	 *
	 *	Make sure there is a high-to-low transition by first driving high, delay, then drive low.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);
	OSA_TimeDelay(10);
	GPIO_DRV_ClearPinOutput(kSSD1331PinCSn);

	/*
	 *	Drive DC low (command).
	 */
	GPIO_DRV_ClearPinOutput(kSSD1331PinDC);

	payloadBytes[0] = commandByte;
	status = SPI_DRV_MasterTransferBlocking(0	/* master instance */,
					NULL		/* spi_master_user_config_t */,
					(const uint8_t * restrict)&payloadBytes[0],
					(uint8_t * restrict)&inBuffer[0],
					1		/* transfer size */,
					1000		/* timeout in microseconds (unlike I2C which is ms) */);

	/*
	 *	Drive /CS high
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);

	return status;
}



int
devSSD1331init(void)
{
	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Re-configure SPI to be on PTA8 and PTA9 for MOSI and SCK respectively.
	 */
	PORT_HAL_SetMuxMode(PORTA_BASE, 8u, kPortMuxAlt3);
	PORT_HAL_SetMuxMode(PORTA_BASE, 9u, kPortMuxAlt3);

	warpEnableSPIpins();

	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Reconfigure to use as GPIO.
	 */
	PORT_HAL_SetMuxMode(PORTB_BASE, 13u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTA_BASE, 12u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB_BASE, 0u, kPortMuxAsGpio);


	/*
	 *	RST high->low->high.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_ClearPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_SetPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);

	/*
	 *	Initialization sequence, borrowed from https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
	 */
	writeCommand(kSSD1331CommandDISPLAYOFF);	// 0xAE
	writeCommand(kSSD1331CommandSETREMAP);		// 0xA0
	writeCommand(0x72);				// RGB Color
	writeCommand(kSSD1331CommandSTARTLINE);		// 0xA1
	writeCommand(0x0);
	writeCommand(kSSD1331CommandDISPLAYOFFSET);	// 0xA2
	writeCommand(0x0);
	writeCommand(kSSD1331CommandNORMALDISPLAY);	// 0xA4
	writeCommand(kSSD1331CommandSETMULTIPLEX);	// 0xA8
	writeCommand(0x3F);				// 0x3F 1/64 duty
	writeCommand(kSSD1331CommandSETMASTER);		// 0xAD
	writeCommand(0x8E);
	writeCommand(kSSD1331CommandPOWERMODE);		// 0xB0
	writeCommand(0x0B);
	writeCommand(kSSD1331CommandPRECHARGE);		// 0xB1
	writeCommand(0x31);
	writeCommand(kSSD1331CommandCLOCKDIV);		// 0xB3
	writeCommand(0xF0);				// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
	writeCommand(kSSD1331CommandPRECHARGEA);	// 0x8A
	writeCommand(0x64);
	writeCommand(kSSD1331CommandPRECHARGEB);	// 0x8B
	writeCommand(0x78);
	writeCommand(kSSD1331CommandPRECHARGEA);	// 0x8C
	writeCommand(0x64);
	writeCommand(kSSD1331CommandPRECHARGELEVEL);	// 0xBB
	writeCommand(0x3A);
	writeCommand(kSSD1331CommandVCOMH);		// 0xBE
	writeCommand(0x3E);
	writeCommand(kSSD1331CommandMASTERCURRENT);	// 0x87
	writeCommand(0x06);
	writeCommand(kSSD1331CommandCONTRASTA);		// 0x81
	writeCommand(0x91);
	writeCommand(kSSD1331CommandCONTRASTB);		// 0x82
	writeCommand(0x50);
	writeCommand(kSSD1331CommandCONTRASTC);		// 0x83
	writeCommand(0x7D);
	writeCommand(kSSD1331CommandDISPLAYON);		// Turn on oled panel
	//writeCommand(0xA5);

	/*
	 *	To use fill commands, you will have to issue a command to the display to enable them. See the manual.
	 */

	writeCommand(0x26); //enable filling
	writeCommand(0xA1);
	//writeCommand(kSSD1331CommandFILL);
	//writeCommand(0x01);

	writeCommand(0x22); //draw rectangle
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x7F);
	writeCommand(0x3F);

	writeCommand(0x00); //line colors
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x00); //fill colors
	writeCommand(0x00);
	writeCommand(0x00);

	/*
	 *	Clear Screen
	 */
	//writeCommand(kSSD1331CommandCLEAR);
	//writeCommand(0x00);
	//writeCommand(0x00);
	//writeCommand(0x5F);
	//writeCommand(0x3F);



	/*
	 *	Any post-initialization drawing commands go here.
	 */
	//...



	return 0;
}

int draw_rectangle(int x_start, int y_start, int x_end, int y_end, int red, int green, int blue){
	writeCommand(0x22); //draw rectangle
	writeCommand(x_start);
	writeCommand(y_start);
	writeCommand(x_end);
	writeCommand(y_end);

	writeCommand(red); //line colors
	writeCommand(green);
	writeCommand(blue);
	writeCommand(red); //fill colors
	writeCommand(green);
	writeCommand(blue);

	return 0;
}

void draw_number(int number, int position){
	/*function to draw the necessary lines on the seven segment display
	to show the given number in the required position*/
	if(number == 4 || number == 5 || number == 6 || number == 8 || number == 9 || number == 0){
		draw_rectangle(6 + 29 * position, 10, 10 + 29 * position, 30, 0, 0, 255);
	}
	if(number == 2 || number == 3 || number == 5 || number == 6 || number == 7 || number == 8 || number == 9 || number == 0){
		draw_rectangle(6 + 29 * position, 10, 26 + 29 * position, 14, 0, 0, 255);
	}
	if(number == 1 || number == 2 || number == 3 || number == 4 || number == 7 || number == 8 || number == 9 || number == 0){
		draw_rectangle(26 + 29 * position, 10, 30 + 29 * position, 30, 0, 0, 255);
	}
	if(number == 1 || number == 3 || number == 4 || number == 5 || number == 6 || number == 7 || number == 8 || number == 9 || number == 0){
		draw_rectangle(26 + 29 * position, 34, 30 + 29 * position, 54, 0, 0, 255);
	}
	if(number == 2 || number == 3 || number == 4 || number == 5 || number == 6 || number == 8 || number == 9){
		draw_rectangle(6 + 29 * position, 30, 30 + 29 * position, 34, 0, 0, 255);
	}
	if(number == 2 || number == 6 || number == 8 || number == 0){
		draw_rectangle(6 + 29 * position, 34, 10 + 29 * position, 54, 0, 0, 255);
	}
	if(number == 2 || number == 3 || number == 5 || number == 6 || number == 8 || number == 0){
		draw_rectangle(6 + 29 * position, 50, 26 + 29 * position, 54, 0, 0, 255);
	}
	if(number == 10){
		draw_rectangle(32*position, 0, 32*position + 32, 0x3F, 0, 0, 0);
	}
}

void clear_screen(){
	//creates a blank screen by drawing a rectangle with the dimensions of the screen
	writeCommand(0x22); //draw rectangle
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x7F);
	writeCommand(0x3F);

	writeCommand(0x00); //line colors
	writeCommand(0xFF);
	writeCommand(0x00);
	writeCommand(0x00); //fill colors
	writeCommand(0xFF);
	writeCommand(0x00);
}

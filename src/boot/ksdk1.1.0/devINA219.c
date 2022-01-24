#include <stdlib.h>

#include "config.h"

#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"

#include "gpio_pins.h"
#include "SEGGER_RTT.h"
#include "warp.h"

extern volatile WarpI2CDeviceState	deviceINA219State;
extern volatile uint32_t		gWarpI2cBaudRateKbps;
extern volatile uint32_t		gWarpI2cTimeoutMilliseconds;

void
initINA219(const uint8_t i2cAddress)
{
	deviceINA219State.i2cAddress			= i2cAddress;

	return;
}

void
writeINARegister(uint8_t deviceRegister, uint8_t payload1, uint8_t payload2)
{
    uint8_t		payloadByte[2], commandByte[1];
	i2c_status_t	status;

	i2c_device_t slave =
	{
		.address = 0x40,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	/* Write config register */
	commandByte[0] = deviceRegister;
	payloadByte[0] = payload1; //0x01
	payloadByte[1] = payload2;
	warpEnableI2Cpins();

	status = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C instance */,
							&slave,
							commandByte,
							1,
							payloadByte,
							2,
							gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		warpPrint("Failed");
		warpPrint("%x", status);
	}
}

uint8_t
readINA(uint8_t deviceRegister, int numberOfBytes)
{
    uint8_t		cmdBuf[1] = {0xFF};
	i2c_status_t	status1, status;

	USED(numberOfBytes);
	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	cmdBuf[0] = deviceRegister;

	warpEnableI2Cpins();

	///warpPrint("Starting read");

	status1 = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							NULL,
							0, /* replace with size of current buffer */
							gWarpI2cTimeoutMilliseconds);

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							(uint8_t *)deviceINA219State.i2cBuffer,
							numberOfBytes, /* replace with size of current buffer */
							gWarpI2cTimeoutMilliseconds);

	if (status1 != kStatus_I2C_Success){
		warpPrint("NULL write failed");
		warpPrint("%x", status1);
	}

	if (status != kStatus_I2C_Success)
	{
		warpPrint("Failed");
		warpPrint("%x", status);
	}

	return status;
}

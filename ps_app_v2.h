#ifndef PS_APP_V2_H_
#define PS_APP_V2_H_

#include "main.h"
#include <unistd.h>
#include <string.h>
#include <x86intrin.h>
#include "power_states.h"
#include "qm_common.h"
#include "qm_interrupt.h"
#include "qm_gpio.h"

#include "qm_comparator.h"
#include "qm_pinmux.h"
#include "qm_isr.h"
#include "qm_adc.h"
#include "clk.h"
#include "qm_spi.h"

#include "24lcxx/24lcxx.h"
#include "crc16.h"

#include "debug.h"
#include "rtc/rtc.h"
#include "sx1272/sx1272.h"
#include "magnetometer/magnetometer.h"

#define THRESHOLD_ADDR 0x0003
#define THRESHOLD_SIZE 1
#define THRESHOLD_STEP 1 //Can be in 2..10 range

#define INTERRUPT_PIN (8) /*A0 - 3; D7 - 8*/

#define ACK 0xC0
#define END 0xB0
#define START 0xA0

typedef enum {
	DS_LOW, //FREE
	DS_HIGH, //OCCUPIED
} detected_state;

typedef enum {
	STATE_INIT, 		/* First run state, config is empty */
	STATE_CALIBRATE,    /* Device configured, but not calibrated */
	STATE_RUN,			/* Device configured, but not calibrated */
} state_flag;

typedef enum {
	AC_CLEAR,
	AC_SET,
} ac_setup;

void ParkSens_RawMag(void);
void ParkSens_SendPacket(uint8_t * f_address, detected_state f_ds_state, state_flag f_main_state);
void ParkSens_Init(bool calibrated);
void ParkSens_MainTask(uint8_t * dev_addr);
void ParkSens_TxTest(uint8_t * device_addr);
void ParkSens_HalInit(void);

#endif /* PS_APP_V2_H_ */

#ifndef __MAIN_H__
#define __MAIN_H__

#include <unistd.h>
#include <string.h>
#include <x86intrin.h>

#include "power_states.h"
#include "qm_common.h"
#include "qm_interrupt.h"
#include "qm_gpio.h"

#include "qm_uart.h"
#include "qm_comparator.h"
#include "qm_pinmux.h"
#include "qm_isr.h"
#include "qm_adc.h"
#include "clk.h"
#include "qm_spi.h"

#include "24lcxx/24lcxx.h"
#include "crc16.h"
#include "magnetometer/magnetometer.h"

#include "debug.h"
#include "rtc/rtc.h"
#include "cli/parser.h"
#include "cli/cli_config.h"
#include "cli/cli_microrl.h"
#include "sx1272/sx1272.h"

#define DEBUG0



#define RX (1)
/* the SPI clock divider is calculated in reference to a 32MHz system clock */
#define SPI_CLOCK_125KHZ_DIV (256)
#define SPI_CLOCK_1MHZ_DIV (32)
#define SPI_CLOCK_DIV SPI_CLOCK_125KHZ_DIV
#define QM_SPI_BMODE QM_SPI_BMODE_0
#define BUFFERSIZE (128)
/* wait time expressed in usecs */
#define WAIT_4SEC 4000000

#define PACKET_DELAY (ALARM_SEC*2)
#define CALIB_DELAY (ALARM_SEC*10)
#define VEHICLE_DELAY (ALARM_SEC*10)


#define STATE_ADDR 0x0000
#define STATE_SIZE 1

#define ID_DEVICE_ADDR 0x0001
#define ID_DEVICE_SIZE 2
#define ID_DEVICE_PRESET 1776

#define LED_PIN_ID (QM_PIN_ID_24)
#define PIN_MUX_FN (QM_PMUX_FN_0)

#define get_ticks() _rdtsc()

typedef struct  {
	uint8_t device_id[8];
	uint8_t hardware_id[5];
	uint8_t ps_stat;
	uint8_t relay_stat;
} device_t;

typedef struct {
	uint8_t device_id[8];
	uint8_t device_id_s : 8;
	uint8_t hardware_id[5];
	uint8_t hardware_id_s : 5;
	uint8_t ps_stat;
	uint8_t ps_stat_s : 1;
	uint8_t relay_stat;
	uint8_t relay_stat_s : 1;
	bmx1xx_mag_t mag;
} memory_t;


void led_blink(uint8_t count, unsigned long int time);

#endif /* __MAIN_H__ */

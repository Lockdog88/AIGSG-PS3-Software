#ifndef SX1272_H_
#define SX1272_H_

#include "stdio.h"
#include "math.h"
#include <inttypes.h>
#include "qm_common.h"
#include "qm_uart.h"
#include "qm_spi.h"
#include "qm_gpio.h"
#include "qm_pinmux.h"
#include "clk.h"
#include "stdlib.h"

#include "qm_comparator.h"
#include "qm_interrupt.h"
#include "qm_isr.h"
#include "../rtc/rtc.h"

//#define US915
#define EU868

#define DELAY50  50000UL
#define DELAY100 100000UL
#define DELAY200 200000UL

#define CSN_SX1272 QM_PIN_ID_0
#define sx1272_csn_enable() qm_gpio_clear_pin(QM_GPIO_0, CSN_SX1272)
#define sx1272_csn_disable() qm_gpio_set_pin(QM_GPIO_0, CSN_SX1272)

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define PIN_DIO0 (14)
#define PIN_DIO5 (3)

#define SPI_CLOCK_125KHZ_DIV (256)
#define SPI_CLOCK_1MHZ_DIV (32)
#define SPI_CLOCK_DIV SPI_CLOCK_125KHZ_DIV
#define QM_SPI_BMODE QM_SPI_BMODE_0

#define FREQ_STEP                   61.03515625


#define PA_RAMP						0x08 //50us

#define DEFAULT_SYNCWORD			0x12 //0x34 - Public LoRa

#define REG_LR_PARAMP               0x0A

#define REG_LR_RSSIVALUE            0x1B
#define REG_LR_PKTSNRVALUE          0x19
#define REG_LR_PKTRSSIVALUE         0x1A

#define REG_LR_FRFMSB               0x06
#define REG_LR_FRFMID               0x07
#define REG_LR_FRFLSB               0x08

#define REG_LR_FEIMSB               0x28
#define REG_LR_FEIMID               0x29
#define REG_LR_FEILSB               0x2A

#define REG_LR_SYNCWORD				0x39

#define REG_FIFO                    0x00
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_AD         0x0E
#define REG_FIFO_RX_BASE_AD         0x0F
#define REG_RX_NB_BYTES             0x13
#define REG_OPMODE                  0x01
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS               0x12
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_MODEM_CONFIG1           0x1D
#define REG_MODEM_CONFIG2           0x1E
#define REG_PAYLOAD_LENGTH          0x22
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_HOP_PERIOD              0x24
#define REG_RSSI_WIDEBAND           0x2C //For generate random number

// MODES
#define SX1272_MODE_RX_CONTINUOS      0x85
#define SX1272_MODE_TX                0x83
#define SX1272_MODE_SLEEP             0x80
#define SX1272_MODE_STANDBY           0x81
#define SX1272_MODE_CAD           	0x87

#define PAYLOAD_LENGTH              (10)
#define IMPLICIT_MODE               0x0A//0x0E//0x0C

// POWER AMPLIFIER CONFIG
#define REG_PA_CONFIG               0x09
#define PA_MAX_BOOST                0x8F
#define PA_LOW_BOOST                0x81
#define PA_MED_BOOST                0x8A
#define PA_OFF_BOOST                0x00

// LOW NOISE AMPLIFIER
#define REG_LNA                     0x0C
#define LNA_MAX_GAIN                0x23  // 0010 0011
#define LNA_OFF_GAIN                0x00

#define CH0 						(868100000)
#define CH1 						(868300000)
#define CH2 						(868500000)
#define CH3 						(867100000)
#define CH4 						(867300000)
#define CH5 						(867500000)
#define CH6 						(867700000)
#define CH7 						(867900000)



void Radio_Init(void);
void sendData(uint8_t buffer[], uint8_t size);
void readAllRegs(void);
uint8_t readRegister(uint8_t addr);
void SPI_Init(void);
uint8_t receiveMessage(uint8_t * message, uint8_t ret);
void startReceiving();
void setLoRaMode();
void SX1272SetChannel(uint8_t channel);
void setMode(uint8_t newMode);
uint32_t SX1272ReadFreqError(void);
void led_blink(uint8_t count, unsigned long int time);
int16_t SX1272ReadRssi();
bool SX1272IsChannelFree( int16_t rssiThresh );
void writeRegister(uint8_t addr, uint8_t value);
bool SX1272_CadDetector(void);
//void SX1272_GetRandSeed(uint8_t * seed);

#endif /* SX1272_H_ */

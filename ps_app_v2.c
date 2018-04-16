#include "ps_app_v2.h"
#include "debug.h"
#include "sx1272/sx1272.h"
#include "power_states.h"

uint16_t samples_polled[3] = {0};
state_flag st = STATE_RUN;
detected_state ds_state = DS_LOW;

void ParkSens_batmoninit(void);

void ParkSens_SendPacket(uint8_t * f_address, detected_state f_ds_state, state_flag f_main_state)
{
	uint8_t  payload[2] = {0};
	ParkSens_batmoninit();

	payload[0] |= BIT(f_main_state)|(f_ds_state<<3);
	payload[1] = samples_polled[2];
	uint8_t data_pack[100];
	sprintf((char*)data_pack, "%d,%s,%d,%d,%d", START, f_address, payload[0], payload[1], END);
	sendData(data_pack, sizeof(data_pack));
}

void ParkSens_batmoninit(void)
{
	qm_adc_xfer_t xfer;
	qm_adc_channel_t channels[1] = {QM_ADC_CH_1};
	/* Set the mode and calibrate. */
	qm_adc_set_mode(QM_ADC_0, QM_ADC_MODE_NORM_CAL);
	qm_adc_calibrate(QM_ADC_0);
	/* Set up xfer. */
	xfer.ch = channels;
	xfer.ch_len = 1;
	xfer.samples = samples_polled;
	xfer.samples_len = 3;
	/* Run the conversion. */
	qm_adc_convert(QM_ADC_0, &xfer, NULL);
	qm_adc_set_mode(QM_ADC_0, QM_ADC_MODE_PWR_DOWN);
}

__attribute__((unused)) void ParkSens_RawMag(void)
{
	while(1){
		bmx1xx_mag_t mag = {0};
		bmx1xx_mag_t mag_r = {0};
		bmx1xx_read_mag(&mag);
		QM_PRINTF("COMP: [ X %d Y %d Z %d ] ", mag.x, mag.y, mag.z);
		bmx1xx_read_raw_mag(&mag_r);
		QM_PRINTF("RAW: [ X %d Y %d Z %d ] \r\n", mag.x, mag.y, mag.z);
		//rtc_wait_sec(ALARM_SEC);

		uint8_t data_pack[100];
		sprintf((char*)data_pack, "COMP: [ X %d Y %d Z %d ] RAW: [ X %d Y %d Z %d ]",  mag.x, mag.y, mag.z,  mag_r.x, mag_r.y, mag_r.z);
		sendData(data_pack, sizeof(data_pack));
		clk_sys_udelay(5000000UL);
	}
}

void ParkSens_HalInit(void)
{
	qm_adc_config_t cfg;
	/* Enable the ADC and set the clock divisor. */
	clk_periph_enable(CLK_PERIPH_CLK | CLK_PERIPH_ADC |
					  CLK_PERIPH_ADC_REGISTER);
	clk_adc_set_div(100); /* ADC clock is 320KHz @ 32MHz. */

	/* Set up pinmux. */
	qm_pmux_select(QM_PIN_ID_1, QM_PMUX_FN_1);
	qm_pmux_input_en(QM_PIN_ID_1, true);

	/* Set up config. */
	cfg.window = 20; /* Clock cycles between the start of each sample. */
	cfg.resolution = QM_ADC_RES_8_BITS;
	qm_adc_set_config(QM_ADC_0, &cfg);
}

void ParkSens_Init(bool calibrated)
{
	mag_init();
	if (!calibrated)
	{
		RTC_WaitInSleepTicks(ALARM_SEC*10);
		bmx1xx_mag_t mag = {0};
		bmx1xx_read_raw_mag(&mag);
		RTC_WaitInSleepTicks(ALARM_SEC*10);
		bmc150_mag_read_reg_int();
	} else {

	}
	led_blink(3, 1600);
	ds_state = DS_LOW;
}

void ParkSens_TxTest(uint8_t * device_addr)
{
	while(1){
			ParkSens_SendPacket(device_addr, DS_HIGH, st);
			RTC_WaitTicks(10*QM_RTC_ALARM_SECOND(CLK_RTC_DIV_1));
			ParkSens_SendPacket(device_addr, DS_LOW, st);
			RTC_WaitTicks(10*QM_RTC_ALARM_SECOND(CLK_RTC_DIV_1));
		}
}

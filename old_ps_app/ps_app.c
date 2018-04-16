#include "ps_app.h"
#include "debug.h"
#include "sx1272/sx1272.h"
#include "power_states.h"
#include "crc16.h"

uint16_t samples_polled[3] = {0};
state_flag st = STATE_RUN;
detected_state ds_state = DS_LOW;

int high_mag_threshold = 0;
int low_mag_threshold = 0;

void ParkSens_batmoninit(void);
void ParkSens_acinit(ac_setup state);
void ParkSens_Wait_After_Detect_Routine(uint8_t * device_addr, uint32_t time, detected_state state);

void ParkSens_Send_Packet(uint8_t * f_address, detected_state f_ds_state, state_flag f_main_state)
{
	//unsigned short crc;

	uint8_t  payload[2] = {0};
	ParkSens_batmoninit();

	payload[0] |= BIT(f_main_state)|(f_ds_state<<3);
	payload[1] = samples_polled[2];
	//crc = calculateCRC16(payload, sizeof(payload));
	uint8_t data_pack[100];
	sprintf((char*)data_pack, "%d,%s,%d,%d,%d", START, f_address, payload[0], payload[1], END);
	sendData(data_pack, sizeof(data_pack));
}

void ParkSens_High_Thres_Routine(uint8_t * device_addr)
{
	bmc150_mag_read_reg_int();
	//bmc150_mag_set_h_threshold(high_mag_threshold);
	bmc150_mag_set_threshold((uint8_t)low_mag_threshold, (uint8_t)high_mag_threshold);
	ParkSens_acinit(AC_SET);
	qm_power_soc_deep_sleep(QM_POWER_WAKE_FROM_GPIO_COMP);
	ParkSens_acinit(AC_CLEAR);
	ParkSens_Wait_After_Detect_Routine(device_addr, VEHICLE_DELAY, DS_HIGH);
}

void ParkSens_Low_Thres_Routine(uint8_t * device_addr)
{
	bmc150_mag_read_reg_int();
	//bmc150_mag_set_l_threshold(low_mag_threshold);
	//bmc150_mag_set_threshold(high_mag_threshold, low_mag_threshold);
	bmx1xx_mag_t mag = {0};
	bmx1xx_read_raw_mag(&mag);
	if ((mag.z/16)<low_mag_threshold) bmc150_mag_set_h_threshold((uint8_t)low_mag_threshold);
	else bmc150_mag_set_l_threshold((uint8_t)high_mag_threshold);
	ParkSens_acinit(AC_SET);
	qm_power_soc_deep_sleep(QM_POWER_WAKE_FROM_GPIO_COMP);
	ParkSens_acinit(AC_CLEAR);
	ParkSens_Wait_After_Detect_Routine(device_addr, VEHICLE_DELAY, DS_LOW);
}

void ParkSens_accallback()
{
	//QM_SCSS_INT->int_comparators_host_mask |= BIT(WAKEUP_COMPARATOR_PIN);
	QM_INTERRUPT_ROUTER->comparator_0_host_int_mask |= BIT(WAKEUP_COMPARATOR_PIN);
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

void ParkSens_acinit(ac_setup state)
{
	qm_ac_config_t ac_cfg;
	if (state)
	{
		ac_cfg.reference = BIT(WAKEUP_COMPARATOR_PIN);
		ac_cfg.polarity = 0x0;
		ac_cfg.power = BIT(WAKEUP_COMPARATOR_PIN);
		ac_cfg.cmp_en = BIT(WAKEUP_COMPARATOR_PIN);
		ac_cfg.callback = ParkSens_accallback;
		qm_ac_set_config(&ac_cfg);
		QM_IR_UNMASK_INT(QM_IRQ_COMPARATOR_0_INT);
		QM_IRQ_REQUEST(QM_IRQ_COMPARATOR_0_INT, qm_comparator_0_isr);

		qm_pmux_select(QM_PIN_ID_8, QM_PMUX_FN_1);
		qm_pmux_input_en(QM_PIN_ID_8, true);
	} else
	{
		ac_cfg.reference = 0x0;
		ac_cfg.polarity = 0x0;
		ac_cfg.power = 0x0;
		ac_cfg.cmp_en = 0x0;
		ac_cfg.callback = NULL;
		qm_ac_set_config(&ac_cfg);
		QM_IR_UNMASK_INT(QM_IRQ_COMPARATOR_0_INT);
		QM_IRQ_REQUEST(QM_IRQ_COMPARATOR_0_INT, qm_comparator_0_isr);

		qm_pmux_select(QM_PIN_ID_8, QM_PMUX_FN_1);
		qm_pmux_input_en(QM_PIN_ID_8, false);
	}
}

void ParkSens_Wait_After_Detect_Routine(uint8_t * device_addr, uint32_t time, detected_state state)
{
	RTC_WaitInSleepTicks(time);
	bmx1xx_mag_t mag = {0};
	bmx1xx_read_raw_mag(&mag);
	QM_PRINTF("Z: %d Z/16: %d\r\n", mag.z, mag.z/16);
	if (((((mag.z/16)<low_mag_threshold)||(mag.z/16)>high_mag_threshold))&&(state == DS_HIGH))
	{

		ds_state = DS_HIGH;
		ParkSens_Send_Packet(device_addr, ds_state, st);
	} else if ((((mag.z/16)>low_mag_threshold)||((mag.z/16)<high_mag_threshold))&&(state == DS_HIGH))
	{
		ds_state = DS_LOW;
	} else if ((((mag.z/16)>low_mag_threshold)||((mag.z/16)<high_mag_threshold))&&(state == DS_LOW))
	{
		ds_state = DS_LOW;
		ParkSens_Send_Packet(device_addr, ds_state, st);
	} else if (((((mag.z/16)<low_mag_threshold)||(mag.z/16)>high_mag_threshold))&&(state == DS_LOW))
	{
		ds_state = DS_HIGH;
	}
	bmc150_mag_read_reg_int();
}

__attribute__((unused)) void ParkSens_rawmag(void)
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

__attribute__((unused)) void ParkSens_devinit(void)
{
	//M24C64_byte_write((int)STATE_ADDR, (uint8_t)STATE_CALIBRATE);
	//M24C64_byte_write((int)ID_DEVICE_ADDR, (uint8_t)(device_addr>>8));
	//M24C64_byte_write((int)ID_DEVICE_ADDR+1, (uint8_t)(device_addr&0xFF));
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

void ParkSens_Init(void)
{
	mag_init();


	RTC_WaitInSleepTicks(ALARM_SEC*10);
	bmx1xx_mag_t mag = {0};
	bmx1xx_read_raw_mag(&mag);
	low_mag_threshold = (mag.z)/16-THRESHOLD_STEP;
	high_mag_threshold = (mag.z)/16+THRESHOLD_STEP;
	//QM_PRINTF("Z: %d Z/16: %d", mag.z, mag.z/16);
	//QM_PRINTF(" Low: %d High: %d \r\n", low_mag_threshold, high_mag_threshold);
	RTC_WaitInSleepTicks(ALARM_SEC*10);
	bmc150_mag_read_reg_int();
	led_blink(3, DELAY200);
	ds_state = DS_LOW;
}

void ParkSens_TxTest(uint8_t * device_addr)
{
	while(1){
			ParkSens_Send_Packet(device_addr, DS_HIGH, st);
			RTC_WaitTicks(10*QM_RTC_ALARM_SECOND(CLK_RTC_DIV_1));
			ParkSens_Send_Packet(device_addr, DS_LOW, st);
			RTC_WaitTicks(10*QM_RTC_ALARM_SECOND(CLK_RTC_DIV_1));
		}
}

void ParkSens_MainTask(uint8_t * dev_addr)
{
	ParkSens_Send_Packet(dev_addr, ds_state, st);
	if (st==STATE_RUN)
	{
		while(1)
		{
			if (ds_state == DS_LOW)
			{
				ParkSens_High_Thres_Routine(dev_addr);
			}

			if (ds_state == DS_HIGH)
			{
				ParkSens_Low_Thres_Routine(dev_addr);
			}
		}
	}
}

/*void ParkSens_MainTask(uint8_t * dev_addr)
{
	ParkSens_Send_Packet(dev_addr, ds_state, st);
	if (st==STATE_RUN)
	{
		while(1)
		{
			if (ds_state == DS_LOW)
			{
				ParkSens_High_Thres_Routine(dev_addr);
			}

			if (ds_state == DS_HIGH)
			{
				ParkSens_Low_Thres_Routine(dev_addr);
			}
		}
	}
}*/

#include "relay_app.h"

#define PIN_OUT 24
#define RELAY_OUT (5) //8
#define PIN_TOUCH (10) //5
#define RELAY_PIN_ID (QM_PIN_ID_5) //8

#define R_ACK 0xC0
#define R_END 0xB1
#define R_START 0xA1

typedef enum {
	RX_DONE = 0x00,
	TX_DONE = 0x40,
} rxtx_done_t;

typedef enum {
	IRQ_CLEAR,
	IRQ_SET,
} irq_setup;

bool relay_state = false;

void comp_done_callback();
void IntCompSet(irq_setup state, rxtx_done_t irq);

void Init_Relay()
{
	qm_pmux_input_en(RELAY_OUT, false);
	qm_pmux_select(PIN_TOUCH, QM_PMUX_FN_0);
	qm_pmux_input_en(PIN_TOUCH, true);
}

void task_RelayControl(device_t * device_data)
{
	qm_gpio_state_t touch_check_state;
	//uint64_t start_ticks = 0;//get_ticks();
	//uint64_t current_ticks = 0;

	startReceiving();
	IntCompSet(IRQ_SET, RX_DONE);
	power_soc_deep_sleep(POWER_WAKE_FROM_GPIO_COMP);
	//start_ticks = get_ticks();
	IntCompSet(IRQ_CLEAR, RX_DONE);
	qm_gpio_read_pin(QM_GPIO_0, PIN_TOUCH, &touch_check_state);
	if (touch_check_state == QM_GPIO_HIGH)
	{
		uint8_t data_pack[100] = {0};
		uint8_t payload[2] = {0};
		payload[1] = R_ACK;
		if (device_data->relay_stat == 1)
		{
			device_data->relay_stat = 0;
			qm_gpio_clear_pin(QM_GPIO_0, PIN_OUT);
			qm_gpio_clear_pin(QM_GPIO_0, RELAY_OUT);
			payload[0] = 0;
			sprintf((char*)data_pack, "%d,%s,%d,%d,%d", R_START, device_data->device_id, payload[0], payload[1], R_END);
			sendData(data_pack, sizeof(data_pack));
		} else
		{
			device_data->relay_stat = 1;
			qm_gpio_set_pin(QM_GPIO_0, PIN_OUT);
			qm_gpio_set_pin(QM_GPIO_0, RELAY_OUT);
			payload[0] = 255;
			sprintf((char*)data_pack, "%d,%s,%d,%d,%d", R_START, device_data->device_id, payload[0], payload[1], R_END);
			sendData(data_pack, sizeof(data_pack));
		}
	}

	setMode(SX1272_MODE_STANDBY);

	uint8_t data_pack[100] = {0};
	uint8_t payload[2] = {0};
	uint8_t msg[100];
	payload[1] = R_ACK;
	if (!receiveMessage(msg, 2))
	{
		if (strstr((char*)msg,(char*)device_data->device_id)!=NULL)
		{
			if (strstr((char*)msg,"ON")!=NULL)
			{
				device_data->relay_stat = 1;
				payload[0] = 255;
				sprintf((char*)data_pack, "%d,%s,%d,%d,%d", R_START, device_data->device_id, payload[0], payload[1], R_END);
				//rtc_wait_sec(ALARM_SEC);
				clk_sys_udelay(1000000UL);
				sendData(data_pack, sizeof(data_pack));
				//rtc_wait_sec(ALARM_SEC);
				clk_sys_udelay(1000000UL);
				qm_gpio_set_pin(QM_GPIO_0, PIN_OUT);
				qm_gpio_set_pin(QM_GPIO_0, RELAY_OUT);
			} else if (strstr((char*)msg,"OF")!=NULL)
			{
				device_data->relay_stat = 0;
				payload[0] = 0;
				sprintf((char*)data_pack, "%d,%s,%d,%d,%d", R_START, device_data->device_id, payload[0], payload[1], R_END);
				//rtc_wait_sec(ALARM_SEC);
				clk_sys_udelay(1000000UL);
				sendData(data_pack, sizeof(data_pack));
				//rtc_wait_sec(ALARM_SEC);
				clk_sys_udelay(1000000UL);
				qm_gpio_clear_pin(QM_GPIO_0, PIN_OUT);
				qm_gpio_clear_pin(QM_GPIO_0, RELAY_OUT);
			}
		}
	}
}

__attribute__((unused)) void comp_done_callback()
{
	QM_SCSS_INT->int_comparators_host_mask |= BIT(PIN_DIO0) | BIT(PIN_TOUCH);
	QM_PRINTF("KNOCK!\r\n");
}

__attribute__((unused)) void IntCompSet(irq_setup state, rxtx_done_t irq)
{
	qm_ac_config_t ac_cfg;

	writeRegister(REG_DIO_MAPPING_1,irq);
	if (state)
	{
		ac_cfg.reference |= BIT(PIN_DIO0);
		ac_cfg.reference |= BIT(PIN_TOUCH);
		ac_cfg.polarity = 0x0;
		ac_cfg.power |= BIT(PIN_DIO0);
		ac_cfg.power |= BIT(PIN_TOUCH);
		ac_cfg.int_en |= BIT(PIN_DIO0);
		ac_cfg.int_en |= BIT(PIN_TOUCH);
		ac_cfg.callback = comp_done_callback;
		qm_ac_set_config(&ac_cfg);
		qm_irq_request(QM_IRQ_AC, qm_ac_isr);

		qm_pmux_select(QM_PIN_ID_14, QM_PMUX_FN_1);
		qm_pmux_select(PIN_TOUCH, QM_PMUX_FN_1);
		qm_pmux_input_en(PIN_TOUCH, true);
		qm_pmux_input_en(QM_PIN_ID_14, true);
	} else
	{
		ac_cfg.reference = 0x0;
		ac_cfg.polarity = 0x0;
		ac_cfg.power = 0x0;
		ac_cfg.int_en = 0x0;
		ac_cfg.callback = NULL;
		qm_ac_set_config(&ac_cfg);
		qm_irq_request(QM_IRQ_AC, qm_ac_isr);

		qm_pmux_select(QM_PIN_ID_14, QM_PMUX_FN_1);
		qm_pmux_select(PIN_TOUCH, QM_PMUX_FN_1);
		qm_pmux_input_en(PIN_TOUCH, false);
		qm_pmux_input_en(QM_PIN_ID_14, false);
	}
}

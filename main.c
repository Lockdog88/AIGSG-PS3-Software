#include "main.h"
#include "ps_app_v2.h"
#include "relay_app.h"

volatile int int_flag = 0;
volatile int state = 0;
volatile uint32_t int_time = 0;

/*
 * PRKS3 - Main Parking Sensor Task
 * PRKTX - TX Continuous TX
 * RELAY - Relay Task
 */
uint8_t hardware_id[] = "RELAY";//ID_DEVICE_PRESET;
uint8_t device_addr[] = "AiGsGSeR";//ID_DEVICE_PRESET;

microrl_t rl;
microrl_t * prl = &rl;

uint8_t msg[100];



device_t device_data;
memory_t data_mem;
int main(void)
{
	//qm_pmux_select(QM_PIN_ID_4, QM_PMUX_FN_2); /* RTC Clock Out */

	//UART Enable
	qm_pmux_select(QM_PIN_ID_12, QM_PMUX_FN_2); /* configure UART_A_TXD */
	qm_pmux_select(QM_PIN_ID_13, QM_PMUX_FN_2); /* configure UART_A_RXD */
	qm_pmux_input_en(QM_PIN_ID_13, true); /* UART_A_RXD is an input */

	/* Set the GPIO LED. */
	qm_pmux_select(LED_PIN_ID, PIN_MUX_FN);
	qm_pmux_input_en(LED_PIN_ID, false);

	ParkSens_HalInit();
	RTC_HalInit();

	uint8_t state_read_buf = 0x00;
	M24C64_nByteRead((int)STATE_ADDR, &state_read_buf, STATE_SIZE);

	if (state_read_buf == 0x00)
	{
		memcpy(device_data.device_id, device_addr, 8);
		memcpy(device_data.hardware_id, hardware_id, 5);

		memcpy(data_mem.device_id, device_addr, data_mem.device_id_s);
		memcpy(data_mem.hardware_id, hardware_id, data_mem.hardware_id_s);

		M24C64_ByteWrite((int)STATE_ADDR, (uint8_t)0x01);
		M24C64_nByteWrite(0x0001, data_mem.device_id, data_mem.device_id_s);
		M24C64_nByteWrite(0x0001+data_mem.device_id_s, data_mem.hardware_id, data_mem.hardware_id_s);
	} else {
		M24C64_nByteRead(0x0001, device_data.device_id, data_mem.device_id_s);
		M24C64_nByteRead(0x0001+data_mem.device_id_s, device_data.hardware_id, data_mem.hardware_id_s);
	}

	M24C64_nByteRead((int)STATE_ADDR, &state_read_buf, STATE_SIZE);
	if (state_read_buf == 0x01)
	{
		ParkSens_Init(false);
	}

	if (state_read_buf == 0x02)
	{
		M24C64_nByteRead(0x0001+data_mem.device_id_s+data_mem.hardware_id_s+
								 data_mem.ps_stat_s+data_mem.relay_stat_s,
						  &data_mem.mag.x, 4);
		M24C64_nByteRead(0x0001+data_mem.device_id_s+data_mem.hardware_id_s+
								 data_mem.ps_stat_s+data_mem.relay_stat_s+4,
						  &data_mem.mag.y, 4);
		M24C64_nByteRead(0x0001+data_mem.device_id_s+data_mem.hardware_id_s+
					      data_mem.ps_stat_s+data_mem.relay_stat_s+8,
						  &data_mem.mag.z, 4);
	}
	//microrl_init (prl, print);
	//microrl_set_execute_callback (prl, execute);

	//#ifdef _USE_COMPLETE
	// set callback for completion
	//	microrl_set_complete_callback (prl, complet);
	//#endif
	// set callback for Ctrl+C
	//	microrl_set_sigint_callback (prl, sigint);
	QM_PRINTF("Starting. . . \r\n");
	QM_PRINTF("DEVICE SERIAL: %s\r\n", device_addr);

	qm_uart_status_t uart_status __attribute__((unused)) = 0;
	uint8_t msgBase __attribute__((unused)) = 1;

	Radio_Init();
	led_blink(5, 1600);

	if(strcmp((char*)hardware_id, "RELAY") == 0)
	{
		Init_Relay();
		while(1)
		{
			task_RelayControl(&device_data);
		}
	}

	if(strcmp((char*)hardware_id, "PRKTX") == 0)
	{
		ParkSens_TxTest(device_addr);
	}

	if(strcmp((char*)hardware_id, "PRKS3") == 0)
	{
		ParkSens_Init(false);
		ParkSens_MainTask(device_addr);
	}
}

void led_blink(uint8_t count, unsigned long int time)
{
	for(uint8_t i=0; i<count+1; i++)
	{
		qm_gpio_set_pin(QM_GPIO_0, LED_PIN_ID);
		RTC_WaitTicks(time);
		qm_gpio_clear_pin(QM_GPIO_0, LED_PIN_ID);
		RTC_WaitTicks(time);
	}
}



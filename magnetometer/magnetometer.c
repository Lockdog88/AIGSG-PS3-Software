#include "magnetometer.h"

void bmc150_clear_threshold(void)
{
	write_register(BMC150_ADDR, BMC150_REG_INTERRUPT, 0x07);
	write_register(BMC150_ADDR, BMC150_REG_HIGH_THRESHOLD_AXES_SET, 0x3F);
}

void bmc150_mag_set_h_threshold(uint8_t threshold)
{
	/*ENABLE INTERRUPT*/
	write_register(BMC150_ADDR, BMC150_REG_INTERRUPT, 0x47);
	/*SET THRES ON X*/
	write_register(BMC150_ADDR, BMC150_REG_HIGH_THRESHOLD_AXES_SET, 0x1F);//0x37);
	/*SET THRES VALUE (XDATA/16)*/
	write_register(BMC150_ADDR, BMC150_REG_HIGH_THRESHOLD, threshold);
}

void bmc150_mag_set_l_threshold(uint8_t threshold)
{
	/*ENABLE INTERRUPT*/
	write_register(BMC150_ADDR, BMC150_REG_INTERRUPT, 0x47);
	/*SET THRES ON X*/
	write_register(BMC150_ADDR, BMC150_REG_LOW_THRESHOLD_AXES_SET, 0x3B);//0x3E);
	/*SET THRES VALUE (XDATA/16)*/
	write_register(BMC150_ADDR, BMC150_REG_LOW_THRESHOLD, threshold);
}

void bmc150_mag_set_threshold(uint8_t low, uint8_t high)
{
	/*ENABLE INTERRUPT*/
	write_register(BMC150_ADDR, BMC150_REG_INTERRUPT, 0x41);
	/*SET THRES ON X*/
	write_register(BMC150_ADDR, 0x4D, 0x1B);//0x3E);
	/*SET THRES VALUE (XDATA/16)*/
	write_register(BMC150_ADDR, BMC150_REG_LOW_THRESHOLD, low);
	write_register(BMC150_ADDR, BMC150_REG_HIGH_THRESHOLD, high);
}

void mag_init(void)
{
	bmx1xx_setup_config_t cfg;
	cfg.pos = BMC150_J14_POS_0;
	//int mag_init_stat = 0;

	bmx1xx_init(cfg);
	uint8_t data;
	read_register(0x13, 0x40, &data, sizeof(data));
	QM_PRINTF("BMC150 status: 0x%X\r\n", data);
	if (data!=0x32) {qm_gpio_set_pin(QM_GPIO_0, 24);}
	//write_register(0x13, 0x4B, 0x01);
	write_register(0x13, 0x4B, 0x83);
	write_register(0x13, 0x4B, 0x00);
	write_register(0x13, 0x4B, 0x01);
	write_register(0x13, 0x4B, 0x00);
	//bmx1xx_init(cfg);
	bmx1xx_mag_set_power(BMX1XX_MAG_POWER_ACTIVE);
	bmx1xx_mag_set_preset(BMX1XX_MAG_PRESET_LOW_POWER);
	write_register(0x11, 0x11, 0x20);//Suspend accel
	write_register(0x13, 0x51, 0x01);
	write_register(0x13, 0x52, 0x02);

	bmc150_mag_read_reg_int();
}

__attribute__((unused)) void bmc150_mag_callback(void* data)
{

	/*qm_rtc_set_alarm(QM_RTC_0, (QM_RTC[QM_RTC_0].rtc_ccvr + ALARM));*/
	/*if(!qm_uart_read(QM_UART_0, buf)) {QM_PRINTF("BUF: %s\r\n",buf);}*/
	/*bmc150_mag_read_reg_int();*/
}

void bmc150_mag_read_reg_int(void)
{
	uint8_t raw_mag;
	read_register(BMC150_ADDR, 0x4A, &raw_mag, sizeof(raw_mag));
}

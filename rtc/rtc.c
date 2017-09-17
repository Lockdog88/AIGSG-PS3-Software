#include "rtc.h"

void RTC_WaitInSleepTicks(uint32_t time)
{
	RTC_SetAlarm(RTC_SET, time);
	power_soc_deep_sleep(POWER_WAKE_FROM_RTC);
}

void RTC_WaitTicks(uint32_t time)
{
	uint32_t start = QM_RTC[QM_RTC_0].rtc_ccvr;
	while(QM_RTC[QM_RTC_0].rtc_ccvr < (start+time)){}
}

void RTC_SetAlarm(rtc_setup state, uint32_t time)
{
	if(state) qm_rtc_set_alarm(QM_IRQ_RTC_0, QM_RTC[QM_RTC_0].rtc_ccvr+time);
}

void RTC_HalInit(void)
{
	qm_rtc_config_t rtc;

	clk_periph_enable(CLK_PERIPH_RTC_REGISTER | CLK_PERIPH_CLK);

	rtc.init_val = 0;
	rtc.alarm_en = true;
	rtc.alarm_val = 0;
	rtc.callback = NULL;
	rtc.callback_data = NULL;
	qm_rtc_set_config(QM_RTC_0, &rtc);

	qm_irq_request(QM_IRQ_RTC_0, qm_rtc_isr_0);
}

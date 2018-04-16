#ifndef RTC_H_
#define RTC_H_

#include "qm_rtc.h"
#include "qm_isr.h"
#include "qm_interrupt.h"
#include "qm_interrupt_router.h"
#include "qm_soc_regs.h"
#include "../debug.h"
#include "power_states.h"

#define ALARM_SEC  (QM_RTC_ALARM_SECOND(CLK_RTC_DIV_1))
#define ALARM_MIN  (QM_RTC_ALARM_MINUTE(CLK_RTC_DIV_1))
#define ALARM_HOUR (QM_RTC_ALARM_HOUR(CLK_RTC_DIV_1))

typedef enum {
	RTC_CLEAR,
	RTC_SET,
} rtc_setup;

void RTC_WaitInSleepTicks(uint32_t time);
void RTC_WaitTicks(uint32_t time);
void RTC_SetAlarm(rtc_setup state, uint32_t time);
void RTC_HalInit(void);

#endif /* RTC_H_ */

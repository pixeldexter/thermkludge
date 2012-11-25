#ifndef _RTC_H_INC_
#define _RTC_H_INC_

#include <stdint.h>
#include <util/atomic.h>

#if defined(CONFIG_RTC_LONG_JIFFIES)
extern uint32_t rtc_jiffies_32;
#endif
extern uint16_t rtc_jiffies;

#define hz_to_jiffies(ticks) (			\
		((ticks)<<CONFIG_RTC_PRECISION) \
		/ (CONFIG_RTC_HZ)		\
		)

/* return count of full seconds since start of timer */
static __inline__ uint16_t rtc_get_seconds_atomic(void)
{
	return rtc_jiffies >> CONFIG_RTC_PRECISION;
}

static __inline__ uint16_t rtc_get_seconds(void)
{
	uint16_t jiff;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		jiff = rtc_jiffies;
	return jiff >> CONFIG_RTC_PRECISION;
}

extern void rtc_init(void);
extern void rtc_init_final(void);

#endif /* _RTC_H_INC_ */

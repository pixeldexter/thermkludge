#include "rtc.h"
#include "events.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define _rtc_ps ( (CONFIG_RTC_F) / 256U / (CONFIG_RTC_HZ) )
#define _rtc_fract_mask (((1<<(CONFIG_RTC_PRECISION))-1))

/* sanity checks */
#if CONFIG_RTC_PRECISION > 8
#error CONFIG_RTC_PRECISION is too high
#endif

/* compute prescaler */
#if   _rtc_ps == 128
#define _rtc_cs ( _BV(CS22) | _BV(CS20) )
#elif _rtc_ps == 64
#define _rtc_cs ( _BV(CS22) )
#elif _rtc_ps == 32
#define _rtc_cs ( _BV(CS21) | _BV(CS20) )
#elif _rtc_ps == 8
#define _rtc_cs ( _BV(CS21) )
#elif _rtc_ps == 1
#define _rtc_cs ( _BV(CS20) )
#else
#error Unable to match clock divider to a given CONFIG_RTC_F / CONFIG_RTC_HZ combination
#endif

/* compute jiffies increment */
#define _rtc_jiffies_incr ( (1<<(CONFIG_RTC_PRECISION)) / (CONFIG_RTC_HZ) )
#if _rtc_jiffies_incr <= 0
#error Tick increment would be too small, reduce CONFIG_RTC_HZ or increase CONFIG_RTC_PRECISION
#elif _rtc_jiffies_incr > UINT8_MAX+1
#error Tick increment would be too large, increase CONFIG_RTC_HZ or reduce CONFIG_RTC_PRECISION
#endif

/* type-agnostic name for jiffies var */
#if defined(CONFIG_RTC_LONG_JIFFIES)
#define __jiffies_name rtc_jiffies_32
#else
#define __jiffies_name rtc_jiffies
#endif



#if defined(CONFIG_RTC_LONG_JIFFIES)
uint32_t rtc_jiffies_32;
/* uint16_t rtc_jiffies provided by linker script */
#else
uint16_t rtc_jiffies;
#endif



void rtc_init(void)
{
	TIMSK &= ~( _BV(OCIE2) | _BV(TOIE2) );

	ASSR = _BV(AS2);

	TCNT2 = OCR2 = 0;
	TCCR2 = _rtc_cs;
}

void rtc_init_final(void)
{
	// wait until ready
	while ( ASSR & (_BV(TCN2UB) | _BV(OCR2UB) | _BV(TCR2UB)) )
		;

	// reset interrupts and re-enable ticks
	TIFR = _BV(OCF2) | _BV(TOV2);
	TIMSK |= _BV(TOIE2);
}

SIGNAL(TIMER2_OVF_vect)
{
	uint8_t emask = _BV(evt__rtc_tick);

#if 0 /* optimizations */
	/* boilerplate asm code */
	__asm__ (
		"nop" "\n\t"
		: "=r" (emask)
		: "0"  (emask),
		  "e"  (&rtc_jiffies),
		  "M"  (_BV(evt_rtc_seconds)),
		  "M"  (_rtc_fract_mask),
		: "r24"
		);
#elif _rtc_jiffies_incr == 0x100
	__asm__ (
		"ldi  %1, %3"       "\n\t" // always set flag
		"ld   r24, %a2"     "\n\t" // load 2nd byte
		"subi r24, lo8(-1)" "\n\t" // add 1
		"st   %a2+, r24"    "\n\t" // store 2nd byte
		// only if long jiffies are allowed
#if defined(CONFIG_RTC_LONG_JIFFIES)
		"ld   r24, %a2"     "\n\t" // 3rd byte
		"sbci r24, lo8(-1)" "\n\t"
		"st   %a2+, r24"    "\n\t"
		"ld   r24, %a2"     "\n\t" // 4th byte
		"sbci r24, lo8(-1)" "\n\t"
		"st   %a2, r24"     "\n\t"
#endif // defined(CONFIG_RTC_LONG_JIFFIES)
		: "=r" (emask)
		: "0"  (emask),
		  "e"  ((uint8_t*)&rtc_jiffies+1),
		  "M"  (_BV(evt_rtc_second) | _BV(evt_rtc_tick))
		: "r24"
		);
#elif CONFIG_RTC_PRECISION == 8
	__asm__ (
		"ld r24, %a2"        "\n\t" // increment 1st byte
		"subi r24, lo8(-%4)" "\n\t"
		"st %a2+, r24"       "\n\t"
		"brcs 1f"            "\n\t" // if carry, then overflow
		"ori %1, %3"         "\n\t"
		"1:"                 "\n\t"
		"ld r24, %a2"        "\n\t" // 2nd byte
		"sbci r24, hi8(-%4)" "\n\t"
		"st %a2+, r24"       "\n\t"
#if defined(CONFIG_RTC_LONG_JIFFIES)
		"ld r24, %a2"        "\n\t" // 3rd byte
		"sbci r24, hlo8(-%4)""\n\t"
		"st %a2+, r24"       "\n\t"
		"ld r24, %a2"        "\n\t" // 4th byte
		"sbci r24, hhi8(-%4)""\n\t"
		"st %a2, r24"        "\n\t"
#endif // defined(CONFIG_RTC_LONG_JIFFIES)
		: "=r" (emask)
		: "0"  (emask),
		  "e"  (&rtc_jiffies),
		  "M"  (_BV(evt_rtc_second)),
		  "M"  (_rtc_jiffies_incr)
		: "r24"
		);
#elif CONFIG_RTC_PRECISION == 4
	__asm__ (
		"ld r24, %a2"        "\n\t" // increment 1st byte
		"subi r24, lo8(-%4)" "\n\t"
		"st %a2+, r24"       "\n\t"
		"brhs 1f"            "\n\t" // if carry, then overflow
		"ori %1, %3"         "\n\t"
		"1:"                 "\n\t"
		"ld r24, %a2"        "\n\t" // 2nd byte
		"sbci r24, hi8(-%4)" "\n\t"
		"st %a2+, r24"       "\n\t"
#if defined(CONFIG_RTC_LONG_JIFFIES)
		"ld r24, %a2"        "\n\t" // 3rd byte
		"sbci r24, hlo8(-%4)""\n\t"
		"st %a2+, r24"       "\n\t"
		"ld r24, %a2"        "\n\t" // 4th byte
		"sbci r24, hhi8(-%4)""\n\t"
		"st %a2, r24"        "\n\t"
#endif // defined(CONFIG_RTC_LONG_JIFFIES)
		: "=r" (emask)
		: "0"  (emask),
		  "e"  (&rtc_jiffies),
		  "M"  (_BV(evt__rtc_second)),
		  "M"  (_rtc_jiffies_incr)
		: "r24"
		);
#else /* default version that works with all settings */
	const uint8_t fract_save = __jiffies_name & _rtc_fract_mask;
	__jiffies_name += _rtc_jiffies_incr;
	if ( (__jiffies_name & _rtc_fract_mask) <= fract_save ) // overflow
		emask |= _BV(evt_rtc_second);
#endif

	raise_event_bitmap_atomic(emask);
}

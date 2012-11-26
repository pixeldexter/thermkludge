#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <alloca.h>
#include <util/delay.h>
#include <stdint.h>
#include "common/uart.h"
#include "ds18b20/ds18b20.h"
#include "common/rtc.h"
#include "common/events.h"
#include "common/flashdb.h"


/* configuration */
#define __paste__(s1,s2) s1##s2
#define  _paste_(s1,s2)  __paste__(s1,s2)

#define button_port_ _paste_(PORT, CONFIG_BUTTON_GPIO)
#define button_pin_  _paste_(PIN,  CONFIG_BUTTON_GPIO)
#define button_ddr_  _paste_(DDR,  CONFIG_BUTTON_GPIO)
#define button_bit_  _paste_(PIN,  CONFIG_BUTTON_GPBIT)
#define button_ddb_  _paste_(DD,   CONFIG_BUTTON_GPBIT)

#define led_port_ _paste_(PORT, CONFIG_LED_GPIO)
#define led_pin_  _paste_(PIN,  CONFIG_LED_GPIO)
#define led_ddr_  _paste_(DDR,  CONFIG_LED_GPIO)
#define led_bit_  _paste_(PIN,  CONFIG_LED_GPBIT)
#define led_ddb_  _paste_(DD,   CONFIG_LED_GPBIT)



/* communication */
#if   ( CONFIG_UART_BAUDRATE == 38400 )
#define UART_BAUDRATE 12
#elif ( CONFIG_UART_BAUDRATE ==  9600 )
#define UART_BAUDRATE 63
#elif
#error baudrate not supported
#endif



union sensor_id
{
	uint64_t u64;
	uint8_t arr[sizeof(uint64_t)];
};



volatile uint8_t event_flags;
static union sensor_id dev_ids[CONFIG_MAX_SENSORS];
static uint8_t         n_devs;



/*
 * debug
 */

#if 0
static void __attribute__((noinline)) putstr_P(PGM_P str)
{
	__asm__ (
		"1: lpm r24, Z+"     "\n\t"
		"tst r24"            "\n\t"
		"breq 2f"            "\n\t"
		"rcall uart_putchar" "\n\t"
		"rjmp 1b"            "\n\t"
                "2:"                 "\n\t"
		:
		: "z" (str)
		: "r24"
		);
	uart_putchar2('\r\n');
}
#endif

#define CRLF() uart_putchar2('\r\n')



/*
 * LED handling
 */
enum led_state
{
	led_disable,
	led_green,
	led_red,
};

static __attribute__((always_inline)) void set_led(uint8_t state)
{
	switch (state)
	{
	case led_disable:
		led_port_ &= ~_BV(led_bit_);
		led_ddr_  &= ~_BV(led_ddb_);
		break;
	case led_green:
		led_port_ &= ~_BV(led_bit_);
		led_ddr_  |=  _BV(led_ddb_);
		break;
	case led_red:
		led_port_ |=  _BV(led_bit_);
		led_ddr_  |=  _BV(led_ddb_);
		break;
	}
}

static __attribute__((always_inline)) uint8_t get_led(void)
{
	return led_port_ & _BV(led_bit_);
}



/*
 * button
 */

static __attribute__((always_inline)) uint8_t button_is_pressed(void)
{
	return bit_is_clear(button_pin_, button_bit_);
}

static inline void int0_enable_edge(void)
{
	MCUCR &= ~_BV(ISC01);
	MCUCR |=  _BV(ISC00);
}

static inline void int0_enable_level(void)
{
	MCUCR &= ~(_BV(ISC01) | _BV(ISC00));
}

static void button_enable(void)
{
	button_ddr_  &= ~_BV(button_bit_);
	button_port_ |=  _BV(button_bit_); // enable pull-up

#if CONFIG_BUTTON_INTR == INT0
	int0_enable_edge();

	GIFR = _BV(INTF0);
	GICR = _BV(INT0);
#else
#error this CONFIG_BUTTON_INTR configuration is not supported
#endif
}

#if CONFIG_BUTTON_INTR == INT0
ISR(INT0_vect)
#else
#error this CONFIG_BUTTON_INTR configuration is not supported
#endif
{
	raise_event_atomic( button_is_pressed()
			    ? evt__key_press
			    : evt__key_release);
}



/*
 * timers
 */

#define MAX_TIMERS 3

struct timer
{
	uint16_t jiff_next;
	uint16_t jiff_interval;
};

static struct timer g_timers[MAX_TIMERS];

static void cancel_timer(uint8_t timer_id)
{
	if ( timer_id >= MAX_TIMERS ) return;

	g_timers[timer_id].jiff_next = 0;
}

static uint8_t is_running_timer(uint8_t timer_id)
{
	if ( timer_id >= MAX_TIMERS ) return 0;

	return g_timers[timer_id].jiff_next != 0;
}

static void set_alarm(uint8_t timer_id, uint16_t delay)
{
	if ( timer_id >= MAX_TIMERS ) return;

	g_timers[timer_id].jiff_next = rtc_jiffies + delay;
	g_timers[timer_id].jiff_interval = 0;
}

static void set_periodic(uint8_t timer_id, uint16_t delay)
{
	if ( timer_id >= MAX_TIMERS ) return;

	g_timers[timer_id].jiff_next = rtc_jiffies + delay;
	g_timers[timer_id].jiff_interval = delay;
}

static uint8_t process_timers(void)
{
	const uint16_t now = rtc_jiffies;
	uint8_t result = 0, mask = 1;
	struct timer * const end = g_timers + MAX_TIMERS;
#if MAX_TIMERS >= 8
#error too many timers
#endif

	for(struct timer * pt = g_timers; pt != end; ++pt, mask<<=1)
	{
		if ( !pt->jiff_next ) continue;

		if ( now >= pt->jiff_next )
		{
			result |= mask;
			if ( pt->jiff_interval )
				pt->jiff_next = now + pt->jiff_interval;
			else
				pt->jiff_next = 0;
		}
	}

	return result;
}



/*
 * platform deps
 */

static inline void idle_cpu(void)
{
	sei();
	sleep_cpu();
	cli();
}

static void onewire_power_on()
{
	DDRC = _BV(DDC4);
	PORTC = _BV(PC4);
}

void powerup_init(void) __attribute__((naked, section(".init5")));
void powerup_init(void)
{
	TWCR = 0;
	ADCSRA = 0;
	ACSR = _BV(0);
	SFIOR = 0;

	set_led(led_red);

	// move interrupts to start of bootloader
	GICR = _BV(IVCE);
	GICR = _BV(IVSEL);

	// do rest of initialization
	uart_init(UART_BAUDRATE);
	rtc_init();
	button_enable();
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	sleep_enable();
	onewire_power_on();
}



/*
 * Logic
 */

int8_t ow_searchROM(uint8_t *p_id)
{
	static uint8_t *src_addr = 0;

	if ( (uint16_t)src_addr >= E2END )
		return 0;

	uint8_t mask = ~0;
	for(uint8_t i=0; i!=8; ++i)
	{
		*p_id = eeprom_read_byte(src_addr++);
		mask &= *p_id++;
	}
	return ((0xff) != mask) ? 1 : 0;
}

static uint8_t onewire_enum_sensors(void)
{
	int8_t r;
	uint8_t n_devs = 0;

	while ( (r=ow_searchROM(dev_ids[n_devs].arr)) != 0
		&& n_devs < CONFIG_MAX_SENSORS )
	{
		if ( -1 == r )
			break;
		++n_devs;
	}
	return n_devs;
}

uint8_t sample(void)
{
	struct record
	{
		uint16_t time;
		int16_t temp[];
	} __attribute__((packed));

	const uint8_t  count  = (n_devs ? n_devs : 1);
	const size_t   sz_rec = 0
		+ sizeof(struct record)
		+ sizeof(int16_t) * count
		;
	unsigned char buf[sz_rec];
	struct record *p_rec = &buf;

	set_led(led_green);

	// perform simultaneous conversion on all sensors
	ds18b20_selectSensor(0);
	ds18b20_convert();

	// fill structure
	p_rec->time = rtc_get_seconds_atomic();
	for(uint8_t id = 0; id != count; ++id)
	{
		ds18b20_selectSensor( n_devs ? dev_ids[id].arr : 0);
		p_rec->temp[id] = ds18b20_readTemp();
	}


	// write
	set_led(led_red);
	int8_t err = flashdb_write(p_rec, sz_rec);
	set_led(led_disable);
	return err;
}

enum
{
	sample_timer,
	alarm0,
	alarm1,
};

typedef void * (pfn_state)(uint8_t events, uint8_t timers);
void *st_acquire(uint8_t events, uint8_t timers);
void *st_startup(uint8_t events, uint8_t timers);

/* implement a 5-seconds delay and check for full erase sequence */
void *st_startup(uint8_t events, uint8_t timers)
{
	static uint8_t was_pressed;
	static uint8_t init__;
	if ( ! init__ )
	{
		set_led(led_green);
		set_alarm(alarm0,
			  hz_to_jiffies(CONFIG_STARTUP_DELAY * CONFIG_RTC_HZ)
			);
		was_pressed = button_is_pressed();
		init__ = ~0;
	}

	if ( (1<<alarm0) & timers )
	{
		if ( button_is_pressed() && was_pressed )
		{
			set_led(led_red);
			flashdb_erase();
		}
		goto exit_state;
	}

	// premature exit
	if ( is_set_event(events, evt__key_release) )
	{
		if ( was_pressed )
			goto exit_state;
	}

	return 0;

exit_state:
	set_led(led_disable);
	return &st_acquire;
}

/* normal acquisition */
void *st_acquire(uint8_t events, uint8_t timers)
{
	if ( is_set_event(events, evt__rtc_second) )
		set_led(led_disable);

	// hw key processing
	if ( is_set_event(events, evt__key_press) )
		set_alarm(alarm0,
			  hz_to_jiffies(CONFIG_ACTIVATE_DELAY * CONFIG_RTC_HZ)
			);

	if ( is_set_event(events, evt__key_release) )
		cancel_timer(alarm0);

	// state switching
	if ( ((1<<alarm0) & timers)
	     && button_is_pressed() )
	{
		// periodic timer shows whether
		// sampling process is running or not
		if ( is_running_timer(sample_timer) )
		{
			cancel_timer(sample_timer);
			set_led(led_red);
			flashdb_close();
		}
		else
		{
			// running
			if ( flashdb_open(O_APPEND) >= 0 )
			{
				set_periodic(sample_timer,
					     hz_to_jiffies(
						     CONFIG_SAMPLE_INTERVAL
						     * CONFIG_RTC_HZ
						     )
					);
				set_led(led_green);
			}
			else
			{
				set_led(led_red);
			}
		}
	}

	// if needs sampling
	if ( (1<<sample_timer) & timers )
	{
		int8_t r = sample();
		if ( r < 0 )
		{
			cancel_timer(sample_timer);
			set_led(led_red);
			flashdb_close();
		}
	}
	return 0;
}

#if defined(__GNUC__) && defined(__AVR__)
int main(void) __attribute__((OS_main));
#endif
int main(void)
{
	rtc_init_final();
	onewire_enum_sensors();

	pfn_state *think = &st_startup;
	void *nextthink;
	for(;;)
	{
		const uint8_t events = wait_event();
		uint8_t timers = 0;

		if ( is_set_event(events, evt__rtc_tick) )
			timers = process_timers();

		nextthink = (*think)(events, timers);
		if ( nextthink )
			think = (pfn_state*)nextthink;
	}
}

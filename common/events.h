#ifndef _EVENTS_H_INC_
#define _EVENTS_H_INC_

enum events_type
{
	evt__rtc_tick,    /* one tick of realtime clock */
	evt__rtc_second,  /* one second */
	evt__key_press,   /* HW button is pressed */
	evt__key_release, /* HW button is released */
	evt__max,         /* not a real event
			     MUST be the last entry */
};

#include <stdint.h>
#include <util/atomic.h>

extern volatile uint8_t event_flags;

/* allowed from atomic context */
#define raise_event_bitmap_atomic(bm) ({ event_flags |= (bm); })
#define raise_event_atomic(id) ({ raise_event_bitmap_atomic(_BV(id)); })

/* allowed from possible non-atomic contexts */
#define raise_event_bitmap(bm) ({				\
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)	\
			{					\
				raise_event_bitmap_atomic(bm);	\
			}					\
		})						\

/* allowed from possible non-atomic contexts */
#define raise_event(id) raise_event_bitmap( (1<<(id)) )


/* allowed from non-atomic contexts only */
#define wait_event() ({						\
			uint8_t lflags_ = 0;			\
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)	\
			{					\
				for(;;)				\
				{				\
					lflags_ = event_flags;	\
					event_flags = 0;	\
					if ( lflags_ ) break;	\
					idle_cpu();		\
				}				\
			}					\
			lflags_;				\
		})

#define is_set_event(mask, event) ( (mask) & _BV(event) )

#endif /* _EVENTS_H_INC_ */

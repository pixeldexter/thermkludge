/*
 * pointer to bit within an array
 *
 */
#ifndef BITPOINTER_H_INC
#define BITPOINTER_H_INC

#include <stdint.h>

#define BIT_TO_BITPTR(bit) ( 0			\
			     | (((bit)/8)<<8)	\
			     | (1<<((bit)%8))	\
		)

#define BITPTR_OFF(ptr) ( ((ptr)>>8) & 0xFF )

#define BITPTR_MASK(ptr) ( (ptr)&0xFF )

typedef uint16_t bitptr_t;

static __inline__ bitptr_t bitptr_inc(bitptr_t ptr)
{
	__asm__ (
		"clc"     "\n " \
		"rol %A0" "\n " \
		"brcc 1f" "\n " \
                "rol %A0" "\n " \
		"inc %B0" "\n " \
		"1:"      "\n " \
		: "=r" (ptr)    \
		: "0" (ptr)     \
		);
	return ptr;
}

static __inline__ bitptr_t bitptr_dec(bitptr_t ptr)
{
	__asm__ (
		"clc"     "\n " \
		"ror %A0" "\n " \
		"brcc 1f" "\n " \
		"ror %A0" "\n " \
		"dec %B0" "\n " \
		"1:"      "\n " \
		: "=r" (ptr)    \
		: "0"  (ptr)    \
		);
	return ptr;
}

#endif /* BITPOINTER_H_INC */


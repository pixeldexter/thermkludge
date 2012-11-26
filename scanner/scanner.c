#include "ds18b20/ds18b20.h"
#include "common/uart.h"

#if defined(__GNUC__) && defined(__AVR__)
int main(void) __attribute__((OS_main));
#endif
int main(void)
{
	for(;;); // loop forever
}


#include <avr/io.h>

extern void main(void);

__attribute__((naked, section(".vectors"))) void start(void)
{
	asm volatile("ldi r16, %0" ::"i"(RAMEND & 0xff));
	asm volatile("out __SP_L__, r16");
	asm volatile("ldi r16, %0" ::"i"(RAMEND >> 8));
	asm volatile("out __SP_H__, r16");
	asm volatile("clr __zero_reg__"); // r1 set to 0
	main();
}

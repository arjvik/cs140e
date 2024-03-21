#include "rpi.h"

#define pin 8
#define button_pin 10

#define check_button_swi() { if (gpio_read(button_pin)) asm volatile("swi 0" ::: "memory"); }
#define gpio_blink_on() { gpio_set_output(pin); gpio_set_on(pin); }
#define gpio_blink_off() { gpio_set_output(pin); gpio_set_off(pin); }

void notmain(void) {
	// NB: we can't do this b/c the shell already initialized and resetting
	// uart may reset connection to Unix.
	// uart_init();

	gpio_set_off(pin);
	gpio_set_output(pin);
	gpio_set_pulldown(button_pin);
	gpio_set_input(button_pin);

	for (unsigned i = 1; i; i++) {
		printk("%d\n", i);
		delay_ms(1000);
		check_button_swi();
		gpio_blink_on();
		for (unsigned j = 1; j < i; j++) {
			delay_ms(200);
			check_button_swi();
			gpio_blink_off();
			delay_ms(200);
			check_button_swi();
			gpio_blink_on();
		}
		delay_ms(200);
		check_button_swi();
		gpio_blink_off();
	}

	// NB: this is supposed to be a thread_exit().  calling reboot will
	// kill the pi.
	// clean_reboot();
}

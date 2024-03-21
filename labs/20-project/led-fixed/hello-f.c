#include "rpi.h"

void notmain(void) {
	// NB: we can't do this b/c the shell already initialized and resetting
	// uart may reset connection to Unix.
	// uart_init();

	unsigned pin = 8;
	gpio_set_off(pin);
	gpio_set_output(pin);

	for (unsigned i = 1; i; i++) {
		printk("%d\n", i);
		delay_ms(1000);
		gpio_set_on(pin);
		for (unsigned j = 1; j < i; j++) {
			delay_ms(200);
			gpio_set_off(pin);
			delay_ms(200);
			gpio_set_on(pin);
		}
		delay_ms(200);
		gpio_set_off(pin);
	}

	// NB: this is supposed to be a thread_exit().  calling reboot will
	// kill the pi.
	// clean_reboot();
}

/*
 * DEBUG ONLY: boot progress tracer + fault-code blinker for HW-954 bring-up.
 *
 * Checkpoints on the blue LED (P0.15):
 *   2 short blinks -> image entered, C runtime alive (PRE_KERNEL_1)
 *   3 LONG blinks  -> kernel started, timer/LFCLK alive (POST_KERNEL)
 *   solid ON       -> application init completed (APPLICATION)
 *
 * On any Zephyr fatal error: loops forever strobing (reason + 1) rapid
 * flashes, then a long pause. No reboot, so the pattern stays readable.
 *   1 strobe  = CPU exception (hard fault etc.)
 *   2 strobes = spurious interrupt
 *   3 strobes = stack overflow
 *   4 strobes = kernel oops
 *   5 strobes = kernel panic
 */
#include <zephyr/init.h>
#include <zephyr/fatal.h>
#include <hal/nrf_gpio.h>

#define LED 15

static void delay_units(int units)
{
	for (volatile int i = 0; i < 160000 * units; i++) {
		__asm__ volatile("nop");
	}
}

static void blink(int n, int units_on)
{
	nrf_gpio_cfg_output(LED);
	for (int i = 0; i < n; i++) {
		nrf_gpio_pin_set(LED);
		delay_units(units_on);
		nrf_gpio_pin_clear(LED);
		delay_units(2);
	}
}

static int boot_trace_pre_kernel(void)
{
	blink(2, 1); /* 2 short */
	return 0;
}
SYS_INIT(boot_trace_pre_kernel, PRE_KERNEL_1, 0);

static int boot_trace_post_kernel(void)
{
	blink(3, 4); /* 3 long */
	return 0;
}
SYS_INIT(boot_trace_post_kernel, POST_KERNEL, 0);

static int boot_trace_app_done(void)
{
	nrf_gpio_cfg_output(LED);
	nrf_gpio_pin_set(LED);
	return 0;
}
SYS_INIT(boot_trace_app_done, APPLICATION, 99);

/* Override Zephyr's weak fatal handler: strobe the reason, never reboot. */
void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
{
	ARG_UNUSED(esf);
	(void)arch_irq_lock();
	while (1) {
		blink((int)reason + 1, 1);
		delay_units(10);
	}
}

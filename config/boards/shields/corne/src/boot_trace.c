/*
 * DEBUG ONLY: boot tracer + reset-cause reporter for HW-954 bring-up.
 *
 * Every boot, blue LED (P0.15):
 *   2 short blinks                  -> image entered (PRE_KERNEL_1)
 *   then N LONG blinks = why the chip last reset (RESETREAS):
 *     1 = power-on/brownout (no sticky bits)
 *     2 = reset pin
 *     3 = WATCHDOG
 *     4 = soft reset (SREQ)
 *     5 = CPU LOCKUP
 *     6 = other (OFF/LPCOMP/DIF/VBUS wake)
 *   3 medium blinks                 -> kernel started (POST_KERNEL)
 *   solid ON                        -> app init done
 * Zephyr-caught fatal errors: endless rapid strobes of (reason+1).
 */
#include <zephyr/init.h>
#include <zephyr/fatal.h>
#include <hal/nrf_gpio.h>

#define LED 15
#define RESETREAS (*(volatile unsigned int *)0x40000400UL)

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
	unsigned int reas = RESETREAS;
	int code;

	if (reas & (1u << 1)) {
		code = 3; /* watchdog */
	} else if (reas & (1u << 3)) {
		code = 5; /* CPU lockup */
	} else if (reas & (1u << 2)) {
		code = 4; /* soft reset */
	} else if (reas & (1u << 0)) {
		code = 2; /* reset pin */
	} else if (reas == 0) {
		code = 1; /* power-on / brownout */
	} else {
		code = 6; /* other */
	}

	blink(2, 1);      /* checkpoint: alive */
	delay_units(8);
	blink(code, 5);   /* long blinks: last reset cause */
	delay_units(8);

	/* clear sticky bits so the NEXT boot reports only the next reset */
	RESETREAS = 0xFFFFFFFFu;
	return 0;
}
SYS_INIT(boot_trace_pre_kernel, PRE_KERNEL_1, 0);

static int boot_trace_post_kernel(void)
{
	blink(3, 3);
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

void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
{
	ARG_UNUSED(esf);
	(void)arch_irq_lock();
	while (1) {
		blink((int)reason + 1, 1);
		delay_units(10);
	}
}

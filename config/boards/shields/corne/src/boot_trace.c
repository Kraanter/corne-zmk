/*
 * DEBUG ONLY: boot progress tracer for HW-954 clone bring-up.
 *
 * Blinks the blue LED (P0.15) at three boot checkpoints so boot progress
 * is visible without USB or UART:
 *   2 blinks  -> firmware image entered, C runtime alive (PRE_KERNEL_1)
 *   3 blinks  -> kernel started, system timer/LFCLK survived (POST_KERNEL)
 *   solid ON  -> application init completed (APPLICATION)
 *
 * Uses raw HAL + busy loops only, so it depends on no driver or kernel
 * service — it cannot itself be the thing that hangs.
 */
#include <zephyr/init.h>
#include <hal/nrf_gpio.h>

#define BOOT_TRACE_LED_PIN 15

static void crude_delay(void)
{
	for (volatile int i = 0; i < 800000; i++) {
		__asm__ volatile("nop");
	}
}

static void blink(int n)
{
	nrf_gpio_cfg_output(BOOT_TRACE_LED_PIN);
	for (int i = 0; i < n; i++) {
		nrf_gpio_pin_set(BOOT_TRACE_LED_PIN);
		crude_delay();
		nrf_gpio_pin_clear(BOOT_TRACE_LED_PIN);
		crude_delay();
	}
}

static int boot_trace_pre_kernel(void)
{
	blink(2);
	return 0;
}
SYS_INIT(boot_trace_pre_kernel, PRE_KERNEL_1, 0);

static int boot_trace_post_kernel(void)
{
	blink(3);
	return 0;
}
SYS_INIT(boot_trace_post_kernel, POST_KERNEL, 0);

static int boot_trace_app_done(void)
{
	nrf_gpio_cfg_output(BOOT_TRACE_LED_PIN);
	nrf_gpio_pin_set(BOOT_TRACE_LED_PIN);
	return 0;
}
SYS_INIT(boot_trace_app_done, APPLICATION, 99);

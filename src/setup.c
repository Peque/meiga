#include "setup.h"

/**
 * @brief Initial clock setup.
 *
 * Use the Internal High Speed clock (HSI), at 16 MHz, and set the SYSCLK
 * at 168 MHz.
 *
 * The peripheral clocks are set to:
 *
 * - AHB to 168 MHz (max. is 180 MHz)
 * - APB1 to 42 MHz
 * - APB2 to 84 MHz
 *
 * Enable required clocks for the GPIOs and timers as well.
 *
 * @see Reference manual (RM0090), in particular "Reset and clock control for
 * STM32F405xx" section.
 */
static void setup_clock(void)
{
	rcc_clock_setup_hsi_3v3(&rcc_hse_16mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

	/* GPIOs */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Timers */
	rcc_periph_clock_enable(RCC_TIM8);
	rcc_periph_clock_enable(RCC_TIM11);

	/* Enable clock cycle counter */
	dwt_enable_cycle_counter();
}

/**
 * @brief Set SysTick interruptions frequency and enable SysTick counter.
 *
 * SYSCLK is at 168 MHz as well as the Advanced High-permormance Bus (AHB)
 * because, by default, the AHB divider is set to 1, so the AHB clock has the
 * same frequency as the SYSCLK.
 *
 * SysTick interruption frequency is set to `SYSTICK_FREQUENCY_HZ`.
 *
 * @see RM0090 reference manual and in particular the "Clock tree" figure.
 */
static void setup_systick(void)
{
	systick_set_frequency(SYSTICK_FREQUENCY_HZ, SYSCLK_FREQUENCY_HZ);
	systick_counter_enable();
}

/**
 * @brief Enable SysTick interruption.
 */
void enable_systick_interruption(void)
{
	systick_interrupt_enable();
}

/**
 * @brief Disable SysTick interruption.
 */
void disable_systick_interruption(void)
{
	systick_interrupt_disable();
}

/**
 * @brief Initial GPIO configuration.
 *
 * Set GPIO modes and initial states.
 *
 * @see STM32F405RG datasheet and in particular the "Alternate function
 * mapping" section.
 */
static void setup_gpio(void)
{
	/* LEDs */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO0 | GPIO1 | GPIO2 | GPIO3);
	gpio_clear(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3);

	/* Speaker */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
	gpio_set_af(GPIOB, GPIO_AF3, GPIO9);

	/* Motor driver */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO6 | GPIO7 | GPIO8 | GPIO9);
	gpio_set_af(GPIOC, GPIO_AF3, GPIO6 | GPIO7 | GPIO8 | GPIO9);
}

/**
 * @brief Setup PWM for the motor drivers.
 *
 * TIM8 is used to generate both PWM signals (left and right motor):
 *
 * - Edge-aligned, up-counting timer.
 * - Prescale to increment timer counter at 24 MHz.
 * - Set PWM frequency to 24 kHz.
 * - Configure channels 1, 2, 3 and 4 as output GPIOs.
 * - Set output compare mode to PWM1 (output is active when the counter is
 *   less than the compare register contents and inactive otherwise.
 * - Reset output compare value (set it to 0).
 * - Enable channels 1, 2, 3 and 4 outputs.
 * - Enable outputs in the break subsystem.
 * - Enable timer counter.
 *
 * @see Reference manual (RM0090) "Advanced-control timers (TIM1 and TIM8)"
 * and in particular the "PWM mode" section.
 */
static void setup_motor_driver(void)
{
	timer_set_mode(TIM8, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);

	timer_set_prescaler(
	    TIM8, (rcc_apb2_frequency / 24000000 - 1));
	timer_set_repetition_counter(TIM8, 0);
	timer_enable_preload(TIM8);
	timer_continuous_mode(TIM8);
	timer_set_period(TIM8, DRIVER_PWM_PERIOD);

	timer_set_oc_mode(TIM8, TIM_OC1, TIM_OCM_PWM1);
	timer_set_oc_mode(TIM8, TIM_OC2, TIM_OCM_PWM1);
	timer_set_oc_mode(TIM8, TIM_OC3, TIM_OCM_PWM1);
	timer_set_oc_mode(TIM8, TIM_OC4, TIM_OCM_PWM1);
	timer_set_oc_value(TIM8, TIM_OC1, 0);
	timer_set_oc_value(TIM8, TIM_OC2, 0);
	timer_set_oc_value(TIM8, TIM_OC3, 0);
	timer_set_oc_value(TIM8, TIM_OC4, 0);
	timer_enable_oc_output(TIM8, TIM_OC1);
	timer_enable_oc_output(TIM8, TIM_OC2);
	timer_enable_oc_output(TIM8, TIM_OC3);
	timer_enable_oc_output(TIM8, TIM_OC4);

	timer_enable_break_main_output(TIM8);

	timer_enable_counter(TIM8);
}

/**
 * @brief Setup PWM for the speaker.
 *
 * TIM11 is used to generate the PWM signals for the speaker:
 *
 * - Edge-aligned, up-counting timer.
 * - Prescale to increment timer counter at SPEAKER_BASE_FREQUENCY_HZ.
 * - Set output compare mode to PWM1 (output is active when the counter is
 *   less than the compare register contents and inactive otherwise.
 * - Disable output compare output (speaker is off by default).
 * - Enable outputs in the break subsystem.
 *
 * @see Reference manual (RM0090) "General-purpose timers (TIM9 to TIM14)"
 * and in particular the "PWM mode" section.
 */
static void setup_speaker(void)
{
	timer_set_mode(TIM11, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);

	timer_set_prescaler(
	    TIM11, (rcc_apb2_frequency / SPEAKER_BASE_FREQUENCY_HZ - 1));
	timer_set_repetition_counter(TIM11, 0);
	timer_enable_preload(TIM11);
	timer_continuous_mode(TIM11);

	timer_disable_oc_output(TIM11, TIM_OC1);
	timer_set_oc_mode(TIM11, TIM_OC1, TIM_OCM_PWM1);

	timer_enable_break_main_output(TIM11);
}

/**
 * @brief Execute all setup functions.
 */
void setup(void)
{
	setup_clock();
	setup_gpio();
	setup_speaker();
	setup_motor_driver();
	setup_systick();
}

#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <stdint.h>

#include <libopencm3/cm3/dwt.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>

#include "setup.h"

uint32_t read_cycle_counter(void);
uint16_t read_encoder_left(void);
uint16_t read_encoder_right(void);
float get_battery_voltage(void);
float get_motors_voltage(void);
uint8_t mpu_read_register(uint8_t address);
void mpu_write_register(uint8_t address, uint8_t value);
void speaker_on(float hz);
void speaker_off(void);

#endif /* __PLATFORM_H */

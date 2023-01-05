#ifndef __HARD_H
#define __HARD_H

#include <stdbool.h>

void reset_WDT(void);
void on_ps(void);
void off_ps(void);
bool PinPowerEnableState(void);
bool checkStatePower(void);
void calibr_zero_AD712(void);
uint16_t adc1_convertion(void);
uint16_t adc2_convertion(void);

#endif /* __HARD_H */

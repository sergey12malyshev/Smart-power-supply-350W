#ifndef __ADC_H
#define __ADC_H

uint16_t getADC1value(void);
uint16_t getADC2value(void);
void setADC1value(uint16_t adc1_result);
void setADC2value(uint16_t adc1_result);
void calibr_zero_AD712(void);
void setZeroAD712(uint16_t zero);
uint16_t getZeroAD712(void);
uint16_t adc1_convertion(void);
uint16_t adc2_convertion(void);

#endif /* __ADC_H */

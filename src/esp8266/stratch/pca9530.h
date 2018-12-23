#ifndef __PCA9530_H__
#define __PCA9530_H__

#define PCA9530_ADDRESS  0xC0 // in write mode - C1 read 
#define  INPUT     0x00
#define  PSC0      0x01
#define  PWM0      0x02
#define  PSC1      0x03
#define  PWM1      0x04
#define  LSEL      0x05

#define LED0_OFF    LED1_OFF    0b00
#define LED0_ON     LED1_ON     0b01
#define LED0_RATE0  LED1_RATE0  0b10
#define LED0_RATE1  LED1_RATE1  0b11

#define  PWM_CHANNELS    2
#define  PWM_RESOLUTION  8
#define  DAC_CHANNELS    2
#define  DAC_RESOLUTION  8
#define  GPIO_CHANNELS   6
#define  GPIO_BANKS      3
#define  VREF            1 // Unused but must be set for DAC class init, so 1.0 is used


#endif
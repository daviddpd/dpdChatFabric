#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "driver/gpio16.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "mem.h"
#include <salsa20.h>
#include <poly1305-donna.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "esp8266.h"
#include "util.h"
#include "esp-cf-config.h"
#include "esp-cf-wifi.h"
#include "driver/spi.h"
#include "pwm.h"
#include <sys/time.h>
#include "uuuid2.h"
#include <c_types.h>
#include <sys/types.h>


#define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_MTMS_U
#define PWM_0_OUT_IO_NUM 14
#define PWM_0_OUT_IO_FUNC  FUNC_GPIO14

#define PWM_1_OUT_IO_MUX PERIPHS_IO_MUX_MTDO_U
#define PWM_1_OUT_IO_NUM 15
#define PWM_1_OUT_IO_FUNC  FUNC_GPIO15


void CP_ICACHE_FLASH_ATTR 
pwm_setup() 
{
    uint32 pwm_duty_init[2] = {0};

	uint32 io_info[][3] = { 
		{PWM_0_OUT_IO_MUX,PWM_0_OUT_IO_FUNC,PWM_0_OUT_IO_NUM},		
		{PWM_1_OUT_IO_MUX,PWM_1_OUT_IO_FUNC,PWM_1_OUT_IO_NUM}		
	};
	
    set_pwm_debug_en(0);//disable debug print in pwm driver
	
    /*PIN FUNCTION INIT FOR PWM OUTPUT*/
//    pwm_init(pwm_period,  pwm_duty_init ,0,io_info);

    pwm_init(1000, pwm_duty_init ,2,io_info);
	pwm_start();


}
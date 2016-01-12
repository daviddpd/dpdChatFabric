
/*

=================	SCRATCH PAD  ==================================

Left over shift register bits ... didn't want to lose them.



*/

void CP_ICACHE_FLASH_ATTR
changeMode(enum deviceModes m) {
//	os_printf ( " ==> changeMode %d\n", m );

	int i = 0;
	for ( i=7; i>=0; i-- ) {
		shiftBits1[i] = 0;
	}
	
	switch (m) {
		case MODE_UNCONFIGURED:
			shiftBits1[0] = 0x01;
		break;
		case MODE_BOOTING:
			shiftBits1[0] = 0x03;
			shiftBits1[1] = 0x02;		
		break;
		
		case MODE_AP_UNPAIRED:
			shiftBits1[5] = 1;
			shiftBits1[4] = 0x03;			
		break;
		
		case MODE_AP_PAIRED:
			shiftBits1[5] = 1;
			shiftBits1[4] = 1;
		break;

		case MODE_STA_NOWIFI:
			shiftBits1[3] = 1;
			shiftBits1[2] = 0;
			shiftBits1[0] = 0x03;
			shiftBits1[1] = 0x03;
		break;			
		case MODE_STA_UNPAIRED:
			shiftBits1[3] = 1;
			shiftBits1[2] = 0x03;			
		break;
		case MODE_STA_PAIRED:
			shiftBits1[3] = 1;
			shiftBits1[2] = 1;
		break;
		
		case MODE_MENU_IN:
		case MODE_MENU_APMODE:
		case MODE_MENU_STAMODE:
		case MODE_MENU_FACTORYRESET:
		case MODE_MENU_UNPAIRALL:
			shiftBits1[7] = 0x03;
			shiftBits1[6] = 0x02;		
			if ( m == MODE_MENU_APMODE ) 		{ shiftBits1[5] = 1; }
			if ( m == MODE_MENU_STAMODE ) 		{ shiftBits1[3] = 1; }
			if ( m == MODE_MENU_FACTORYRESET )	{ shiftBits1[1] = 1; shiftBits1[0] = 1; }
			if ( m == MODE_MENU_UNPAIRALL ) 	{ shiftBits1[5] = 1; shiftBits1[3] = 1; }
		break;
		default:
			os_printf ( " ==> changeMode %d - mode not found, case default.\n", m );
		break;
	}
/*
	os_printf ( " ==> shiftBits: " );
	for ( i=7; i>=0; i-- ) {
		os_printf ( "%02x ", shiftBits1[i] );
	}
	os_printf ( "\n");	
*/

}



void CP_ICACHE_FLASH_ATTR
shiftReg1() {
	SR_SRCLK = SR_SRCLK2; 
	GPIO_OUTPUT_SET(SR_SRCLK, 0);
	int i;
	for ( i=7; i>=0; i-- ) {
		
		uint8 value = shiftBits1[i] & 0x01;
		uint8 isBlink = shiftBits1[i] & 0x02;
		
		uint8 new_shiftBits = 0;
		if ( isBlink ) {
			new_shiftBits = shiftBits1[i] ^ 0x01;
			shiftBits1[i] = new_shiftBits;
		} 
		
		if ( value ) {
			GPIO_OUTPUT_SET(SR_DATA, 1);
		} else {
			GPIO_OUTPUT_SET(SR_DATA, 0);
		}		

		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 0);
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 1);
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 0);
		
	}

	os_delay_us(10);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		
	os_delay_us(10);
	GPIO_OUTPUT_SET(SR_RCLK, 1);
	os_delay_us(10);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		

//	os_printf("\n");

}

//LOCAL os_timer_t buttonDebounce;
void CP_ICACHE_FLASH_ATTR
shiftReg0() {

	int i;

	if (shiftCounter == -1 ) {
		for ( i=7; i>=0; i-- ) {
			shiftBits0[i] = 0;
			shiftBits1[i] = 0;
		}
		shiftCounter =0;
	} 



	SR_SRCLK = SR_SRCLK1; 
	GPIO_OUTPUT_SET(SR_SRCLK, 0);
	for ( i=7; i>=0; i-- ) {
		
		if ( shiftBits0[i] ) {
			GPIO_OUTPUT_SET(SR_DATA, 1);
		} else {
			GPIO_OUTPUT_SET(SR_DATA, 0);
		}		
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 0);
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 1);
		os_delay_us(5);
		GPIO_OUTPUT_SET(SR_SRCLK, 0);
		
	}

	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		
	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 1);
	os_delay_us(5);
	GPIO_OUTPUT_SET(SR_RCLK, 0);		

}
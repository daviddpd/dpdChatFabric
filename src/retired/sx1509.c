
void CP_ICACHE_FLASH_ATTR 
SX1509_set(uint8 reg, uint8 value) {
	uint8 i = 0;
	uint8 cmds[][3] = { 
		{SX1509_ADDRESS, REG_INPUT_DISABLE_A, 0x0},
		{SX1509_ADDRESS, REG_PULL_UP_A, 0x0},
		{SX1509_ADDRESS, REG_OPEN_DRAIN_A, 0xFF},
		{SX1509_ADDRESS, REG_DIR_A, 0},
		{SX1509_ADDRESS, REG_CLOCK, 0x57},
		{SX1509_ADDRESS, REG_MISC, 0x32},
		{SX1509_ADDRESS, REG_LED_DRIVER_ENABLE_A, 0x10},
		{SX1509_ADDRESS, REG_T_ON_4, 0x1F},
		{SX1509_ADDRESS, REG_OFF_4, 0xFF},
		{SX1509_ADDRESS, REG_T_RISE_4, 0x0F},
		{SX1509_ADDRESS, REG_T_FALL_4, 0x0F},
		{SX1509_ADDRESS, REG_I_ON_4, 0xFF},
		{SX1509_ADDRESS, REG_DATA_A, 0x10},		
	};
	
	for (i=0; i<13; i++)  {
		if ( cmds[i][1] == reg ) {
			i2c_send(cmds[i][0],reg, value);	
		} else {
			i2c_send(cmds[i][0],cmds[i][1], cmds[i][2]);	
		}
		
	}

}

void CP_ICACHE_FLASH_ATTR 
SX1509_Setup() {

/*
- Disable input buffer (RegInputDisable)
- Disable pull-up (RegPullUp)
- Enable open drain (RegOpenDrain)
- Set direction to output (RegDir) – by default RegData is set high => LED OFF
- Enable oscillator (RegClock)
- Configure LED driver clock and mode if relevant (RegMisc)
- Enable LED driver operation (RegLEDDriverEnable)
- Configure LED driver parameters (RegTOn, RegIOn, RegOff, RegTRise, RegTFall)
- Set RegData bit low => LED driver started
*/


	uint8 i = 0;
	uint8 cmds[][3] = { 
		{SX1509_ADDRESS, REG_INPUT_DISABLE_A, 0x0},
		{SX1509_ADDRESS, REG_PULL_UP_A, 0x0},
		{SX1509_ADDRESS, REG_OPEN_DRAIN_A, 0xFF},
		{SX1509_ADDRESS, REG_DIR_A, 0},
		{SX1509_ADDRESS, REG_CLOCK, 0x57},
		{SX1509_ADDRESS, REG_MISC, 0x32},
		{SX1509_ADDRESS, REG_LED_DRIVER_ENABLE_A, 0x10},
		{SX1509_ADDRESS, REG_T_ON_4, 0x1F},
		{SX1509_ADDRESS, REG_OFF_4, 0xFF},
		{SX1509_ADDRESS, REG_T_RISE_4, 0x0F},
		{SX1509_ADDRESS, REG_T_FALL_4, 0x0F},
		{SX1509_ADDRESS, REG_I_ON_4, 0xFF},
		{SX1509_ADDRESS, REG_DATA_A, 0x10},		
	};
	
	for (i=0; i<13; i++)  {
		i2c_send(cmds[i][0],cmds[i][1], cmds[i][2]);	
	}



}

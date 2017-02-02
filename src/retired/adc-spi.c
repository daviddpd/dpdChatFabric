void CP_ICACHE_FLASH_ATTR
adc() {
	if ( 
		hostMeta.hwaddr[0] == 0x18 
		&& hostMeta.hwaddr[1] == 0xfe 
		&& hostMeta.hwaddr[2] == 0x34 
		&& hostMeta.hwaddr[3] == 0xd4 
		&& hostMeta.hwaddr[4] == 0xd3
		&& hostMeta.hwaddr[5] == 0x1d
	
	) {

	int i=0;
	uint16 data0;
	uint16 data1;
	uint32 data0sum = 0;
	uint32 data1sum = 0;
	
	for (i=0; i<20; i++) {
	
		os_timer_disarm(&poketimer);
		data0 = (uint16) spi_transaction(
			HSPI, 
			0, 
			0,
			0, 
			0, 
			4,
			0b1101, 
			12, 
			0);
		
		data0sum+=data0;
		
		data1 = (uint16) spi_transaction(
			HSPI, 
			0, 
			0,
			0, 
			0, 
			4,
			0b1111, 
			12, 
			0);
		data1sum+=data1;

		}
		
//		double voltsPerUnit = 5.22 / 2047;
		double voltsPerUnit = 25.61 /10000;
		uint16 voltsPerUnit_i = (uint16)voltsPerUnit;
		uint16 voltsPerUnit_f = (uint16)((voltsPerUnit - voltsPerUnit_i) *1000);

		double data0volt  = (data0sum/20)*voltsPerUnit;
		uint16 data0volt_i  = (uint16)data0volt;
		uint16 data0volt_f  = (uint16)((data0*voltsPerUnit - data0volt_i ) *1000);


		double data1volt  = (data1sum/20)*voltsPerUnit;
		uint16 data1volt_i  = (uint16)data1volt;
		uint16 data1volt_f  = (uint16)((data1*voltsPerUnit - data1volt_i ) *1000);


		CHATFABRIC_DEBUG_FMT(1, "[perUnit: %02u.%03u]  ADC0: %04x : volts  %02u.%03u : ADC1: %04x : volts  %02u.%03u ",voltsPerUnit_i, voltsPerUnit_f,  data0,  data0volt_i, data0volt_f, data1, data1volt_i, data1volt_f);
		
		os_timer_setfn(&poketimer, (os_timer_func_t *)adc, NULL);
		os_timer_arm(&poketimer, 500, 1);
			
/*

		uint16 cmd = ;
		spi_tx16(HSPI, cmd);
		uint16 data = (uint16) spi_rx16(HSPI);
		CHATFABRIC_DEBUG_FMT(1, "ADC0: %04x", data ); 											

		cmd = 0b0111100000000000;
		spi_tx16(HSPI, cmd);
		data = (uint16) spi_rx16(HSPI);
		CHATFABRIC_DEBUG_FMT(1, "ADC1: %04x", data );
*/
	}
}


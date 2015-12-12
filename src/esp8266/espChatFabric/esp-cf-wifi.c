#include "esp8266.h"
#include "esp-cf-wifi.h"

extern int _GLOBAL_DEBUG;

void udp_callback(void *arg, char *data, unsigned short length);
void tcp_listen(void *arg);

void CP_ICACHE_FLASH_ATTR changeMode(enum deviceModes m);

void CP_ICACHE_FLASH_ATTR
espCfWiFi_listen() {


	c.tcpconn.type = ESPCONN_TCP;
	c.tcpconn.state = ESPCONN_NONE;
	c.tcpconn.proto.tcp = &c.esptcp;
	c.tcpconn.proto.tcp->local_port = config.port;
	espconn_regist_connectcb(&c.tcpconn, tcp_listen);
	espconn_accept(&c.tcpconn);

	c.udpconn.type = ESPCONN_UDP;
	c.udpconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	c.udpconn.proto.udp->local_port = config.port;
	espconn_regist_recvcb(&c.udpconn, udp_callback);
	espconn_create(&c.udpconn);


}




void CP_ICACHE_FLASH_ATTR
espCfWiFi_callBack(System_Event_t *evt)
{

	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "event %02x\n", evt->event);
	switch (evt->event) {
		case EVENT_STAMODE_CONNECTED:
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "connect to ssid %s, channel %d\n",
			evt->event_info.connected.ssid,
			evt->event_info.connected.channel);

			currentMode = MODE_STA_UNPAIRED;
			changeMode(currentMode);
			
		break;
		case EVENT_STAMODE_DISCONNECTED:
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "disconnect from ssid %s, reason %d\n",
			evt->event_info.disconnected.ssid,
			evt->event_info.disconnected.reason);
			currentMode = MODE_STA_NOWIFI;
			changeMode(currentMode);
		break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "mode: %d -> %d\n",
			evt->event_info.auth_change.old_mode,
			evt->event_info.auth_change.new_mode);

		break;
		case EVENT_STAMODE_GOT_IP:
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR "\n",
			IP2STR(&evt->event_info.got_ip.ip),
			IP2STR(&evt->event_info.got_ip.mask),
			IP2STR(&evt->event_info.got_ip.gw));
			ntp_get_time();
			espCfMdns();
			espCfWiFi_listen();
			currentMode = MODE_STA_UNPAIRED;
			changeMode(currentMode);
		break;
		case EVENT_SOFTAPMODE_STACONNECTED:
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "station: " MACSTR "join, AID = %d\n",
			MAC2STR(evt->event_info.sta_connected.mac),
			evt->event_info.sta_connected.aid);
			espCfMdns();

		break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "station: " MACSTR "leave, AID = %d\n",
			MAC2STR(evt->event_info.sta_disconnected.mac),
			evt->event_info.sta_disconnected.aid);
		break;
		default:
			CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "UNKNOWN EVENT:  %02x\n", evt->event );
		break;
	}
}


void CP_ICACHE_FLASH_ATTR
espWiFiInit()
{
    //Set station mode

	if ( config.wifi_ap_switch == 1 && config.wifi_sta_switch == 0 ) 
	{
		config.mode = SOFTAP_MODE;
	} else if ( config.wifi_ap_switch == 1 && config.wifi_sta_switch == 1 ) 
	{
		config.mode = STATIONAP_MODE;
	} else if ( config.wifi_ap_switch == 0 && config.wifi_sta_switch == 1 )
	{
		config.mode = STATION_MODE;
	} else {
		config.mode = SOFTAP_MODE;
	}
    
    wifi_set_event_handler_cb(&espCfWiFi_callBack);
    wifi_set_opmode( config.mode);

	if ( config.mode == SOFTAP_MODE || config.mode == STATIONAP_MODE ) {
		currentMode = MODE_STA_NOWIFI;
		changeMode(currentMode);
		struct ip_info info;
		bzero(&info, sizeof(info) );
		info.ip.addr = config.ap_ipv4;
		info.netmask.addr = config.ap_ipv4netmask;
		info.gw.addr = config.ap_ipv4gw;
		
		wifi_station_dhcpc_stop();
		wifi_softap_dhcps_stop();
/*
		typedef enum _auth_mode {
			AUTH_OPEN = 0,
			AUTH_WEP,
			AUTH_WPA_PSK,
			AUTH_WPA2_PSK,
			AUTH_WPA_WPA2_PSK
		} AUTH_MODE;
		struct softap_config {
			uint8 ssid[32];
			uint8 password[64];
			uint8 ssid_len;
			uint8 channel;
			uint8 authmode;
			uint8 ssid_hidden;
			uint8 max_connection;
			uint16 beacon_interval;  
		};
*/	
		struct softap_config apconfig;
		bzero(&apconfig, sizeof(apconfig) );
		
		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "wifi config hostname: (%d) (%s)", strlen(config.hostname), config.hostname);

		os_memcpy (&apconfig.ssid, &config.hostname[0], strlen(config.hostname) );
		apconfig.ssid_len = strlen(config.hostname);
		if ( config.wifi_ap_ssid[0] != 0 ) 
		{
			os_memcpy (&apconfig.ssid, config.wifi_ap_ssid, strlen(config.wifi_ap_ssid) );
			apconfig.ssid_len = strlen(config.wifi_ap_ssid);
		}
		
		os_memcpy (&apconfig.password, config.wifi_ap_passwd, strlen(config.wifi_ap_passwd) );
		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "wifi password : (%d) (%s)", strlen(config.wifi_ap_passwd), config.wifi_ap_passwd);

		apconfig.authmode = AUTH_WPA2_PSK;
		apconfig.max_connection = 4;
		apconfig.ssid_hidden = 0;

		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "wifi SSID: (%d) (%s)", apconfig.ssid_len, apconfig.ssid);
		
		if ( wifi_softap_set_config(&apconfig) ) {
			CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "wifi_softap_set_config OK ");
		} else {
			CHATFABRIC_DEBUG(_GLOBAL_DEBUG, "wifi_softap_set_config failed ");	
		}

		wifi_set_ip_info(SOFTAP_IF, &info);
		if ( config.wifi_ap_dhcps_switch == 1 ) 
		{
			wifi_softap_dhcps_start();
		}
	}
    
    if ( config.mode == STATIONAP_MODE || config.mode == STATION_MODE ) {
		currentMode = MODE_AP_UNPAIRED;
		changeMode(currentMode);
	    //Set ap settings
    	bzero ( &stationConf.ssid, 32);
	    bzero ( &stationConf.password, 64);
    
	    os_memcpy(&stationConf.ssid, config.wifi_sta_ssid, strlen(config.wifi_sta_ssid));
    	os_memcpy(&stationConf.password, config.wifi_sta_passwd, strlen(config.wifi_sta_passwd));    
	    wifi_station_set_config(&stationConf);
	    if ( config.dhcp_client_switch == 1 )
	    {
	    	wifi_station_dhcpc_start();
	    }
	    
	} 

	espCfWiFi_listen();

	    
}



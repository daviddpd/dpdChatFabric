#include "esp-cf-mdns.h"

int mdns_status = 0;
extern int _GLOBAL_DEBUG;

void CP_ICACHE_FLASH_ATTR
espCfMdns()
{
	
	char buffer2[HOSTNAME_MAX_LENGTH] = {0};

	if (mdns_status) {
		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "Closing mDNS (%d) \n", mdns_status);
		espconn_mdns_close();
		mdns_status = 0;
	}

	struct ip_info ipconfig;
	
	bzero(&ipconfig, sizeof(ipconfig) );

	if ( config.mode == SOFTAP_MODE || config.mode == STATIONAP_MODE ) {
		wifi_get_ip_info(SOFTAP_IF, &ipconfig);	
		CHATFABRIC_DEBUG (_GLOBAL_DEBUG, "softap_if ip \n");
	} else {
		wifi_get_ip_info(STATION_IF, &ipconfig);
		CHATFABRIC_DEBUG (_GLOBAL_DEBUG, "station_if ip \n");	
	}
	
	
	CHATFABRIC_DEBUG (_GLOBAL_DEBUG, "Setting Up mDNS \n");
	CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG, "ip:" IPSTR "\n",  IP2STR(&ipconfig.ip) );
	struct mdns_info *info = (struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
	
	info->ipAddr = ipconfig.ip.addr; //ESP8266 station IP
	info->server_name = "chatFabric";
	info->server_port = config.port;

	if ( config.hostname == NULL ) {
		os_sprintf(buffer2, "%s-%02x:%02x:%02x:%02x:%02x:%02x", "cf",  MAC2STR(hostMeta.hwaddr) );
	} else {
		os_sprintf(buffer2, "%s", config.hostname );
	}

	int len = strlen (buffer2) + 1;

	info->host_name = (char*)malloc(len*sizeof(char));
	bzero(info->host_name, len*sizeof(char));
	os_memcpy(info->host_name, &buffer2, len);

	bzero(buffer2, HOSTNAME_MAX_LENGTH);
	info->txt_data[0] = (char*)malloc(HOSTNAME_MAX_LENGTH*sizeof(char));
	bzero(info->txt_data[0], HOSTNAME_MAX_LENGTH*sizeof(char));	
	os_sprintf(info->txt_data[0], "MAC=%02x:%02x:%02x:%02x:%02x:%02x", MAC2STR(hostMeta.hwaddr) );

	info->txt_data[1] = (char*)malloc(44*sizeof(char));
	bzero(info->txt_data[1], 44*sizeof(char));
	char uuid_str[38] = {0};
	
	snprintf_uuid(&uuid_str[0], 38, &(config.uuid.u0));
	os_sprintf(info->txt_data[1], "uuid0=%s", uuid_str );

	info->txt_data[2] = (char*)malloc(44*sizeof(char));
	bzero(info->txt_data[2], 44*sizeof(char));
	snprintf_uuid(&uuid_str[0], 38, &(config.uuid.u1));
	os_sprintf(info->txt_data[2], "uuid1=%s", uuid_str );
	espconn_mdns_init(info);
	mdns_status=1;
	

}


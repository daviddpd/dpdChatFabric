#include "esp-cf-mdns.h"

struct mdns_info *mdnsinfo;
int mdns_status = 0;
extern int _GLOBAL_DEBUG;

void CP_ICACHE_FLASH_ATTR
espCfMdns()
{
	
	char buffer2[HOSTNAME_MAX_LENGTH] = {0};

	if (mdns_status) {
		CHATFABRIC_DEBUG_FMT(_GLOBAL_DEBUG,  "Closing mDNS (%d)", mdns_status);
		espconn_mdns_close();
		mdns_status = 0;
	}

	struct ip_info ipconfig;
	
	bzero(&ipconfig, sizeof(ipconfig) );	

	if ( config.mode == SOFTAP_MODE ) { 		
		// wifi_get_ip_info(SOFTAP_IF, &ipconfig);	
		ipconfig.ip.addr = config.ap_ipv4;
		CHATFABRIC_DEBUG (_GLOBAL_DEBUG, "softap_if ip");
	} else {
		wifi_get_ip_info(STATION_IF, &ipconfig);
		CHATFABRIC_DEBUG (_GLOBAL_DEBUG, "station_if ip");
	}
	
	wifi_set_broadcast_if(config.mode);
	
	CHATFABRIC_DEBUG_FMT (_GLOBAL_DEBUG, "Setting Up mDNS; ip:" IPSTR " ",  IP2STR(&ipconfig.ip) );
	mdnsinfo = (struct mdns_info *)os_zalloc(sizeof(struct mdns_info));
	
	mdnsinfo->ipAddr = ipconfig.ip.addr; //ESP8266 station IP
//	mdnsinfo->server_name = "chatFabric";
	mdnsinfo->server_name = "exp1";
	mdnsinfo->server_port = config.port;

	if ( config.hostname[0] == 0 ) {
		os_sprintf(buffer2, "%s-%02x:%02x:%02x:%02x:%02x:%02x", "cf",  MAC2STR(hostMeta.hwaddr) );
	} else {
		os_sprintf(buffer2, "%s", config.hostname );
	}

	int len = strlen (buffer2) + 1;

	mdnsinfo->host_name = (char*)malloc(len*sizeof(char));
	bzero(mdnsinfo->host_name, len*sizeof(char));
	os_memcpy(mdnsinfo->host_name, &buffer2, len);

	bzero(buffer2, HOSTNAME_MAX_LENGTH);
	mdnsinfo->txt_data[0] = (char*)malloc(HOSTNAME_MAX_LENGTH*sizeof(char));
	bzero(mdnsinfo->txt_data[0], HOSTNAME_MAX_LENGTH*sizeof(char));	
	os_sprintf(mdnsinfo->txt_data[0], "MAC=%02x:%02x:%02x:%02x:%02x:%02x", MAC2STR(hostMeta.hwaddr) );

	mdnsinfo->txt_data[1] = (char*)malloc(44*sizeof(char));
	bzero(mdnsinfo->txt_data[1], 44*sizeof(char));
	char uuid_str[38] = {0};
	
	uuuid2_to_str(&uuid_str[0], 38, &(config.uuid.u0));
	os_sprintf(mdnsinfo->txt_data[1], "uuid0=%s", uuid_str );

	mdnsinfo->txt_data[2] = (char*)malloc(44*sizeof(char));
	bzero(mdnsinfo->txt_data[2], 44*sizeof(char));
	uuuid2_to_str(&uuid_str[0], 38, &(config.uuid.u1));
	os_sprintf(mdnsinfo->txt_data[2], "uuid1=%s", uuid_str );
	espconn_mdns_init(mdnsinfo);
	mdns_status=1;
	

}


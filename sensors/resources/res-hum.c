#include <stdio.h>
#include <string.h>
#include "coap-engine.h"
#include "coap.h"
#include <stdlib.h>
#include "contiki.h"
#include "os/dev/leds.h"

#include"time.h"

#include "sys/log.h"
#define LOG_MODULE "Humidity sensor"
#define LOG_LEVEL LOG_LEVEL_DBG

int hum_value = 50;
bool ideal_hum = false;
int threshold_hum = 60;

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_hum,
               "title=\"Humidity Sensor: ?POST/PUT hum_thr=<hum_thr>\";rt=\"humidity sensor\";obs",
               res_get_handler,
               res_post_put_handler,
               res_post_put_handler,
               NULL,
               res_event_handler);

static void res_event_handler(void) {
	LOG_DBG("Sending notification observing humidity sensor");
  	coap_notify_observers(&res_hum);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	if(request !=NULL){
		LOG_DBG("Received GET\n");
	}

	unsigned int accept = -1;
	  coap_get_header_accept(request, &accept);

	  if(accept == TEXT_PLAIN) {
	    coap_set_header_content_format(response, TEXT_PLAIN);
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "hum_value=%d,thr_hum=%d", hum_value,threshold_hum);
	    coap_set_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	    
	  } else if(accept == APPLICATION_XML) {
	    coap_set_header_content_format(response, APPLICATION_XML);
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "<hum_value=\"%d\"/><thr_hum=%d/>", hum_value,threshold_hum);
	    coap_set_payload(response, buffer, strlen((char *)buffer));
	    
	  } else if(accept == -1 || accept == APPLICATION_JSON) {
	    coap_set_header_content_format(response, APPLICATION_JSON);
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "{\"hum_value\":\"%d\",\"thr_hum\":\"%d\"}", hum_value,threshold_hum);
	    coap_set_payload(response, buffer, strlen((char *)buffer));
	    
	  } else {
	    coap_set_status_code(response, NOT_ACCEPTABLE_4_06);
	    const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
	    coap_set_payload(response, msg, strlen(msg));
	  }
}

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){  
	
	if(request!=NULL){
		LOG_DBG("Received POST/PUT\n");
	}

	size_t len = 0; 
	const char *text = NULL;
	char hum[100];
    memset(hum, 0, 100);
	
	len = coap_get_post_variable(request, "thr_hum", &text);
	if(len > 0 && len < 100) {
		memcpy(hum, text, len);
		threshold_hum = atoi(hum);
		LOG_DBG("Humidity threshold setted to: %d\n",threshold_hum);
		char msg[50];
	    memset(msg, 0, 50);
		sprintf(msg, "Humidity threshold setted to %d", threshold_hum);
		int length=sizeof(msg);
		coap_set_header_content_format(response, TEXT_PLAIN);
		coap_set_header_etag(response, (uint8_t *)&length, 1);
		coap_set_payload(response, msg, length);
		coap_set_status_code(response, CHANGED_2_04);
	}else{
		coap_set_status_code(response, BAD_REQUEST_4_00);
	}
	
}

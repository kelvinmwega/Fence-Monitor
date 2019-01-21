#ifndef MQTT_H_INCLUDED
#define MQTT_H_INCLUDED

#include <string.h>
#include <stdint.h>

int mqtt_connect_message(uint8_t *, char *, char *, char *);

int mqtt_publish_message(uint8_t *, char *, char *);

int mqtt_disconnect_message(uint8_t *);


#endif // MQTT_H_INCLUDED

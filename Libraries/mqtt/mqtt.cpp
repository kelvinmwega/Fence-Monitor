// MQTT library for C language
// only CONNECT, PUBLISH and DISCONNECT implemented
//
// V1: initial version    	// ates yurdakul, atesh@puaga.com
// V2: 20150527				// reha yurdakul, rehay@tr.ibm.com
//  	support for logon credential added a
//      all functions returns length of the resulting message


#include <mqtt.h>

int mqtt_connect_message(uint8_t * mqtt_message, char * client_id, char * user_id, char * password ) {

  uint8_t i = 0;
  uint8_t msgOffSet = 0;
  uint8_t client_id_length = strlen(client_id);
  uint8_t user_id_length = strlen(user_id);
  uint8_t password_length = strlen(password);
  uint8_t payload_length,payload_extra;
  uint8_t flags=2;
  
  
  if ( client_id_length  > 0 ) { payload_extra = 2;                flags=2;         } 
  if ( user_id_length  > 0   ) { payload_extra = payload_extra +2; flags=flags+128; } 
  if ( password_length > 0   ) { payload_extra = payload_extra +2; flags=flags+64;  } 
  
  payload_length=client_id_length+user_id_length+password_length+payload_extra;
  // fix header
		mqtt_message[0] = 16;                      // MQTT Message Type CONNECT 00010 0000
		mqtt_message[1] = 12 + payload_length;     // Remaining length of the message 

  // variable header
 		mqtt_message[2] = 0;                       // Protocol Name Length MSB
		mqtt_message[3] = 6;                       // Protocol Name Length LSB
		mqtt_message[4] = 77;                      // ASCII Code for M
		mqtt_message[5] = 81;                      // ASCII Code for Q
		mqtt_message[6] = 73;                      // ASCII Code for I
		mqtt_message[7] = 115;                     // ASCII Code for s
		mqtt_message[8] = 100;                     // ASCII Code for d
		mqtt_message[9] = 112;                     // ASCII Code for p
		mqtt_message[10] = 3;                      // MQTT Protocol version = 3
		mqtt_message[11] = flags;
		mqtt_message[12] = 0;                      // Keep-alive Time Length MSB
		mqtt_message[13] = 15;                     // Keep-alive Time Length LSB

  // payload
  // Client ID
		msgOffSet=13;
		mqtt_message[msgOffSet+1] = 0;                      // Client ID length MSB
		mqtt_message[msgOffSet+2] = client_id_length;       // Client ID length LSBc
		for(i = 0; i < client_id_length ; i++){
			mqtt_message[msgOffSet+ 3 + i] = client_id[i];
  }

  // user ID
		if ( user_id_length >0 ) {
			msgOffSet=msgOffSet+ client_id_length + 2;
			mqtt_message[msgOffSet+1] = 0;                      // User ID length MSB
			mqtt_message[msgOffSet+2] = user_id_length;         // User ID length LSB
			for(i = 0; i < user_id_length ; i++){
				mqtt_message[msgOffSet + 3 + i] = user_id[i];
		}
  }

  // password
		if ( password_length >0 ) {  
			msgOffSet=msgOffSet+ user_id_length + 2;
			mqtt_message[msgOffSet+1] = 0;                      // Password ID length MSB
			mqtt_message[msgOffSet+2] = password_length;        // Password ID length LSB
			for(i = 0; i < password_length; i++){
				mqtt_message[msgOffSet + 3 + i] = password[i];
		}
  }

  return 14+ payload_length;
  
} 

int mqtt_publish_message(uint8_t * mqtt_message, char * topic, char * message) {

  uint8_t i = 0;
  uint8_t topic_length = strlen(topic);
  uint8_t message_length = strlen(message);

  mqtt_message[0] = 48;                                  // MQTT Message Type CONNECT
  mqtt_message[1] = 2 + topic_length + message_length;   // Remaining length
  mqtt_message[2] = 0;                                   // MQTT Message Type CONNECT
  mqtt_message[3] = topic_length;                        // MQTT Message Type CONNECT

  // Topic
  for(i = 0; i < topic_length; i++){
    mqtt_message[4 + i] = topic[i];
  }

  // Message
  for(i = 0; i < message_length; i++){
    mqtt_message[4 + topic_length + i] = message[i];
  }
  return 4 + topic_length + i;
}

int mqtt_disconnect_message(uint8_t * mqtt_message) {
  mqtt_message[0] = 0xE0; // msgtype = connect
  mqtt_message[1] = 0x00; // length of message (?)
  return 2;
}




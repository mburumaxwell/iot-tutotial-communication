#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------
 *      Thread 1 'CommunicationThread': Thread that handles communications
 *---------------------------------------------------------------------------*/
 
static void CommunicationThread (void const *argument);			// thread function
osThreadId tid_CommunicationThread;								// thread id
osThreadDef (CommunicationThread, osPriorityNormal, 1, 768);	// thread object

int Init_CommunicationThread (void) {
	tid_CommunicationThread = osThreadCreate (osThread(CommunicationThread), NULL);
	if (!tid_CommunicationThread) return(-1);
	
	return(0);
}

#define CONNECTION_WAITS	10
#define SERVER_NAME "maxwellweru.azurewebsites.net"

static int SendReport(void);

static void CommunicationThread (void const *argument) {
	osEvent evt;
	netStatus stat;
	uint8_t connection_waits;
	HOSTENT* hostentry;
	int err, j;
	IN_ADDR *addr;
	
	while (1) {
		// Wait forever until there is a signal sent to this thread
		evt = osSignalWait(0, osWaitForever);
		// Ensure that the result received is from a signal
		if (evt.status != osEventSignal) 
			continue;

		stat = netPPP_Connect("*99#", "", "");
		if (stat != netOK) {
			printf("PPP connection failed. ret=%d\r\n", stat);
			continue;
		}
		
		// Wait for the PP link to be established
		connection_waits = CONNECTION_WAITS;
		while (connection_waits--) {
			if (netPPP_LinkUp())
				break;
			osDelay(800);
		}
		
		// Do not proceed if the PPP link did not work
		if (!netPPP_LinkUp()) {
			printf("Could not estalish a PPP connection after %dms\r\n", CONNECTION_WAITS*800);
			continue;
		}
		
		// At this point we have a connection now we can do somthing
		// Let us resolve the IP address of the blog
		hostentry = gethostbyname(SERVER_NAME, &err);
		if (hostentry == NULL) {
			if (err == BSD_ERROR_NONAME)
				printf("The host name \'%s\' does not exist\r\n", SERVER_NAME);
			else
				printf("The host name \'%s\' could not be resolved. ret=%d", SERVER_NAME, err);
		} else {
			printf("Hostname \'%s\' resolved to:\r\n", SERVER_NAME);
			for (j = 0; hostentry->h_addr_list[j]; j++) {
				addr = (IN_ADDR *)hostentry->h_addr_list[j];
				printf("IPv4: %d.%d.%d.%d\r\n", addr->s_b1, addr->s_b2, addr->s_b3, addr->s_b4);
			}
			free(hostentry);
		}
		
		// Test MQTT
		SendReport();
		
		
		// We must close the PP link after using it
		printf("CLosing the PPP link\r\n");
		stat = netPPP_Close();
		if (stat != netOK)
			printf("There was a problem closing the PPP link\r\n");
		
				// Wait for the PP link to be established
		connection_waits = CONNECTION_WAITS;
		while (connection_waits--) {
			if (!netPPP_LinkUp())
				break;
			osDelay(400);
		}
		
		if (netPPP_LinkUp())
			printf("Could not close the PPP connection after %dms\r\n", CONNECTION_WAITS*400);
		else
			printf("PPP link was closed\r\n");
	}
}

#include <string.h>
#include "mqtt/MQTTPacket.h"
#include "mqtt_keilds_transport.h"

char *host = "m2m.eclipse.org";
int port = 1883;
char* username = "testuser";
char* password = "testpassword";
MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
int rc = 0;
unsigned char buf[200];
int buflen = sizeof(buf);
int mysock = 0;
MQTTString topicString = MQTTString_initializer;
char* payload = "{\"temp1\":22.3546,\"temp2\":54.1287,\"weight\":4578.125}";
int payloadlen;
int len = 0;
char* topic = "devices/64F7295EA8C/telemetry";

static int SendReport(void) {
	payloadlen = strlen(payload);
	mysock = transport_open(host,port);
	if(mysock < 0)
		return mysock;
	printf("Sending to hostname %s port %d\n", host, port);

	data.clientID.cstring = "th2gv1";
	data.keepAliveInterval = 20;
	data.cleansession = 1;
	data.username.cstring = username;
	data.password.cstring = password;
	data.MQTTVersion = 4;

	len = MQTTSerialize_connect((unsigned char *)buf, buflen, &data);
	
	topicString.cstring = topic;
	len += MQTTSerialize_publish((unsigned char *)(buf + len), buflen - len, 0, 0, 0, 0, topicString, (unsigned char *)payload, payloadlen);

	len += MQTTSerialize_disconnect((unsigned char *)(buf + len), buflen - len);

	rc = transport_sendPacketBuffer(mysock, buf, len);
	if (rc == len)
		printf("Successfully published\n");
	else
		printf("Publish failed\n");
	
//exit:
	transport_close(mysock);
	return 0;	
}

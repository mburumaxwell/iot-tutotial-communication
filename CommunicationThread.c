#include "cmsis_os.h"                                           // CMSIS RTOS header file
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include <stdio.h>

/*----------------------------------------------------------------------------
 *      Thread 1 'CommunicationThread': Thread that handles communications
 *---------------------------------------------------------------------------*/
 
static void CommunicationThread (void const *argument);		// thread function
osThreadId tid_Thread;										// thread id
osThreadDef (CommunicationThread, osPriorityNormal, 1, 0);	// thread object

int Init_CommunicationThread (void) {
	tid_Thread = osThreadCreate (osThread(CommunicationThread), NULL);
	if (!tid_Thread) return(-1);
	
	return(0);
}

#define CONNECTION_WAITS	10
#define SERVER_NAME "maxwellweru.azurewebsites.net"

static void CommunicationThread (void const *argument) {
	osEvent evt;
	netStatus stat;
	uint8_t connection_waits;
	HOSTENT* hostentry;
	int err, i, j;
	IN_ADDR *addr;
	
	while (1) {
		// Wait forever until there is a signal sent to this thread
		evt = osSignalWait(0, osWaitForever);
		// Ensure that the result received is from a signal
		if (evt.status != osEventSignal) 
			continue;

		stat = netPPP_Connect("*99#", NULL, NULL);
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
		}
		
		
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

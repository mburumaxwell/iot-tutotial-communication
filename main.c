#include "LPC43xx.h"                    // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

#ifdef RTE_Network_Interface_PPP
extern int Init_CommunicationThread(void);
extern osThreadId tid_CommunicationThread;
#endif
extern int Init_TLSThread(void);
extern osThreadId tid_TLSThread;

int main(void) {
	osKernelInitialize();       // Initialize CMSIS-RTOS
	SystemCoreClockUpdate();    // Update the clock variable in case we need it
	
#ifdef RTE_Network_Interface_PPP
	netInitialize();            // Initialize the network stack
	osDelay(500);               // Allow the network stack to be initialized
	
//	Init_CommunicationThread();
#endif
	Init_TLSThread();

	osKernelStart();            // Start thread execution
	
    // Signal the communication thread after 5 seconds of running
    // This functionality can be replaced by an kind of interrupt
    // example: RTC wakeup alarm, button press, physical timer timeout etc.
	osDelay(5000);
#ifdef RTE_Network_Interface_PPP
//	osSignalSet(tid_CommunicationThread, 0x01);
#endif
	osSignalSet(tid_TLSThread, 0x01);
}

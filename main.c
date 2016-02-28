#include "LPC43xx.h"                    // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

extern int Init_CommunicationThread(void);
extern int Init_TLSThread(void);
extern osThreadId tid_CommunicationThread, tid_TLSThread;

int main(void) {
	osKernelInitialize();       // Initialize CMSIS-RTOS
	SystemCoreClockUpdate();    // Update the clock variable in case we need it
	
	netInitialize();            // Initialize the network stack
	osDelay(500);               // Allow the network stack to be initialized
	
//	Init_CommunicationThread();
	Init_TLSThread();

	osKernelStart();            // Start thread execution
	
    // Signal the communication thread after 5 seconds of running
    // This functionality can be replaced by an kind of interrupt
    // example: RTC wakeup alarm, button press, physical timer timeout etc.
	osDelay(5000);
//	osSignalSet(tid_CommunicationThread, 0x01);
	osSignalSet(tid_TLSThread, 0x01);
}

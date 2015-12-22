#include "LPC43xx.h"                    // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

int main(void) {
	osKernelInitialize();       // Initialize CMSIS-RTOS
	SystemCoreClockUpdate();    // Update the clock variable in case we need it
	
	netInitialize();            // Initialize the network stack
	osDelay(500);               // Allow the network stack to be initialized
	

	osKernelStart();            // Start thread execution 
}

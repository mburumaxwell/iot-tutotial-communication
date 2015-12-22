#include "LPC43xx.h"                    // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

int main(void) {
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  SystemCoreClockUpdate();				// Update the clock variable incase we need it
	
  osKernelStart();                      // Start thread execution 
}

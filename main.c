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

#include "stdio.h"
void hex_dump(FILE* f, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    if (len == 0 || len < 0) return;

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                fprintf(f, "  %s\n", buff);

            // Output the offset.
            fprintf(f, "  %04x: ", i);
        }

        // Now the hex code for the specific character.
        fprintf(f, " %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        fprintf(f, "   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}


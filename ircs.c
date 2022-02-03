/**
*	@file ircs.c
*	@brief This file contain the functions that are common in the client.c and server.c in order to have more organizated all the project. 
*	The declaration of the functions is in the "ircs.h" file.
*	
*/

#include <taskLib.h>
#include <stdio.h>
#include <kernelLib.h>
#include <semLib.h>
#include <intLib.h>
#include <iv.h>

#include <xlnx_zynq7k.h>
/*#include <lite5200b.h>*/
#include <arch/ppc/ppc5200.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inetLib.h>
#include <sockLib.h>
#include <math.h>


#include "ircs.h"
 
/**
*	This method inicialize all the registers in order to have it all ready.
*	
*/
void irc_init(void)
{
        *(volatile uint32_t *) (ZYNQ7K_GPIO_BASE + 0x00000298) = 0x4; /* reset (stat) */
        *(volatile uint32_t *) (ZYNQ7K_GPIO_BASE + 0x00000284) = 0x0; /* set as input (dirm) */
        *(volatile uint32_t *) (ZYNQ7K_GPIO_BASE + 0x0000029c) = 0x4; /* rising edge (type) */
        *(volatile uint32_t *) (ZYNQ7K_GPIO_BASE + 0x000002a0) = 0x0; /* rising edge (polarity) */
        *(volatile uint32_t *) (ZYNQ7K_GPIO_BASE + 0x000002a4) = 0x0; /* rising edge (any) */
        *(volatile uint32_t *) (ZYNQ7K_GPIO_BASE + 0x00000290) = 0x4; /* enable interrupt (en) GPIO2 */

        intConnect(INUM_TO_IVEC(INT_LVL_GPIO), irc_isr, 0);
        intEnable(INT_LVL_GPIO);
        p_irc = 0;
       
}


/**
*	This method connect an interrupt service routine to the hardware IRQ generated by the motor hardware. 
*	It uses some definitions from xlnx_zynq7k.h header file, which is a part of the BSP.
*	
*/
void irc_isr(void)
{
int sr; /* status register */
sr = *(volatile uint32_t *) (0x43c20000 + 0x0004);
irc_a = (sr & 0x100) >> 8;
irc_b = (sr & 0x200) >> 9;
int irc = 0;
irc = (irc_a << 1) + irc_b;
if ((p_irc == 0 && irc == 1) ||
(p_irc == 1 && irc == 3) ||
(p_irc == 3 && irc == 2) ||
(p_irc == 2 && irc == 0)) motorPos++;
if ((p_irc == 1 && irc == 0) ||
(p_irc == 3 && irc == 1) ||
(p_irc == 2 && irc == 3) ||
(p_irc == 0 && irc == 2)) motorPos--;
p_irc = irc;
*(volatile uint32_t *) (ZYNQ7K_GPIO_BASE + 0x00000298) = 0x4; /* reset (stat) */
/*SPIN=0;*/
}


/**
*	This method disable the interrupt and registers connected previously.
*	
*/
void irc_disable(void)
{
        *(volatile uint32_t *) (ZYNQ7K_GPIO_BASE + 0x00000294) = 0x4; /* disable interrupt (dis) */

        intDisable(INT_LVL_GPIO);
        intDisconnect(INUM_TO_IVEC(INT_LVL_GPIO), irc_isr, 0);
}

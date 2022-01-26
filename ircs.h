/**
*	@file ircs.h
*	@brief This file contain the declaration of the functions that are common in the client.c and server.c in order to have more organizated all the project. 
*	@author ROC BENAIGES MORAGREGA
*/
#ifndef _MYLIB_H_
#define _MYLIB_H_

volatile int irc_a, irc_b;			/**< Value of the IRC input A and B */
volatile int p_irc;
int motorPos;						/**< Value of the position of the motor*/
int recvPos;						/**< Value of the position received of the motor*/

extern void irc_init(void);
extern void irc_isr(void);
extern void irc_disable(void);

#endif

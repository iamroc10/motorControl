/**
*	@file server.c
*	@brief This file represents the server, i.e. the motor that must receive the position of the motor of the client and move to the position received. 
*	
*/


#include <taskLib.h>
#include <stdio.h>
#include <kernelLib.h>
#include <semLib.h>
#include <intLib.h>
#include <iv.h>

#include <xlnx_zynq7k.h>
#include <lite5200b.h>
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

#define SERVER_PORT             80 /* Port 80 is reserved for HTTP protocol */
#define SERVER_MAX_CONNECTIONS  20
#define K                       25/*48*/
#define MAX_SPEED               0b00000000000000000000001100000000
#define CW                      0b01000000000000000000000000000000
#define ACW                     0b10000000000000000000000000000000
#define SPIN                    *(volatile uint32_t *) (0x43c20000 + 0x000C)

SEM_ID move_sem;
volatile int irc_a, irc_b;
int sockd;
struct sockaddr_in my_addr, srv_addr;
struct sockaddr_in my_name, cli_name;
char buf[32];
int addrlen;
int motorPos;
int recvPos;
short dir = -1;
int motorPosBuf[50];
int recvPosBuf[50];
int pointerBuf;


/**
*	This method creates the simple web server and generates the graphs
*	
*/
void www(void)
{
  int i;
  int pos;
  int s;
  int tmpPointer;
  int newFd;
  struct sockaddr_in serverAddr;
  struct sockaddr_in clientAddr;
  int sockAddrSize;

  sockAddrSize = sizeof(struct sockaddr_in);
  bzero((char *) &serverAddr, sizeof(struct sockaddr_in));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  s=socket(AF_INET, SOCK_STREAM, 0);
  if (s<0)
  {
    printf("Error: www: socket(%d)\n", s);
    return;
  }


  if (bind(s, (struct sockaddr *) &serverAddr, sockAddrSize) == ERROR)
  {
    printf("Error: www: bind\n");
    return;
  }

  if (listen(s, SERVER_MAX_CONNECTIONS) == ERROR)
  {
    perror("www listen");
    close(s);
    return;
  }

  printf("www server running\n");

  while(1)
  {
    /* accept waits for somebody to connect and the returns a new file descriptor */
    if ((newFd = accept(s, (struct sockaddr *) &clientAddr, &sockAddrSize)) == ERROR)
    {
      perror("www accept");
      close(s);
      return;
    }

    FILE *f = fdopen(newFd, "w");
    fprintf(f, "HTTP/1.0 200 OK\r\n\r\n");
    fprintf(f, "<html>\
				  <head>\
					<title>Motor status</title>\
				  </head>\
				  <body onload=\"setTimeout(function(){location.reload()}, 100);\">\
				    <svg width=\"600\" height=\"400\" xmlns='http://www.w3.org/2000/svg'>\
					  <g transform=\"translate(50,380) scale(1)\">\
						<!-- Now Draw the main X and Y axis -->\
						<g style=\"stroke-width:2; stroke:black\">\
						  <!-- X Axis -->\
						  <path d=\"M 0 0 L 500 0 Z\"/>\
						  <!-- Y Axis -->\
						  <path d=\"M 0 -360 L 0 0 Z\"/>\
						</g>\
						<g style=\"fill:none; stroke:#B0B0B0; stroke-width:1; stroke-dasharray:2 4;text-anchor:end; font-size:15\">\
						  <text style=\"fill:black; stroke:none\" x=\"-1\" y=\"-360\" >360</text>\
						  <text style=\"fill:black; stroke:none\" x=\"-1\" y=\"0\" >0</text>\
						  <g style=\"text-anchor:middle\">\
							<text style=\"fill:black; stroke:none\" x=\"100\" y=\"20\" >100</text>\
							<text style=\"fill:black; stroke:none\" x=\"200\" y=\"20\" >200</text>\
							<text style=\"fill:black; stroke:none\" x=\"300\" y=\"20\" >300</text>\
							<text style=\"fill:black; stroke:none\" x=\"400\" y=\"20\" >400</text>\
							<text style=\"fill:black; stroke:none\" x=\"500\" y=\"20\" >500</text>\
						  </g>\
						</g>\
						<polyline\
							  points=\"");
    tmpPointer = pointerBuf;
	for (i=0; i<50; i++) {
		if (motorPosBuf[tmpPointer] > 0) pos = (int)ceil((((motorPosBuf[tmpPointer])%514)/514.0)*360-360);
		else                             pos = (int)ceil((((motorPosBuf[tmpPointer])%514)/514.0)*360);
		fprintf(f, "%d, %d\r\n", i*10, pos );
		tmpPointer++;
		if (tmpPointer==50) tmpPointer=0;
	}
	fprintf(f, 							"\"\
							  style=\"stroke:red; stroke-width: 1; fill : none;\"/>\
						<polyline\
							  points=\"");
    tmpPointer = pointerBuf;
	for (i=0; i<50; i++) {
		if (recvPosBuf[tmpPointer] > 0) pos = (int)ceil((((recvPosBuf[tmpPointer])%514)/514.0)*360-360);
		else                            pos = (int)ceil((((recvPosBuf[tmpPointer])%514)/514.0)*360);
		fprintf(f, "%d, %d\r\n", i*10, pos);
		tmpPointer++;
		if (tmpPointer==50) tmpPointer=0;
	}
	fprintf(f, 							"\"\
							  style=\"stroke:blue; stroke-width: 1; fill : none;\"/>\
					  </g>\
					</svg>\
					<svg width=\"600\" height=\"550\" xmlns='http://www.w3.org/2000/svg'>\
					  <g transform=\"translate(50,530) scale(1)\">\
						<!-- Now Draw the main X and Y axis -->\
						<g style=\"stroke-width:2; stroke:black\">\
						  <!-- X Axis -->\
						  <path d=\"M 0 0 L 500 0 Z\"/>\
						  <!-- Y Axis -->\
						  <path d=\"M 0 -500 L 0 0 Z\"/>\
						</g>\
						<g style=\"fill:none; stroke:#B0B0B0; stroke-width:1; stroke-dasharray:2 4;text-anchor:end; font-size:15\">\
						  <text style=\"fill:black; stroke:none\" x=\"-1\" y=\"0\" >0</text>\
						  <text style=\"fill:black; stroke:none\" x=\"-1\" y=\"-500\" >100</text>\
						  <g style=\"text-anchor:middle\">\
							<text style=\"fill:black; stroke:none\" x=\"100\" y=\"20\" >100</text>\
							<text style=\"fill:black; stroke:none\" x=\"200\" y=\"20\" >200</text>\
							<text style=\"fill:black; stroke:none\" x=\"300\" y=\"20\" >300</text>\
							<text style=\"fill:black; stroke:none\" x=\"400\" y=\"20\" >400</text>\
							<text style=\"fill:black; stroke:none\" x=\"500\" y=\"20\" >500</text>\
						  </g>\
						</g>\
						<polyline\
							  points=\"");
    tmpPointer = pointerBuf;
	for (i=0; i<50; i++) {
		fprintf(f, "%d, %d\r\n", i*10, (int)ceil(((abs(motorPosBuf[tmpPointer]-recvPosBuf[tmpPointer])*K)/(double)MAX_SPEED)*-100) );
		tmpPointer++;
		if (tmpPointer==50) tmpPointer=0;
	}
	if (motorPos > 0) pos = (int)ceil((((motorPos)%514)/514.0)*360-360)*-1;
	else              pos = (int)ceil((((motorPos)%514)/514.0)*360)*-1;
	fprintf(f, 							"\"\
							  style=\"stroke:red; stroke-width: 1; fill : none;\"/>\
					  </g>\
					</svg>\
					<br/>\
					<br/>\
					<br/>\
					<br/>\
					<table>\
    		          <tr><td>Actual motor position</td><td>%d</td><tr>", pos);
	if (recvPos > 0) pos = (int)ceil((((recvPos)%514)/514.0)*360-360)*-1;
	else             pos = (int)ceil((((recvPos)%514)/514.0)*360)*-1;
	fprintf(f,       "<tr><td>Received position</td><td>%d</td><tr>\
    				  <tr><td>Motor speed</td><td>%d</td></tr>\
    			    </table>\
				  </body>\
				</html>", pos, abs(motorPos-recvPos)*K);
    
    fclose(f);
  }
}


/**
*	This method creates the connection, creating a UDP socket, configuring the adress
*	
*/
void init_connectionServer() {
	/* Create a UDP socket */
	sockd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockd == -1)
	{
		perror("Socket creation error");
		exit(1);
	}

	/* Configure client address */
	my_name.sin_family = AF_INET;
	my_name.sin_addr.s_addr = INADDR_ANY;
	my_name.sin_port = htons(5000);

	bind(sockd, (struct sockaddr*)&my_name, sizeof(my_name));
    addrlen = sizeof(cli_name);
}


/**
*	This method receives the position of the engine from the client constantly.
*	
*/
void recv_position () {
	while(1) {
		int status = recvfrom(sockd, &recvPos, 32, 0,(struct sockaddr*)&cli_name, &addrlen);
		/*printf("status = %d \n", status);
		printf("buf = %d \n", recvPos);*/
	}
}


/**
 	 *	This method moves the motor in order to the position of the motor and the received position of the other engine are the same. 
 	 *	We have the K constant which is the responsible to the quality of regulation (no oscillations, fast response, minimal steady state error, ...)
 	 *	
*/
void set_position () {
	while(1) {
		  printf("Motor position %d\n", motorPos);
		  printf("Received position: %d\n",recvPos);
		if (recvPos < motorPos) 
		{
			dir = -1;
			unsigned int speed = ((motorPos-recvPos)*K); 
			if (speed > MAX_SPEED) speed = MAX_SPEED;
			SPIN = CW | speed;
			while (1) {
				printf("RM Actual position %d / %d\n", motorPos, recvPos);
				if (motorPos <= recvPos) break;
				speed = ((motorPos-recvPos)*K); 
				if (speed > MAX_SPEED) speed = MAX_SPEED;
				SPIN = CW | speed;
			}
			SPIN=0;
		} 
		else if (recvPos > motorPos) 
		{
			dir = 1;
			unsigned int speed = ((recvPos-motorPos)*K);
			if (speed > MAX_SPEED) speed = MAX_SPEED;
			SPIN = ACW | speed;
			while (1) {
				printf("MR Actual position %d / %d\n", motorPos, recvPos);
				if (motorPos >= recvPos) break;
				speed = ((recvPos-motorPos)*K);
				if (speed > MAX_SPEED) speed = MAX_SPEED;
				SPIN = ACW | speed;
			}
			SPIN=0;
		}
		SPIN=0;
		printf("Final position %d\n", motorPos);
		taskDelay(2);
	}
}


/**
 	 *	This method stores in a buffer all the data of position of the motor and the received position in order to use them then in the web server.
 	 *	
*/
void buffer(void) {
	pointerBuf = 0;
	
	while(1) {
		motorPosBuf[pointerBuf] = motorPos;
		recvPosBuf[pointerBuf] = recvPos;
		pointerBuf++;
		if (pointerBuf == 50) pointerBuf = 0;
		/*printf("MOTORPOS_BUFFER= %d", motorPosBuf[pointerBuf]);
		printf("RECVPOS_BUFFER= %d", recvPosBuf[pointerBuf]);*/
		taskDelay(2);
	}
}


/**
	*This method will be used as a main and will create the tasks and call all the functions and it will manage the motor of the "server" and then delate all the tasks.
	*	
*/
void motor()
{
	TASK_ID wwwTask;		/**< ID of the task wwwTask created by taskSpawn() */
	TASK_ID recvData;		/**< ID of the task recvData created by taskSpawn() */
	TASK_ID moveMotor;		/**< ID of the task moveMotor created by taskSpawn() */
	TASK_ID bufferTask;		/**< ID of the task bufferTask created by taskSpawn() */
	
	motorPos=0;
	pointerBuf = 0;

	/* IRC init */
	irc_init();
	/* webserver task */
	wwwTask = taskSpawn("wwwTask", 210, 0, 4096, (FUNCPTR) www, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	/* PWM init */
	*(volatile uint32_t *) (0x43c20000 + 0x0000) = 0b01000000;
	/* Set PWM freq 20 kHz */
	*(volatile uint32_t *) (0x43c20000 + 0x0008) = 5000;
	/* UDP connection init */
	init_connectionServer();
	
	recvData = taskSpawn("recvData", 50, 0, 4096, (FUNCPTR) recv_position, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	moveMotor = taskSpawn("moveMotor", 40, 0, 4096, (FUNCPTR) set_position, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	bufferTask = taskSpawn("bufferTask", 100, 0, 4096, (FUNCPTR) buffer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
	/*while(1);
	
	taskDelay(10000);*/
	getchar();
	printf("Out of play time.\n");
	
	irc_disable();
	taskDelete(bufferTask);
	taskDelete(recvData);
	taskDelete(moveMotor);
    taskDelete(wwwTask);
}

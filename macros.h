#ifndef _MACROS_H_
#define _MACROS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define FLAG 			0x7E
#define FLAG_STUFFING	0x5E
#define ESCAPE			0x7D
#define ESCAPE_STUFFING 0x5D
#define A_EMI_REC 		0x03
#define A_REC_EMI 		0x01 //TRAMAS DISC
#define C_SET 			0x07
#define C_DISC			0x0B
#define C_UA			0x03
#define C_RR			0x21
#define C_REJ			0x23

#define TRANSMITTER 0
#define RECEIVER 1

#define BAUDRATE B9600
#define DISPOSITIVO "/dev/ttyS"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define MAX_RETRIES 3
#define ALARM_SPAN 3

typedef enum { SET, DISC, UA, RR, REJ, I } typeFrame;

typedef struct frame_struct {
	 char flag;
	 char a;
	 char c;
	 char bcc;
	 char flag2;
} frame;


int (*stateFunc)(char c,typeFrame f);


#endif
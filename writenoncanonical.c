/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x07

volatile int STOP=FALSE;
int conta=1,flag = 1;


void start(char c);
void flag_RCV(char c);
void A_RCV(char c);
void C_RCV(char c);
void BCC(char c);
void stop(char c);
void (*stateFunc)(char c) = start;

struct trama_SET {
	 char flag;
	 char a;
	 char c;
	 char bcc;
	 char flag2;
};

struct trama_SET trama;
int pos_ack = 0;

void clean_trama(){

     trama.flag = 0;
     trama.a = 0;
     trama.c = 0;
     trama.bcc =0;
     trama.flag2 = 0;	
}
	
void start(char c){
	printf("start\n");   		
	
	if(c == FLAG){
		stateFunc = flag_RCV;
		trama.flag = c;
	}
}

void flag_RCV(char c){
printf("flag_RCV\n");	

     if(c == A){
	stateFunc = A_RCV;
        trama.a = A;
     }
     else if ( c == FLAG)
	stateFunc = flag_RCV;
     else 
	stateFunc = start;
}

void A_RCV(char c){
printf("A_RCV\n");	
	if(c == C_SET){
	stateFunc = C_RCV;
	trama.c = c;	
	}
     else if ( c == FLAG)
	stateFunc = flag_RCV;
     else 
	stateFunc = start;
}

void C_RCV(char c){
printf("C_RCV\n");	
     if( c == (trama.a^trama.c)){
	stateFunc = BCC;
	trama.bcc = c;
}
     else if ( c == FLAG)
	stateFunc = flag_RCV;
     else 
	stateFunc = start;
}

void BCC(char c){
printf("BCC\n");	 
   if (c == FLAG){
	trama.flag2 = FLAG;
	stateFunc = stop;
}
     else 
	stateFunc = start;
}

void stop(char c){
printf("stop\n");		
pos_ack = 1;
	printf("recebeu no emissor\n");
}


void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}

//================================

int main(int argc, char** argv)
{
    (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer 10 ms */
    newtio.c_cc[VMIN]     = 0;   /* non blocking read*/



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    //tp2
	unsigned char SET[5];
	SET[0]=FLAG;
	SET[1]= A; //A
	SET[2]= C_SET; //C
	SET[3]= SET[1]^SET[2]; //BCC
	SET[4]= FLAG;

	int v ;
	char ch;

	while(conta < 4){
		
		if(flag){
		   alarm(3);                 // activa alarme de 3s
		   flag=0;
		}  	
		
		v = write(fd,SET,5); //envia trama	
		printf("escreve %d bytes:\n",v);			
		
		while(pos_ack == 0){
		     	res = read(fd,&ch,1);
			printf("char recebido: %d \n",ch);
			stateFunc(ch);
			if(flag==1)
			   break;
		}
		if(pos_ack == 1)  //recebe trama correctamente
			break;		
	}




/*
    char ch;
    for (i = 0; i < 255; i++) {
	
	ch= (char) getchar();      
	if(ch == '\n'){
		buf[i]='\0';
		break;
	}
	else{
	buf[i]=ch;
	
    	}	
	}
     
    res = write(fd,buf,255);   
    printf("%d bytes written\n", 

	res = 0;
while (STOP==FALSE) { 
res += read(fd,buf+res,1);
if (buf[res-1] == '\0') {
STOP = TRUE;
}
}
printf("%s\n",buf);	
printf("%d bytes written\n", res);
*/




	sleep(5);

   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}

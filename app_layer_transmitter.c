#include "macros.h"
#include "link_layer.h"
#include "app_layer_utils.h"

#define P_HEADER_SIZE 	4 			/* BYTES */
#define NUM_ARGS 		2


int main(int argc, char** argv){
  	
	/* VERIFICACOES */

	if(argc < NUM_ARGS + 1){
		printf("Usage: app [PORT] [FILENAME] .\n");
		return -1;
	}

	if(argv[1] < 0){
		printf("Insert correct port value.\n");
		return -1;
	}	
	int port = atoi(argv[1]);		
	 
	menu(TRANSMITTER);  //MENU
	
			
  //===================
  
  /* OPEN PORT AND CONNECTS */
  
  int fd;
  
  if ( (fd = llopen(port,TRANSMITTER)) < 0) { 	
	printf("Error on llopen.\n");
	return -1;
  }
  
  /* OPEN FILE */
  int file_fd;
  if((file_fd = open(argv[2],O_RDONLY)) < -1){
	  printf("Erro a abrir ficheiro %s.\n", argv[NUM_ARGS]);
	  return -1;
  }

  //FILE'S INFO STRUCT
  struct stat fileStat;
  if(fstat(file_fd,&fileStat) < 0){
	  printf("Erro a obter struct filestat.\n");
	  return -1;	  
  }    
  
  int file_size = fileStat.st_size; //TAMANHO DO FICHEIRO

  //====================================== 
  /* 	 	DATA TRANSMISSION 			*/
  
  int seq_actual = 0;
	
  /* PACOTE DE CONTROLO (START) */
  control_packet c_packet_start;  //c-control d-data
  fillControlPacket(&c_packet_start);
  
  c_packet_start.length_filename = strlen(argv[2]) + 1;  //FILENAME SIZE
  c_packet_start.value_filename = argv[2];   //FILENAME 
  
  c_packet_start.length_file_length = sizeof(file_size);

  //converter inteiro em array de bytes
  unsigned char v[sizeof(file_size)]; 
  int i;
  for(i = 0; i < sizeof(file_size); i++){		
		v[i] = (file_size >> 8*i) & 0x00FF;
  }
  
  //em cada posicao do array esta um byte do tamanho shiftado
  c_packet_start.value_file_length = v;
  
  /* 		SENDING DATA CONTROL FRAME	     	*/
  //TODO: VERIFICAR RETORNO DO LLWRITE
  char packet_start[40];
  
  int size = changeToArray(c_packet_start, packet_start);

  if(llwrite(fd, packet_start, size) < 0) {//envio de packet start
	  printf("llwrite:packet start error.\n");
	  return -1;
  }

  printf("packet start sent\n");  
  
  /* ENVIO DE SEGMENTOS DO FICHEIRO */  
  
  char data_packet[P_HEADER_SIZE + max_data_field], 
				buffer[max_data_field];
				
  int chs_read, stop = 0; 
  
  //TODO MODIFICAR CONDICAO DO CICLO
  while(stop == 0){
 
	/* TRIES TO READ max_data_field BYTES FROM FILE */
	chs_read = read(file_fd,&buffer, max_data_field);  
	
	if(chs_read < 0){
		printf("Erro a ler ficheiro.\n");
		return -1;
	}else if(chs_read < max_data_field){ //terminou ficheiro  Divisao do ficheiro em segmentos
		stop = 1;	//TERMINAR CICLO //TODO VERIFICAR ISTO	
	}	
		
	/* 	DATA PACKET HEADER 	*/
	data_packet[0] = 0;
	data_packet[1] = seq_actual;
	data_packet[2] = (0xFF00 &  chs_read) >> 8; //L2
	data_packet[3] = 0x00FF & chs_read;         //L1
	
	//incorpora fragmento do ficheiro |> I = [PH|___]
	memcpy(&data_packet[4], buffer, chs_read); //data_packet = [PH|DATA]
	
	//=================================
	/* ENVIO DE DADOS */
	
	int d_packet_length = P_HEADER_SIZE + chs_read; //PH_size + numero de chars lidos
	
	if(llwrite(fd,data_packet, d_packet_length) < 0){
		printf("ERROR ON LLWRITE.\n");
		return -1;
	}

	seq_actual++;	
  }
 
 
  //ENVIA PACOTE DE CONTROLO (END)
  c_packet_start.control_field = (char)2;  //2 - END
  size = changeToArray(c_packet_start, packet_start);
  
  if(llwrite(fd, packet_start, size) < 0){//envio de packet start
	  printf("llwrite:packet end error.\n");
	  return -1;
  }
  //=====================================
  
  if(llclose(fd, TRANSMITTER) < 0){
	printf("Error on llclose.\n");
	return -1;	
	}
  
 return 0; 
}

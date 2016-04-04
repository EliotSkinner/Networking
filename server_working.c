#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>

#define FILENAME "TestFile"
#define PSIZE 256
#define PORT 10060
int corruptProb, packetnum, dataloss;
FILE *file;
int sock, length, fromlen, n;
struct sockaddr_in server;
struct sockaddr_in from;
struct stat file_status;
char buf[1024];


void error(char *msg)
{
	perror(msg);
	exit(0);
}

void timeoutOccur(unsigned int msec)
{
  clock_t set = msec + clock();
  while (set > clock());
}

int readfile()
{
  if (stat(FILENAME, &file_status) != 0)
    {
      perror("Error: in file read");
    }
  printf("Starting file read...");
  file = fopen(FILENAME, "r");

  char *buffer = NULL;
  buffer = (char *)malloc(file_status.st_size +1);

  if(!fread(buffer, file_status.st_size, 1, file));
  {
    perror("Error: couldn't read the given file");
  }
  
  buffer[file_status.st_size +1] = '\0';
  createpacket();

  fclose(file);
  free(buffer);
  return 0;
}

int createpacket()
{
  int loss;
  char msg[PSIZE];
  char c, seqNum;
  file = fopen(FILENAME, "r");
  
  seqNum = '0';
  int characters_read = 0;
  int position_in_buffer = 11;
  
  msg[1] = '7';
  msg[2] = '7';
  msg[3] = '7';
  msg[4] = '7';
  msg[5] = '7';
  msg[6] = '7';
  msg[7] = '7';
  msg[8] = '7';
  msg[9] = '7';
  msg[10] = '1';
	
  packetnum = 1;

  while((c=fgetc(file)) != EOF)
  {
    if(characters_read == 0)
    {
      printf("\nWriting file into packet %d\n", packetnum);
    }
    msg[0] = seqNum;
    characters_read++;
    msg[position_in_buffer] = c;
    position_in_buffer++;


    if(characters_read==244)
    {
      printf("Packet filled, packet number %d\n", packetnum);
      printf("Packet of file reads:\n%s\n(%lu bytes).\n", msg, sizeof(msg));
      printf("Applying gremlin function\n");
      loss = gremlin(msg);
      if(loss == 3)
      {
        printf("Your packet has been lost, MUUUHAHA!\n");
        printf("Timeout occurring\n");
        timeoutOccur(30);
        printf("Packet resent");
        sendto(sock,msg,PSIZE,0,(struct sockaddr *)&from,fromlen);
      }
      else 
      {
        sendto(sock,msg,PSIZE,0,(struct sockaddr *)&from,fromlen);
      }
      printf("Packet %d Sent\n\n\n", packetnum);

      characters_read=0;
      position_in_buffer =11;

      recvfrom(sock,msg,PSIZE,0,(struct sockaddr *)&from,fromlen);
      

      while(msg[0] == '1')
      {
        packetnum++;
	printf("\nPacket Corrupted Caught\n");
	msg[0] = '0';
	msg[1] = '7';
	msg[2] = '7';
	msg[3] = '7';
	msg[4] = '7';
	msg[5] = '7';
	msg[6] = '7';
	msg[7] = '7';
	msg[8] = '7';
	msg[9] = '0';
	msg[10] = 'A';
	
	printf("Packet filled, packet number %d\n", packetnum);
	printf("Packet of file reads:\n%s\n(%lu bytes).\n", msg, sizeof(msg));
	printf("Applying gremlin function\n");
	gremlin(msg);
	sendto(sock,msg,PSIZE,0,(struct sockaddr *)&from,fromlen);
	printf("Packet %d Sent\n\n\n", packetnum);
	
	recvfrom(sock,msg,PSIZE,0,(struct sockaddr *)&from,fromlen);
      }
      packetnum++;
    }
  }
  
  if((position_in_buffer != 11) || (position_in_buffer != 255))
  {
    int i;
    int count_null_characters =0;

    for(i = PSIZE-1;i > (position_in_buffer-1); i--)
    {
      count_null_characters++;
      msg[i]='\0';
    }

    if(count_null_characters)
    {
      msg[255] = '*';
      printf("Not Enough Data to Completely Fill Packet Yo\n");
      printf("INserted %i null characters to pad the packet\n", count_null_characters);
      printf("Last Packet Reads:\n");
      sendto(sock,msg,PSIZE,0,(struct sockaddr *)&from,fromlen);
      printf("%s", msg);
      printf("\nPacket %d Sent\n\n\n", packetnum);
    }
  }

  fclose(file);
  return 0;

}

int gremlin(char pack[])
{
  int r, degree, x;
  r = rand() % 10;
  x = rand() % 10;


  if(r <= corruptProb)
  {
    degree = rand() % 10;
    if(degree <= 7)
    {
      pack[1] = '8';
      printf("one bit has been damaged\n");
    }
    else if((degree > 7) & (degree < 10))
    {
    
      pack[1] = '8';
      pack[2] = '8';
      printf("two bits have been damaged\n");
    }
    else 
    {
      pack[1] = '8';
      pack[2] = '8';
      pack[3] = '8';
      printf("three bits have been damaged\n");
    }
  }
  if(x <= dataloss)
  {
    return 3;
  }

  return 0;
}
	

int main(int argc, char *argv[])
{
  
  if (argc > 1)
  {
    fprintf(stderr, "ERROR");
    exit(0);
  }
  printf("Enter a number for damage probability: ");
  scanf("%d", &corruptProb);
  fprintf(stderr, "Damage Probability = %d", corruptProb);
  printf("\nEnter a number for damage probability: ");
  scanf("%d", &dataloss);
  fprintf(stderr, "Loss Probability = %d", dataloss);
  fprintf(stderr, "\nWaiting for GET request from client...\n");  
  sock=socket(AF_INET, SOCK_DGRAM, 0);
  
  if (sock < 0)
  {
    error("Opening socket");
  }
  
  length = sizeof(server);
  bzero(&server,length);
  server.sin_family=AF_INET;
  server.sin_addr.s_addr=INADDR_ANY;
  server.sin_port=htons(PORT);
  
  if (bind(sock,(struct sockaddr *)&server,length)<0)
    {
      error("binding");
    }
  
  fromlen = sizeof(struct sockaddr_in);
  
  /* printf("Waiting for GET request from client..."); */
  while (1)
    {
      n = recvfrom(sock,buf,1024,0,(struct sockaddr *)&from,&fromlen);
      
      if (n < 0)
	{
	  error("recvfrom");
	}
      
      write(1,"Received GET request from client for file...\n",44);
      n = sendto(sock,"Got your file request...packaging file now...\n",48,
		 0,(struct sockaddr *)&from,fromlen);
      
      readfile();
      fprintf(stderr, "File has been read and sent\n");
      if (n < 0)
	{
	  error("sendto");
	    }
    }
}

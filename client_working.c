#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdio.h>

#define MESSAGE "TestFile"
#define PSIZE 256
#define PORT 10060

int sock, length, n;
struct sockaddr_in server, from;
struct hostent *hp;
FILE *ofp;
char buffer[256];
char msg[1052];
int loop;


void error(char *);

int calculateCheckSum(char pack[])
{
  int sum = 0;
  int i = 0;
  for(i = 1;i < 11; i++)
  {
    sum = pack[i] + sum;
  }
  if(sum % 7 == 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }

}

int main(int argc, char *argv[])
{
	loop = 0;
	if (argc != 2)
	{
		printf("Usage: server\n");
		exit(1);
	}

	sock= socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
	{
		error("socket");
	}
	
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);

	if (hp==0)
	{
		error("Unknown host");
	}

	bcopy((char *)hp->h_addr, (char *)&server.sin_addr,hp->h_length);
	server.sin_port = htons(PORT);
	length=sizeof(struct sockaddr_in);

	printf("Sending Message...");
	printf("Your message = [GET %s]\n", MESSAGE);
	

	bzero(buffer,256);
	buffer[255] = MESSAGE;

	n=sendto(sock,buffer,strlen(buffer),0,&server,length);
	
	if (n < 0)
	{
		error("Sendto");
	}
	n = recvfrom(sock,buffer,256,0,&from, &length);

	if (n < 0)
	{
		error("recvfrom");
	}

	write(1,"Got an ack: ",12);
	write(1,buffer,n);
	
	fprintf(stderr, "\nServer is sending the file!\n");
	fprintf(stderr, "Let's start writing it!\n");

	while(loop==0)
	{
	  ofp=fopen("write_file", "w");
	  if(ofp==NULL)
	  {
	    printf("Can't open output file");
	    exit(1);
	  }
	  char c;
	  int packetCount = 1;
	  while(c != '*')
	  {
	    int passed;

	    recvfrom(sock, msg, PSIZE, 0, &from, &length);

	    fprintf(stderr, "\nReceived Packet %d\n", packetCount);
	    packetCount++;
	    printf("Checking packet...\n");
	    passed = calculateCheckSum(msg);
	    if(passed == 1) 
	    {
	      fprintf(stderr, "Packet Valid\n");
	      int i;
	      int s = 11;
	      for(i=11; i<PSIZE; i++)
	      {
	      	s++;
	      	if (msg[i] != '\0') 
	      	{
	      		if(s != PSIZE){
	      			fputc(msg[i], ofp);
	      			c = msg[i];
	      		}
	      		else {
	      			c = msg[i];
	      		}
	      	}

          }
	   
	      sendto(sock, msg, PSIZE, 0, &server, length);
	    }
	    else
	    {
	      fprintf(stderr, "Packet Corrupted yo!\n");
	      msg[0] = '1';
	      sendto(sock, msg, PSIZE, 0, &from, length);
	    }
	  }
	fclose(ofp);
	loop++;
     }
     printf("Finished writing! Let's see what we got...\n");
     
     ofp=fopen("write_file", "r");
     if(ofp == NULL)
     {
       printf("Can't open output file");
       exit(1);
     }
     char c;
     fseek(ofp, 0L, SEEK_END);
     int size = ftell(ofp);
     fseek(ofp, 0L, SEEK_SET);
     printf("\n\nFile from the server reads: \n");
     int pos = 0;
     while((c=fgetc(ofp)) != EOF)
     {
       if(pos < size-1)
       {
	 printf("%c", c);
       }
       pos++;
     }

     fclose(ofp);	

}

void error(char *msg)
{
	perror(msg);
	exit(0);
}




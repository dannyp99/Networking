#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>
#define BSIZE 512

void decrypt(char *mesg, char *cypher, int *perm, char *xKeyS){
	for(int i=0;i<BSIZE;i++)
		mesg[i] = cypher[perm[i]] ^ xKeyS[i];
}

void readfull(int fd, void *buf, int len)
      {
	int br; // bytes read each time;
	int total = 0;
	while (total<len)
	    {
		br = read(fd,buf+total,(len-total));
		total += br;
	    }
      }//readfull

/* argv[1] will be port that server "binds" to */
int main(int argc, char **argv)
{
  int sfd;  // server (listening) socket
  int cfd;  // communications socket with client
  int dfd;
  int rfd;
  printf("check1\n");
  char filename[64];
  char filename2[64];
  struct sockaddr_in saddr;  // info concerning server
  struct sockaddr_in caddr;  // info concerning connecting clients
  unsigned char mesg[512];
  unsigned char keyS[512]; // arbitrary data buffer
  unsigned char keyS_encode[512];
  unsigned char stamp[512];
  unsigned char stamp_encode[512];
  unsigned char text[512];
  unsigned char keyB[512];
  int permB[512];
  unsigned char wbuffer[BSIZE+1];
  printf("check2\n");
  sprintf(filename,"%s.xkey", argv[2]);
  dfd = open(filename,O_RDONLY);
  if(dfd<0){printf("key File not found\n");}
  readfull(dfd,keyB,BSIZE);
  close(dfd);
  sprintf(filename2,"%s.perm",argv[2]);
  rfd = open(filename2,O_RDONLY);
  if(rfd<0){printf("perm File not found\n");}
  readfull(rfd,permB,BSIZE*4);
  close(rfd);
  printf("check3\n");
  for(int i=0; i<BSIZE;i++){
  	permB[i]= ntohl(permB[i]);
  }
  int x, y, retval;             // some values to play with
  socklen_t xlen;
  saddr.sin_family = AF_INET; 
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); // wildcard
  saddr.sin_port = htons(atoi(argv[1]));  // local port to bind to
  sfd = socket(AF_INET,SOCK_STREAM,0); // create server socket;
  bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));
  listen(sfd,16);// set socket queue length

  while(1)  // main server loop
    {

     // wait for client connection:
     xlen = sizeof(caddr);  // in-out parameter to accept:
     cfd = accept(sfd,(struct sockaddr*)&caddr,&xlen);
     if (cfd < 0) { perror("socket error\n"); exit(1); }
 
     // at this point, tcp connection is in ESTABLISHED state, and
     // communication with peer can begin (yeah!)
     printf("reading from client\n");
     readfull(cfd,keyS_encode,512);//read the key  
     readfull(cfd,stamp_encode,512);// read the permutation
     readfull(cfd,text,512);
     printf("Done reading from client\n");
     decrypt(keyS, keyS_encode, permB, keyB);
     decrypt(stamp, stamp_encode, permB, keyB);
     for(int i =0; i<BSIZE;i++){
     	mesg[i] = keyS[i]^stamp[i];
     }
     char response[BSIZE+1];
     char replyA[BSIZE];
     int mesg_check = strncmp(text, mesg, BSIZE);
     printf("Check: %d\n", mesg_check);
     if(mesg_check ==0){
	response[0]=0;
	for(int i =1; i<BSIZE+1;i++)
		response[i]=stamp[i-1];
     	write(cfd,response,BSIZE+1);
     }else{
	response[0]=1;
	sprintf(response+1, "%s", "Nope!");
	write(cfd, response, BSIZE+1);
     }
     readfull(cfd,replyA,BSIZE);
     printf("%s\n",replyA);
     x = 64; // number of bytes to send;
     y = htonl(x); // convert to network byte ordering
    // write(cfd,&y,sizeof(int)); // send number of bytes to expect
     //write(cfd,wbuffer,atoi(mesg)); // write data
     //retval = read(cfd,wbuffer,8); // read up to 8 bytes
     //if (retval != 8) {printf("didn't get 8 bytes!\n"); exit(1);}

     // might want to know who just connected to you:
     printf("client from %s has been served\n",inet_ntoa(saddr.sin_addr));
     
     close(cfd); // note, only communication socket closed here
    } // server loop  
} // main

// this program must be killed externally (cntl-c or kill -9)


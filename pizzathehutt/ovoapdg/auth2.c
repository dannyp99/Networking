#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>

#define TASPORT 21004
#define KEYSIZE 512
#define NAMESIZE 64

// read fully...
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

void writefull(int fd, void *buf, int len)
      {
	int br; // bytes read each time;
	int total = 0;
	while (total<len)
	    {
		br = write(fd,buf+total,(len-total));
printf("just wrote %d bytes\n",br);  fflush(stdout);
		total += br;
	    }
      }//readfull

// returns -1 on error, 0 on timeout, or + number if readable
int readable_timeo(int fd, int sec) 
{
  fd_set rset;
  struct timeval tv; 
  FD_ZERO(&rset);
  FD_SET(fd,&rset);
  tv.tv_sec = sec;
  tv.tv_usec = 0;
  return (select(fd+1,&rset,NULL,NULL,&tv));
} // readable_timeo

void genkeyperm(int P[KEYSIZE], unsigned char K[KEYSIZE]) // writes to P, K
{
  // create id perm
  int i = 0;
  int r, tmp;
  srandom((int)(long long)&r);  // use address to seed random number generator
  for(;i<KEYSIZE;i++) P[i]=i;
  if (K) for(i=0;i<KEYSIZE;i++) K[i]= random() % 256;
  // swap around
  for(i=0;i<KEYSIZE-1;i++)
    {
      r = i + (random()%(KEYSIZE-i));
      tmp = P[i];  P[i]=P[r];  P[r]=tmp;
    }
}//genkeyperm

typedef void * (*threadfun)(void *);

void* serve(void* cfdp)  // thread code
{
  int cfd = *(int*)cfdp;
  char nameA[NAMESIZE];
  char nameB[NAMESIZE];

  int Afd, Bfd;
  char filename1[96];
  unsigned char keyS[KEYSIZE]; // session key
  unsigned char stamp[KEYSIZE]; // session stamp
  unsigned char keyA[KEYSIZE];
  unsigned char keyB[KEYSIZE];
  unsigned char sbuf[KEYSIZE]; // work buffer
  int pA[KEYSIZE*4]; // permutations
  int pB[KEYSIZE*4];

  // read names from cfd
  readfull(cfd,nameA,NAMESIZE);
  readfull(cfd,nameB,NAMESIZE);
  // assume nameA, nameB are zero-terminated strings: force to be so:
  nameA[NAMESIZE-1] = nameB[NAMESIZE-1] = 0;
  printf("   client %s wants to connect to server %s\n",nameA,nameB);
  fflush(stdout);
  
  // open file and read A,B keys and perms
  strcpy(filename1,nameA);
  strcat(filename1,".xkey");
  Afd = open(filename1,O_RDONLY,0);
  readfull(Afd,keyA,KEYSIZE);
  close(Afd);
  strcpy(filename1,nameA);  strcat(filename1,".perm");
  Afd = open(filename1,O_RDONLY,0);
  readfull(Afd,pA,KEYSIZE*4);
  close(Afd);
  strcpy(filename1,nameB);   strcat(filename1,".xkey");
  Bfd = open(filename1,O_RDONLY,0);
  readfull(Bfd,keyB,KEYSIZE);
  close(Bfd);
  strcpy(filename1,nameB);  strcat(filename1,".perm");
  Bfd = open(filename1,O_RDONLY,0);
  readfull(Bfd,pB,KEYSIZE*4);
  close(Bfd);
  // generate session key, stamp
  srandom(cfd); // use socket id to seed random number generator
  int i = 0;
  for(;i<KEYSIZE;i++)
    {
       stamp[i]= random() % 256;
       keyS[i]= random() % 256;       
    }
printf("here4\n"); fflush(stdout);    
  // send permA(keyA(keyS))
  for(i=0;i<KEYSIZE;i++)
    sbuf[pA[i]] = keyS[i] ^ keyA[i];
  writefull(cfd,sbuf,KEYSIZE);
printf("here5\n"); fflush(stdout);    
  // send permA(keyA(stamp))
  for(i=0;i<KEYSIZE;i++)
    sbuf[pA[i]] = stamp[i]^keyA[i];
  writefull(cfd,sbuf,KEYSIZE);
printf("here6\n"); fflush(stdout);      
  // send permB(keyB(keyS))
  for(i=0;i<KEYSIZE;i++)
    sbuf[pB[i]] = keyS[i]^keyB[i];
  writefull(cfd,sbuf,KEYSIZE);
printf("here7\n"); fflush(stdout);        
  // send permB(keyB(stamp))
  for(i=0;i<KEYSIZE;i++)
    sbuf[pB[i]] = stamp[i]^keyB[i];
  writefull(cfd,sbuf,KEYSIZE);
printf("here8\n"); fflush(stdout);
 
  close(cfd);
  printf("  sent encrypted session keys to %s\n",nameA);  fflush(stdout);
  return 0;
}//serve thread


// argv[1] is external ip of NAT box this server is running on.
int main(int argc, char **argv)
{
  int sfd;  // server (listening) socket
  int cfd;  // communications socket with client
  int cfdc;  // copy of cfd
  struct sockaddr_in saddr;  // info concerning server
  struct sockaddr_in caddr;  // info concerning connecting clients
  int x, y, retval;             // some values to play with
  socklen_t xlen;

  saddr.sin_family = AF_INET; 
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); // wildcard
  saddr.sin_port = htons(TASPORT);  // local port to bind to

  sfd = socket(AF_INET,SOCK_STREAM,0); // create server socket;
  bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));  // reserve server port
  listen(sfd,16); // set socket queue length
  x = 1;
  setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&x,sizeof(int));  

  printf("OVOAP Trusted Authentication Server, Version 2C Started ...\n");
  fflush(stdout);  // forces immediate print

  // main server loop creates pthreads
  pthread_t tid; // thread id
  while (1)
    {
      // wait for client connection:
      xlen = sizeof(caddr);  // in-out parameter to accept:
      cfd = accept(sfd,(struct sockaddr*)&caddr,&xlen);
      if (cfd < 0) { printf("socket error, cfd %d\n",cfd); fflush(stdout); }
      else
	{
printf("in main, socekt id is %d\n",cfd);
          cfdc = cfd;
	  retval = pthread_create(&tid, NULL, serve, (void*)&cfdc);
	}
    } // server loop

  return 0;  
}//main

// compile with gcc -pthread  (not -lpthread)

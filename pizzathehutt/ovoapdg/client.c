/* tcp client skeleton */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>


/*
   argv[0] is always the name of the program itself (implicit)
   argv[1] will be ip address of server
   argv[2] will be port number of server
*/

/*
   The read command of c takes an argument indicating the MAXIMUM
   number of bytes to read, and returns the actual number of bytes read,
   so to keep reading until we've read exactly a cetain number of bytes,
   we should use the following:
*/
#define AUTHSERVER_PORT 21004
#define CLIENT_PORT 31003
#define KEYSIZE 512
void readfully(int server_fd, void* addr, int size) 
{
  int r = 0; // number of bytes read so far
  while (r<size)
    {
      r += read(server_fd, (unsigned char*)(addr + r), size-r);
    }
}

void encrypt(char* ciphertext, char* key, int* perm, char* plaintext)
{
	for(int i = 0; i<KEYSIZE; i++)
	{
		ciphertext[perm[i]] = plaintext[i] ^ key[i];
	}
}
void decrypt(char* plaintext, char* key, int* perm, char* ciphertext)
{
	for(int i = 0; i<KEYSIZE; i++)
	{
		plaintext[i] = ciphertext[perm[i]] ^ key[i];
	}
}
int main(int argc, char **argv)
{
  int server_fd; // communication file descriptor (handle on socket)
  // declare structure to hold socket address info:
  struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;
  int retval; // many functions return values to indicate success/failure
  char* clientname = argv[3]; 
  char* servername = argv[4]; 
  serveraddr.sin_family = AF_INET;  // always for IPv4
  serveraddr.sin_addr.s_addr = inet_addr(argv[1]);	
  serveraddr.sin_port = htons(AUTHSERVER_PORT);
  clientaddr.sin_family = AF_INET;
  clientaddr.sin_addr.s_addr = inet_addr(argv[2]);
  clientaddr.sin_port = htons(CLIENT_PORT);
    server_fd = socket(AF_INET,SOCK_STREAM,0); // register tcp socket with OS

  // make connection to server.
  retval = connect(server_fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
  // note the type cast to more generic type (polymorphism)
  if (retval != 0) { perror("server socket failure!"); exit(1); }
  char bufAname[64];
  char bufBname[64];
  strncpy(bufAname,clientname,64);
  strncpy(bufBname,servername, 64); 
  write(server_fd, bufAname, 64);
  write(server_fd, bufBname, 64);
  char keyS_encA[KEYSIZE];
  char stamp_encA[KEYSIZE];
  char keyS_encB[KEYSIZE];
  char stamp_encB[KEYSIZE];
  readfully(server_fd, keyS_encA, KEYSIZE);
  readfully(server_fd, stamp_encA, KEYSIZE);
  readfully(server_fd, keyS_encB, KEYSIZE);
  readfully(server_fd, stamp_encB, KEYSIZE);
  close(server_fd); // close socket exit(0);
  char keyS[KEYSIZE];
  char stamp[KEYSIZE];
  char authtoken[KEYSIZE]; //key xor stamp
  // note the type cast to more generic type (polymorphism)
  char keypath[64];
  char permpath[64];
  sprintf(keypath, "%s.xkey",clientname);
  sprintf(permpath, "%s.perm", clientname);
  int keyfile;
  int permfile;
  keyfile = open(keypath, O_RDONLY);
  permfile = open(permpath, O_RDONLY);
  if (keyfile < 0)
  {
	  printf("Error opening keyfile!\n");
	  exit(1);
  }
  else if (permfile < 0)
  {
  	printf("Error opening permfile!\n");
	exit(1);
  }
  char key[KEYSIZE];
  int perm[KEYSIZE];
  readfully(keyfile, key, KEYSIZE);
  readfully(permfile, perm, 4 * KEYSIZE);
  for (int i =0 ; i < KEYSIZE; i++)
  {
	perm[i] = ntohl(perm[i]);
  } 
  close(keyfile);
  close(permfile);
  

  int client_fd;
  client_fd = socket(AF_INET,SOCK_STREAM,0);
  retval = connect(client_fd,(struct sockaddr*)&clientaddr,sizeof(clientaddr));
  if (retval != 0) { perror("client socket failure!"); exit(1); }

  decrypt(keyS, key, perm, keyS_encA);
  decrypt(stamp, key, perm, stamp_encA);
  for(int i = 0; i < KEYSIZE; i++) 
  {
 	authtoken[i] = keyS[i] ^ stamp[i];
  }
  write(client_fd, keyS_encB, 512);
  write(client_fd, stamp_encB, 512);
  write(client_fd, authtoken, 512);
  char response[KEYSIZE + 1];
  printf("Reading from server\n");
  readfully(client_fd, response, KEYSIZE+1);
  printf("Done reading from server\n");
  char reply[KEYSIZE];
  if(response[0] == 0)
  {
	int stamp_check = strncmp(response+1, stamp, KEYSIZE);

	if (stamp_check == 0) //OK
	{
	 	reply[0] = 1;
		sprintf(reply+1, "%s", "You're good. May the Schwartz be with you...");
		printf("Client is ok\n");
	}
	else 
	{ //haxx0rz
		reply[0] = 1;
		printf("haxx0rz detected!\n");
		sprintf(reply+1,"%s", "12345? That's the sort of stamp an idiot would have on his luggage! (Denied!)");
	}
  }
  else  //remainder is plaintext
  {
	printf("Recieved from B: %s\n", response+1);
	printf("Client thinks we're fake!\n");
	char reply[KEYSIZE];
	reply[0] =1;
	sprintf(reply+1, "%s", "RIP in pepperonis");

  }

  write(client_fd,reply, KEYSIZE);
  close(client_fd);
  return 0;
}


/*
   Compile on Linux with "gcc clientskl.c"

   Solaris and other Unix systems (possibly cygwin also) may require
   "gcc -lnsl -lsocket clientskl.c"
*/

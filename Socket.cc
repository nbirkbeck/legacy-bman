#include "Socket.h"

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

static int count=0;

void defaultSocketError(const char * string)
{
	fprintf(stderr,"%s\n",string);
}



Socket::Socket()
{
	if(count==0){
		
	#if defined(WIN32)
		WORD    wsavers;
		WSADATA wsadata;

		wsavers = MAKEWORD(1, 1);
		if (WSAStartup(wsavers, &wsadata) != 0) {
			(void) fprintf(stderr, "WSAStartup failed.\n");
			exit(1);
		}
	#endif 
		printf("first socket\n");
	}

	errorFunc=defaultSocketError;
	if((sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0){
		errorFunc("Error creating socket");
	}

	
	count++;
}

int Socket::listen(int backlog)
{
	if(::listen(sock,backlog)<0){
		errorFunc("Error listening");
		return 0;
	}
	return 1;
}

int Socket::bind(unsigned short port)
{
	struct sockaddr_in sin;
	
	memset(&sin,0,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=htonl(INADDR_ANY);
	sin.sin_port=htons(port);

	if(::bind(sock,(sockaddr *)&sin,sizeof(sin))<0)
		return 0;

	return 1;
}

Socket * Socket::accept()
{
	Socket * client=NULL;
	sockaddr_in csin;
	int csock=0;
	socklen_t clen=sizeof(sockaddr_in);

	memset(&csin,0,sizeof(csin));

	if((csock=::accept(sock,(struct sockaddr *)&csin,&clen)) <0 ){
		errorFunc("Error accepting");
		return NULL;
	}
	client = new Socket();//client.sock already set, FIXME
	client->sock=csock;
	return client;
}

int Socket::connect(char * str, unsigned short port)
{
	struct sockaddr_in sin;
	memset(&sin,0,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=inet_addr(str);
	sin.sin_port=htons(port);

	if(::connect(sock,(const struct sockaddr *)&sin,sizeof(sin))<0){
		return 0;
	}
	return 1;
}

int Socket::send(char * bytes,int len)
{
	int sent;
	if((sent=::send(sock,bytes,len,0))!=len){
		return 0;
	}
	return sent;
}

int Socket::send(char * bytes,int len,int flags)
{
	int sent;
	if((sent=::send(sock,bytes,len,0))!=len){
		return 0;
	}
	return sent;
}
int Socket::recv(char * bytes,int len)
{
	int nrecv=0;
	int crecv=0;
	while(nrecv<len){
		crecv=::recv(sock,bytes+nrecv,len-nrecv,0);
		if(crecv<0)
			return 0;
		nrecv+=crecv;
	}
	return nrecv;
}
int Socket::recvln(char * bytes,int len)
{
	int nrecv=0;
	int crecv=0;

	len--;
	while(nrecv<len){
		crecv=::recv(sock,bytes+nrecv,1,0);
		
		if(crecv<0)
			return 0;
		if(bytes[nrecv]=='\n'){
			nrecv+=crecv;
			break;
		}
		nrecv+=crecv;
	}
	bytes[nrecv]=0;
	return nrecv;
}


Socket::~Socket()
{
	//::close(sock);
	count--;
	if(count==0){
		printf("last socket\n");
	}
}

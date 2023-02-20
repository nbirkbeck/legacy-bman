#ifndef SOCKET_H
#define SOCKET_H

#define SOCKET_CLIENT 0
#define SOCKET_SERVER 1

class Socket
{
public:
	Socket();
	Socket(int type);

	int bind(unsigned short port);
	int listen(int backlog);

	Socket * accept();

	int recv(char * bytes,int len);
	int recvln(char * bytes,int len);


	int send(char * bytes,int len);
	int send(char * bytes,int len,int flags);

	int connect(char * str, unsigned short port);

	
	~Socket();
	int sock;
	void (* errorFunc)(const char *);
};


#endif


#include "network.h"
#include <string.h>
#include <stdio.h>
#include "globals.h"
#include "level.h"
#include "player.h"
#include "font.h"
#include "list.h"
#include "menu.h"
#include "Socket.h"


extern unsigned int grid[GRID_HEIGHT][GRID_WIDTH];
extern char curTextures[255];

extern List playerList;
extern Player players[4];
extern Font font;
extern int curClientNum;
#define PACKET_SIZE 64
#define UDP_PORT 667

DWORD workerThread(LPVOID param);


int warn=1;
/*
	Server related variables
*/
HANDLE serverHandle=NULL;
DWORD dwServerThreadID;

int netNumPlayers=0;
int netMaxPlayers=0;
char serverName[255];
int serverSock=NULL;

Socket * clientSockets[4];
Socket *  remoteSock=NULL;

HANDLE clientHandle=NULL;
DWORD dwClientThreadID;



int connectFunc(char * line)
{
	char command[25];
	char ip[30];
	int port;
	resetGameState();
	if(2==sscanf(line,"%s %s",command,ip))
		connectToServer(ip,667);
	else if(3==sscanf(line,"%s %s %d",command, ip ,&port))
		connectToServer(ip,port);
	else return 0;

	gotoGameMain();
	
	return 1;
}

int connectToServer(char * ip,int port)
{
	char buffer[1024];
	int cnum;
	int rsofar=0;
	int rthattime=0;
	int len;

	if(remoteSock!=NULL)
	{
		stopClientThread();
		delete(remoteSock);
	}
	remoteSock=new Socket();
	int option=2;
	setsockopt(remoteSock->sock,SOL_SOCKET, SO_SNDLOWAT,(char *)&option,sizeof(option));
	option = 1;
	setsockopt(remoteSock->sock,IPPROTO_TCP,TCP_NODELAY,(char *)&option,sizeof(option));
	if(!remoteSock->connect(ip,port)){
		delete(remoteSock);
		remoteSock=NULL;
		return -1;
	}

	buffer[0]=NET_CONNECT;
	if(remoteSock->send(buffer,1,0)!=1)
	{
		delete(remoteSock);
		remoteSock=NULL;
		return -1;
	}

	if(remoteSock->recv(buffer,1)!=1)
	{
		delete(remoteSock);
		remoteSock=NULL;
		return -1;
	}
	if(!buffer[0])
		return 0;
	if(remoteSock->recv(buffer,3)!=3)
	{
		delete(remoteSock);
		remoteSock=NULL;
		return -1;
	}
	cnum=buffer[2];
	if(remoteSock->recv(&buffer[0],1)!=1)
	{
		delete(remoteSock);
		remoteSock=NULL;
		return -1;
	}
	len = buffer[0];
	remoteSock->recv(buffer,len);

	
	if(remoteSock->recv(&buffer[0],1)!=1)
	{
		delete(remoteSock);
		remoteSock=NULL;
		return -1;
	}

	len = buffer[0];
	remoteSock->recv(buffer,len);

	memset(grid,0,GRID_HEIGHT*GRID_WIDTH*sizeof(int));
	
	rsofar=0;
	len = sizeof(int)*GRID_HEIGHT*GRID_WIDTH;
	remoteSock->recv((char *)grid,len);

	addSinglePlayer(cnum,1);
	clientHandle=CreateThread(NULL,0,clientThread,NULL,0,&dwClientThreadID);
	curClientNum=cnum;

	return cnum;
}

void sendByte(int toSend)
{
	char buffer[2];

	/*
		Dont send if not connected
	*/
	if(remoteSock==NULL)
		return;
	//sendPlayerData(remoteSock,curClientNum);
	buffer[0]=NET_MOVE;
	buffer[1]=(char)toSend;

	if(remoteSock->send(buffer,2,0)!=2)
	{
		MessageBox(NULL,"Sent different length than it should have","Client",MB_OK);
		delete(remoteSock);
		remoteSock=NULL;
	}

}

DWORD clientThread(LPVOID param)
{
	char buffer[1024];
	if(remoteSock==NULL)
		return 0;

	while(remoteSock)
	{	
		int dir=-1;
		int cnum;
		char c;
		if(remoteSock->recv(buffer,1)<=0)
		{
			stopClientThread();
			return 0;
		}
		switch(buffer[0])
		{
		case NET_MOVE:
			
			
			if(remoteSock->recv(buffer,1)<=0)
			{
				stopClientThread();
				return 0;
			}
			c=buffer[0];
			cnum=(buffer[0]>>5);
			
			if(c&0x4)
			{
				dir=(c&0x3);
				preMovePlayer(&players[cnum]);
			}
			switch(dir)
			{
			case UP:
				movePlayerUp(&players[cnum]);break;
			case DOWN:
				movePlayerDown(&players[cnum]);break;
			case LEFT:
				movePlayerLeft(&players[cnum]);break;
			case RIGHT:
				movePlayerRight(&players[cnum]);break;
			}
			if(c&0x8)
			{
				playerPlaceBomb(&players[cnum]);
			}
			if(c&0x10)
			{
				playerAltKey(&players[cnum]);
			}
			break;
		case NET_CHAT:
			/*
				The first byte is the client that sent the
				chat, the second byte is the length of the 
				message
			*/
			if(remoteSock->recv(buffer,2)<=0)
			{
				stopClientThread();
				return 0;
			}
			if(remoteSock->recv(&buffer[2],buffer[1])<=0)
			{
				stopClientThread();
				return 0;
			}
			{
				char * str = (char *) malloc(buffer[1]+1);
				memcpy(str,&buffer[2],buffer[1]);
				str[buffer[1]]='\0';
				addMessage(str,&font,CHAT_X,CHAT_Y,400);
			}
			break;
		case NET_PLAYER:
			recvPlayerData(remoteSock);
			break;
		case NET_PLAYER_DISCONNECT:
			if(remoteSock->recv(buffer,1))
				removePlayer(buffer[0]);
			else return 0;
			break;
		}
		Sleep(4);

	}
	
}

int handleQuery(Socket * socket)
{
	char buffer[255];
	int i=1;
	if(socket==NULL)
		return 0;
	buffer[0]=0;
	buffer[1]=netNumPlayers;
	buffer[2]=netMaxPlayers;
	buffer[3]=strlen(serverName);
	memcpy(&buffer[4],serverName,buffer[3]);
	socket->send(buffer,buffer[3]+4,0);
	return 1;
}

int serverNameFunc(char * line)
{
	int i=0;
	while(i<strlen(line))
	{
		if(line[i]==' ')
		{
			while(line[i]==' ')
				i++;
			break;
		}
		i++;
	}
	if(i<strlen(line))
	{
		sprintf(serverName,&line[i]);
	}
	
	return 1;
}

int startServer(int gameType,int nplayers,char * name)
{
	int port=667;
	netMaxPlayers=nplayers;
	sprintf(serverName,name);

	/*
		Start the thread, if it is running, kill it, and then
		start up a fresh one
	*/
	memset(clientSockets,0,sizeof(int)*4);

	stopServerThread();

	serverHandle=CreateThread(NULL,0,newServerThread,&port,0,&dwServerThreadID);
	Sleep(1000);
	connectToServer("127.0.0.1",667);
	return 1;
}

DWORD WINAPI newServerThread(LPVOID param)
{
	Socket server;
	server.bind(667);
	server.listen(10);

	int option=1;
	setsockopt(server.sock,SOL_SOCKET, SO_REUSEADDR,(char *)&option,sizeof(option));

	struct sockaddr_in clientAddr;
	int cliAddrLen;
	int size;
	char buffer[PACKET_SIZE];
	char string[255];

	int move;
	int clientSock=NULL;
	cliAddrLen = sizeof(clientAddr);

	netNumPlayers=0;
	
	while(1)
	{
		Socket * clientSock;
		if((clientSock = server.accept())==NULL)
			MessageBox(NULL,"Error accepting","Socket",MB_OK);
		

		if((size = clientSock->recv(buffer,1))<0)
		{
			MessageBox(NULL,"Recieve failed","Socket",MB_OK);
		}
		/*
			The first byte can only be one of the following two
			otherwise it is considererd nonsense, and the client
			is kicked
		*/
		if(buffer[0]==NET_CONNECT)
		{
			/* there is room for the client */
			if(netNumPlayers<netMaxPlayers)
			{
				DWORD dwID;
				
				int i=0;
				while(i<netMaxPlayers)
				{
					if(clientSockets[i]==NULL)
						break;
					i++;
				}
				if(i!=netMaxPlayers)
				{
					int * param = (int *)malloc(sizeof(int));
					*param=i;
					clientSockets[i]=clientSock;
					buffer[0]=1;
					buffer[1]=(char)netNumPlayers;
					buffer[2]=(char)netMaxPlayers;
					buffer[3]=(char)i;
					if(clientSockets[i]->send(buffer,4,0)<0)
						;

					buffer[0]=strlen(serverName);
					memcpy(&buffer[1],serverName,buffer[0]);

					if(clientSockets[i]->send(buffer,buffer[0]+1,0)<0)
						;

					buffer[0]=strlen(curTextures);
					memcpy(&buffer[1],curTextures,buffer[0]);

					if(clientSockets[i]->send(buffer,buffer[0]+1,0)<0)
						;
					int tosend=sizeof(int)*GRID_HEIGHT*GRID_WIDTH;
					
					if((clientSockets[i]->send(((char *)grid),tosend,0))<=0)
								;
				
					/*
						Now get the player data and send it over to the client
					*/
		
					Node * finger=(Node *) playerList.head;
				
					while(finger!=NULL)
					{	
						Player * p = (Player *)finger->value;
						sendPlayerData(clientSockets[i],p->clientNum);
						finger = finger->next;
					}
					i=0;
					while(i<netMaxPlayers)
					{
						if(i!=*param && clientSockets[i])
						{
							sendPlayerData(clientSockets[i],*param);
						}
						i++;
					}
					
					CreateThread(NULL,0,workerThread,(void *)param,0,&dwID);
				}
				netNumPlayers++;
			}
			/* no room for the client */
			else
			{
				buffer[0]=(char)0;
				clientSock->send(buffer,1,0);
				delete(clientSock);
				clientSock=NULL;
			}

		}
		else if(buffer[0]==NET_QUERY)
		{
			sockaddr_in address;
			char * warning=NULL;
			int len;
			handleQuery(clientSock);
			len = sizeof(address);
			getpeername(clientSock->sock,(sockaddr *)&address,&len);
			if(warn)
			{
				warning = (char *)malloc(64);
				sprintf(warning,"Query by %s",inet_ntoa(address.sin_addr));
				addMessage(warning,&font,100,10,99);
			}

		}
		else
		{
			delete(clientSock);
			clientSock=NULL;
		}
		/*
			//removeMessage(tstring);
			
		}
		*/
	}
	return 1;
}

/*
	pre: none
	post: chat is sent from the client to the server, iff the client
	is connected to the server
*/
int sendChatClient(char * s)
{
	char c[3];
	if(remoteSock==NULL)
		return 0;
	c[0]=NET_CHAT;
	c[1]=0;
	c[2]=strlen(s);
	if(remoteSock->send(c,3,0)<=0)
	{
		stopClientThread();
		return 0;
	}
	
	if(remoteSock->send(s,strlen(s),0)<=0)
	{
		stopClientThread();
		return 0;
	}
	return 1;
}

/*
	Send the player data on socket.
	player is the index in players[]
*/
int sendPlayerData(Socket * socket,int player)
{
	char c[2];
	c[0]=NET_PLAYER;
	c[1]=player;
	socket->send(c,2,0);
	socket->send((char *)&players[player],PLAYER_NET_SIZE,0);
	return 1;
}


/*
	Assuming NET_PLAYER has already been received, receive
	the rest of the player
*/
int recvPlayerData(Socket * socket)
{
	char c;
	socket->recv(&c,1);
	socket->recv((char *)&players[c],PLAYER_NET_SIZE);
	
	Node * finger = playerList.head;
	while(finger!=NULL)
	{
		Player * p = (Player *) finger->value;
		if(p==&players[c])
		{
			break;
		}
		finger =finger->next;
	}
	/*
		The player we are being sent information on is not in the list, so we must
		add him now
	*/
	if(finger==NULL)
	{
		addSinglePlayer(c,6);
	}
	return c;
}

DWORD workerThread(LPVOID param)
{
	int cnum=*((int *)param);
	Socket * sock;
	char str[255];
	char buffer[1024];
	free(param);

	sock=clientSockets[cnum];

	while(1)
	{
		char c=0;
		int err=0;

		if(sock->recv(&c,1)!=1)
		{
			goto shutdown_;
		}
		switch(c)
		{
		case NET_MOVE:
			if(sock->recv(&c,1)<=0)
				goto shutdown_;
			else
			{
				/*
				int dir=-1;
				if(c&0x4)
				{
					dir=(c&0x3);
					preMovePlayer(&players[cnum]);
				}
				switch(dir)
				{
				case UP:
					movePlayerUp(&players[cnum]);break;
				case DOWN:
					movePlayerDown(&players[cnum]);break;
				case LEFT:
					movePlayerLeft(&players[cnum]);break;
				case RIGHT:
					movePlayerRight(&players[cnum]);break;
				}
				if(c&0x8)
				{
					playerPlaceBomb(&players[cnum]);
				}
				if(c&0x1)
				{
					playerAltKey(&players[cnum]);
				}*/
				buffer[0]=NET_MOVE;
				buffer[1]=c| (char)(cnum<<5);
				int i=0;
				while(i<netMaxPlayers)
				{
					if(i!=cnum)
					{
						clientSockets[i]->send(buffer,2,0);
					}
					i++;
				}
			}
			break;
		case NET_CHAT:
			/*
				This is the client Number, just sent for consistency
			*/
			buffer[0]=NET_CHAT;
			if(sock->recv(&buffer[1],1)<=0)
				goto shutdown_;
			if(sock->recv(&buffer[2],1)<=0)
				goto shutdown_;
			else
			{
				
				
				if(sock->recv(&buffer[3],buffer[2])<=0)
					goto shutdown_;
				buffer[1]=cnum;
				int i=0;
				while(i<netMaxPlayers)
				{
					if(i!=cnum)
						clientSockets[i]->send(buffer,buffer[2]+3,0);
					i++;
				}
			}
			break;
		}
		Sleep(5);
		
	}

	/*
		Shutdown the socket, and set it to null so it can
		be reused
	*/
shutdown_:
		
	delete(clientSockets[cnum]);
	clientSockets[cnum]=NULL;

	int i=0;
	buffer[0]=NET_PLAYER_DISCONNECT;
	buffer[1]=cnum;
	while(i<netMaxPlayers)
	{
		if(clientSockets[i])
		{
			clientSockets[i]->send(buffer,2,0);
		}
		i++;
	}

	netNumPlayers--;

	return 1;
}

Socket * serverStartup(unsigned short port)
{
	Socket * ss = new Socket();
	ss->bind(port);

	return ss;
}

void networkStartup()
{
	clientSockets[0]=0;
	clientSockets[1]=0;
	clientSockets[2]=0;
	clientSockets[3]=0;

}

int stopServerThread()
{
	if(serverSock)
	{
		close(serverSock);
		serverSock=NULL;
	}
	if(serverHandle)
	{
		CloseHandle(serverHandle);
		serverHandle=NULL;
	}

	return 1;
}

int stopClientThread()
{
	if(remoteSock)
	{
		delete(remoteSock);
		remoteSock=0;
	}
	if(clientHandle)
		CloseHandle(clientHandle);
	clientHandle=NULL;
	
	return 1;
}

int disconnectFunc(char *)
{
	stopClientThread();
	gotoMenu();
	return 1;
}

int networkShutdown()
{
	stopClientThread();
	stopServerThread();
	WSACleanup();
	return 1;
}



DWORD WINAPI serverThread(LPVOID param)
{

	struct sockaddr_in serverAddr;
	int sock;
	int port=667;
	if((sock = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP))<0)
		MessageBox(NULL,"Failed to create server socket","Socket",MB_OK);

	memset(&serverAddr,0,sizeof(sockaddr_in));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(port);
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);

	if(bind(sock,(struct sockaddr *)&serverAddr,sizeof(sockaddr_in))<0)
		MessageBox(NULL,"Binding server socket failed","Socket",MB_OK);

	struct sockaddr_in clientAddr;
	int cliAddrLen;
	int size;
	char buffer[PACKET_SIZE];
	char string[255];
	//int sock= *(int *)param;
	printf("socket is %d\n",sock);
	while(1)
	{
		short x,y;
		short frame;
		short clientNum;
		short dir;
		int i=0;
		char * ptr;
		int nBombsInUse;
		cliAddrLen = sizeof(clientAddr);
		
		if((size = recvfrom(sock,buffer,PACKET_SIZE,0,(struct sockaddr *) &clientAddr,&cliAddrLen))<0)
			MessageBox(NULL,"Failed to receive","Socket",MB_OK);
		
		clientNum=buffer[0];
		memcpy(&x,buffer+1,2);
		memcpy(&y,buffer+3,2);
		x=ntohs(x);
		y=ntohs(y);
		frame = (unsigned char) *(buffer+5);
		dir = *(buffer+6);
		nBombsInUse=*(buffer+7);
		//sprintf(string,"x:%3d,y:%3d frame:%3d dir:%d nb:%d ",x,y,frame,dir,nBombsInUse);
		//printf(string);
		ptr = (buffer+8);
		players[0].x=x;
		players[0].y=y;
		players[0].frame=frame;
		players[0].dir=dir;
		players[0].nBombsInUse=nBombsInUse;
		Node * finger = (Node *) players[0].bombsInUse.head;
	/*
		while(i<nBombsInUse)
		{ 
			unsigned char timer = (unsigned char) *(ptr+1);
			int bx,by;
			Bomb * bomb=NULL;
			
			memcpy(&bx,ptr+2,2);
			memcpy(&by,ptr+4,2);
			bx=ntohs(bx);
			by=ntohs(by);

			if(finger==NULL)
			{
				bomb = placeBomb(0,bx,by);
				insert(&players[0].bombsInUse,&bomb);
			}
			else
			{
				bomb = (Bomb *) finger->value;
				finger=finger->next;
			}
			if(bomb)
			{
				bomb->x=bx;
				bomb->y=by;
				bomb->state=*ptr;
				bomb->timer=timer;
			}
			ptr+=6;
			i++;
		}*/
		//printf("\r");
	
		Sleep(10);
		//MessageBox(NULL,string,"Server",MB_OK);

		//size = sendto(sock,buffer,PACKET_SIZE,0,(struct sockaddr *) &clientAddr,sizeof(clientAddr));
		//Sleep(1500);
	}
	return 1;
}

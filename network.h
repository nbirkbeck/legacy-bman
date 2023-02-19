
#ifndef BMAN_NETWORK
#define BMAN_NETWORK

#include "compat.h"
#include "Socket.h"

#define NET_MOVE   '1'
#define NET_CHAT   '2'
#define NET_QUERY  '3'
#define NET_UPDATE 4
#define NET_CONNECT '5'

#define NET_PLAYER  '6'
#define NET_PLAYER_DISCONNECT 7
#define UD_MAP     8
#define UD_GAMETYPE 9
#define UD_NAME     10



int serverNameFunc(char *);

int startServer(int gameType,int nplayers,char * name);
void networkStartup();

int networkShutdown();
int stopServerThread();
int stopClientThread();

int disconnectFunc(char *);
int connectFunc(char * );

int connectToServer(char * ip,int port);

int recvPlayerData(Socket * socket);
int sendPlayerData(Socket * socket,int player);
int sendChatClient(char * s);

void sendByte(int toSend);
DWORD clientThread(LPVOID param);
DWORD serverThread(LPVOID param);
DWORD newServerThread(LPVOID param);
Socket * serverStartup(unsigned short port);
#endif

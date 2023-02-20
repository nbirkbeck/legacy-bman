
#ifndef BMAN_NETWORK
#define BMAN_NETWORK

#include "Socket.h"
#include "compat.h"

#define NET_MOVE '1'
#define NET_CHAT '2'
#define NET_QUERY '3'
#define NET_UPDATE 4
#define NET_CONNECT '5'

#define NET_PLAYER '6'
#define NET_PLAYER_DISCONNECT 7
#define UD_MAP 8
#define UD_GAMETYPE 9
#define UD_NAME 10

#define UDP_PORT 8668

int serverNameFunc(char*);

int startServer(int gameType, int nplayers, const char* name);
void networkStartup();

int networkShutdown();
int stopServerThread();
int stopClientThread();

int disconnectFunc(char*);
int connectFunc(char*);

int connectToServer(const char* ip, int port);

int recvPlayerData(Socket* socket);
int sendPlayerData(Socket* socket, int player);
int sendChatClient(const char* s);

void sendByte(int toSend);
void* clientThread(void* param);
void* serverThread(void* param);
void* newServerThread(void* param);
Socket* serverStartup(unsigned short port);
#endif

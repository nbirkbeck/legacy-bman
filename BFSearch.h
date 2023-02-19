
#include <stdlib.h>

#ifndef NBFSearch
#define NBFSearch

#include "level.h"
#include "globals.h"
#include "list.h"
#include "queue.h"
#include "player.h"

typedef struct BFSNodeType
{
	int depth;
	int i;
	int j;
	BFSNodeType * parent;
}BFSNode;

Queue * doBFS(int i,int j, int goal, int maxDepth);
int BFS(BFSNode * current);
inline int BFSCanGo(int i, int j,int depthSoFar);
int validatePath(Queue * path);
inline int validatePathNode(int i, int j,int depthSoFar);
#endif
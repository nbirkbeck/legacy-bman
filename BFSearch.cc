#include "BFSearch.h"

int visited[GRID_HEIGHT][GRID_WIDTH];
extern int utilities[GRID_HEIGHT][GRID_WIDTH];
extern unsigned int grid[GRID_HEIGHT][GRID_WIDTH];
extern List playerList;

int (*goalFunc)(int, int);
int (*canGoFunc)(int, int, int);

int g_playerNum = -1;
List stack;
int g_maxDepth;

int goalPlaceBomb(int i, int j) {
  if (utilities[i][j] >= 2 && canHide(i, j, 0) != -1) {
    return 1;
  } else
    return 0;
}

int goalSafe(int i, int j) {
  if (utilities[i][j] >= 1) {
    return 1;
  } else
    return 0;
}

int goalPowerUp(int i, int j) {
  if (utilities[i][j] >= 100) {
    return 1;
  } else
    return 0;
}

int goalFight(int i, int j) {
  Node* finger = playerList.head;
  while (finger != NULL) {
    Player* opp = (Player*)finger->value;
    if (opp->clientNum != g_playerNum) {

      if (((opp->x - XOFFSET + 16) >> 5) == j) {
        if (((opp->y - YOFFSET + 16) >> 5) == i)
          return 1;
      }
    }
    finger = finger->next;
  }
  return 0;
}

/*
        validatePath(Queue * path)
        pre: path is not null!!!
        post: returns 1 if the path is still valid,
        returns 0 otherwise.
*/
int validatePath(Queue* path) {
  int depthSoFar = 0;
  Node* finger = path->head;
  while (finger != NULL) {
    GPoint* p = (GPoint*)finger->value;

    finger = finger->next;
    if (!validatePathNode(p->i, p->j, depthSoFar))
      return 0;
    depthSoFar++;
  }
  return 1;
}

inline int validatePathNode(int i, int j, int depthSoFar) {
  int ut;
  if (i < 0 || j < 0 || i >= GRID_HEIGHT || j >= GRID_WIDTH) {
    return 0;
  }
  if (grid[i][j] & HAS_BOMB_MASK) {
    return 0;
  }
  // ut=-1*(8-(depthSoFar))<<4;
  ut = -(BOMB_DEFAULT_TIMER - 1060 - depthSoFar * 530);
  if (ut > 0)
    ut = -1;
  return (!(grid[i][j] & 0xc)) && (utilities[i][j] >= ut);
}

inline int BFSCanGo(int i, int j, int depthSoFar) {
  int ut;
  if (i < 0 || j < 0 || i >= GRID_HEIGHT || j >= GRID_WIDTH) {
    return 0;
  }
  if (grid[i][j] & HAS_BOMB_MASK) {
    return 0;
  }
  ut = -(BOMB_DEFAULT_TIMER - 1060 - depthSoFar * 530);
  if (ut > 0)
    ut = -1;
  return (!(grid[i][j] & 0xc)) && (utilities[i][j] >= ut) && !visited[i][j];
}

inline int BFSCanGoSafe(int i, int j, int currentUtility) {
  if (i < 0 || j < 0 || i >= GRID_HEIGHT || j >= GRID_WIDTH) {
    return 0;
  }
  if (grid[i][j] & HAS_BOMB_MASK) {
    return 0;
  }
  return (!(grid[i][j] & 0xc)) && !visited[i][j] && (utilities[i][i] > -5000);
}

Queue* doBFS(int i, int j, int goal, int maxDepth) {
  int depthSoFar = 0;
  Queue mustFree;
  BFSNode* node;

  canGoFunc = BFSCanGo;
  switch (goal) {
  case 2:
    goalFunc = goalPlaceBomb;
    break;
  case 1:
    goalFunc = goalSafe;
    canGoFunc = BFSCanGoSafe;
    break;
  case 100:
    goalFunc = goalPowerUp;
    break;
  case 200:
  case 201:
  case 202:
  case 203:
    goalFunc = goalFight;
    g_playerNum = (goal - 200);
    break;
  default:
    goalFunc = goalSafe;
    break;
  }

  /*if(goalFunc==goalSafe && goalFunc(i,j))
  {
          Queue * q = (Queue *) malloc(sizeof(Queue *));
          GPoint * p = (GPoint *) malloc(sizeof(GPoint *));
          p->i=i;
          p->j=j;

          initQueue(q);
          insert(q,p);
          return q;
  }*/

  node = (BFSNode*)malloc(sizeof(BFSNode));

  /*
          Clear the visited array, and initialize the stack
          to prepare for the search
  */
  memset(visited, 0, sizeof(int) * GRID_HEIGHT * GRID_WIDTH);

  initList(&stack);
  initQueue(&mustFree);

  visited[i][j] = 1;
  g_maxDepth = maxDepth;

  node->i = i;
  node->j = j;
  node->parent = NULL;
  node->depth = 0;
  insert(&mustFree, node);

  if (canGoFunc(i - 1, j, depthSoFar)) {
    BFSNode* child = (BFSNode*)malloc(sizeof(BFSNode));

    child->i = i - 1;
    child->j = j;
    child->parent = node;
    child->depth = node->depth + 1;
    insert(&stack, child);
  }
  if (canGoFunc(i + 1, j, depthSoFar)) {
    BFSNode* child = (BFSNode*)malloc(sizeof(BFSNode));

    child->i = i + 1;
    child->j = j;
    child->parent = node;
    child->depth = node->depth + 1;
    insert(&stack, child);
  }
  if (canGoFunc(i, j + 1, depthSoFar)) {
    BFSNode* child = (BFSNode*)malloc(sizeof(BFSNode));

    child->i = i;
    child->j = j + 1;
    child->parent = node;
    child->depth = node->depth + 1;
    insert(&stack, child);
  }
  if (canGoFunc(i, j - 1, depthSoFar)) {
    BFSNode* child = (BFSNode*)malloc(sizeof(BFSNode));

    child->i = i;
    child->j = j - 1;
    child->parent = node;
    child->depth = node->depth + 1;
    insert(&stack, child);
  }

  Node* finger;
  BFSNode* current;
  BFSNode* goalNode = NULL;

  while (stack.head) {
    finger = stack.head;
    current = (BFSNode*)finger->value;

    if (BFS(current)) {
      goalNode = current;
      break;
    }

    insert(&mustFree, finger->value);
    rem(&stack, finger->value);
  }
  while (stack.head) {
    finger = stack.head;
    insert(&mustFree, finger->value);
    rem(&stack, finger->value);
  }
  Queue* path;
  if (goalNode == NULL)
    path = NULL;
  else {
    path = (Queue*)malloc(sizeof(Queue));
    initQueue(path);
    current = goalNode;
    while (current != NULL) {
      GPoint* p = (GPoint*)malloc(sizeof(GPoint));
      p->i = current->i;
      p->j = current->j;
      insert(path, p);

      current = current->parent;
    }
  }

  /*
          All the BFSNodes are freed here

  finger = mustFree.head;
  while(finger!=NULL)
  {
          Node * prev = finger;
          finger=finger->next;
          free(prev->value);
          free(prev);
  }*/
  remAll(&mustFree, 1);
  return path;
}

int BFS(BFSNode* current) {
  int i, j;
  int depthSoFar = current->depth;
  if (current->depth > g_maxDepth) {
    return NULL;
  }

  i = current->i;
  j = current->j;
  visited[i][j] = 1;

  if (goalFunc(i, j)) {
    return 1;
  }

  if (canGoFunc(i - 1, j, depthSoFar)) {
    BFSNode* child = (BFSNode*)malloc(sizeof(BFSNode));

    child->i = i - 1;
    child->j = j;
    child->parent = current;
    child->depth = current->depth + 1;
    insert(&stack, child);
  }
  if (canGoFunc(i + 1, j, depthSoFar)) {
    BFSNode* child = (BFSNode*)malloc(sizeof(BFSNode));

    child->i = i + 1;
    child->j = j;
    child->parent = current;
    child->depth = current->depth + 1;
    insert(&stack, child);
  }
  if (canGoFunc(i, j + 1, depthSoFar)) {
    BFSNode* child = (BFSNode*)malloc(sizeof(BFSNode));

    child->i = i;
    child->j = j + 1;
    child->parent = current;
    child->depth = current->depth + 1;
    insert(&stack, child);
  }
  if (canGoFunc(i, j - 1, depthSoFar)) {
    BFSNode* child = (BFSNode*)malloc(sizeof(BFSNode));

    child->i = i;
    child->j = j - 1;
    child->parent = current;
    child->depth = current->depth + 1;
    insert(&stack, child);
  }
  return NULL;
}

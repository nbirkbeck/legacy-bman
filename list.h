#include <stdio.h>
#include <stdlib.h>
#ifndef NBLIST_H
#define NBLIST_H

typedef struct NODE_TYPE {
  struct NODE_TYPE* next;
  void* value;

} Node;
typedef struct LIST_TYPE {
  Node* head;
  Node* tail;
} List;

void initList(List* list);
void insert(List*, void* value);
void rem(List*, void* value);
void printList(List* list, FILE*);
void remAll(List* list);

#endif
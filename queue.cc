#include "queue.h"

void initQueue(Queue* queue) { queue->head = NULL; }

void printQueue(Queue* queue, FILE* file) {
  Node* finger = queue->head;
  fprintf(file, "<");
  while (finger != NULL) {
    fprintf(file, "%p ", finger->value);
    finger = finger->next;
  }
  fprintf(file, ">\n");
}

void insert(Queue* queue, void* value) {
  Node* newNode = (Node*)malloc(sizeof(Node));
  newNode->next = NULL;
  newNode->value = value;

  if (queue->head == NULL) {
    queue->head = newNode;
  } else {
    newNode->next = queue->head;
    queue->head = newNode;
  }
}

/*
        if freeValues is 1, then free the actual values
        as well.
*/
void remAll(Queue* queue, int freeValues) {
  Node* finger;
  finger = queue->head;
  while (finger != NULL) {
    Node* prev = finger;
    finger = finger->next;
    if (freeValues)
      free(prev->value);
    free(prev);
  }
  queue->head = NULL;
}

void rem(Queue* queue, void* value) {
  Node* finger;
  Node* previous = NULL;

  finger = queue->head;
  while (finger != NULL) {
    if (finger->value == value) {
      if (finger == queue->head) {
        queue->head = (queue->head)->next;
        free(finger);
        finger = NULL;
        break;
      } else {
        previous->next = finger->next;
        free(finger);
        finger = NULL;
        break;
      }
    }
    previous = finger;
    finger = finger->next;
  }
}

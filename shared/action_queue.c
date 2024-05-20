#include <stdio.h>
#include <stdlib.h>

#include "action_queue.h"

void enqueue(ActionNode **head, ActionValue value) {
  ActionNode *temp = (ActionNode *)malloc(sizeof(ActionNode));
  if (!temp) {
    printf("Queue memory allocation error\n");
    return;
  }

  temp->data = value;
  temp->next = NULL;

  if (*head == NULL) {
    *head = temp;
    return;
  }

  ActionNode *rear = *head;
  while (rear->next != NULL) {
    rear = rear->next;
  }
  rear->next = temp;
}

ActionValue dequeue(ActionNode **head) {
  ActionValue value = {0};

  if (*head == NULL) {
    printf("Queue is empty\n");
    return value;
  }

  ActionNode *temp = *head;
  value = temp->data;
  *head = (*head)->next;
  free(temp);
  return value;
}

#ifndef ACTION_QUEUE_H
#define ACTION_QUEUE_H

#include "../shared/types.h"

typedef struct {
  u8 player_id;
  u16 message_id;
  Position position;
} ActionValue;

typedef struct ActionNode {
  ActionValue data;

  struct ActionNode *next;
} ActionNode;

void enqueue(ActionNode **head, ActionValue value);
ActionValue dequeue(ActionNode **head);

#endif

// C program for linked list implementation of stack
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "LIFO_stack.h"
#include "syslog.h"

static const char* TAG = "stack_";

struct StackNode* newNode(judge_type_t type, float weight, uint16_t count) {
  struct StackNode* stackNode = (struct StackNode*)malloc(sizeof(struct StackNode));
  stackNode->count = count;
  stackNode->weight = weight;
  stackNode->type = type;
  stackNode->next = NULL;
  return stackNode;
}

int isEmpty(struct StackNode* root) {
  return !root;
}

void push(struct StackNode** root, judge_type_t type, float weight, uint16_t count) {
  struct StackNode* stackNode = newNode(type, weight, count);
  stackNode->next = *root;
  *root = stackNode;
  LOGI(TAG, "%.3f, %d, %d pushed to stack\n", weight, type, count);
}

int pop(struct StackNode** root) {
  if (isEmpty(*root))
    return INT_MIN;
  struct StackNode* temp = *root;
  *root = (*root)->next;
  int popped = temp->type;
  free(temp);

  return popped;
}

int peek(struct StackNode* root) {
  if (isEmpty(root))
    return INT_MIN;
  return root->type;
}

int create_stack() {
  struct StackNode* root = NULL;

  //   push(&root, 10);
  //   push(&root, 20);
  //   push(&root, 30);

  //   LOGI(TAG, "%d popped from stack\n", pop(&root));

  //   LOGI(TAG, "Top element is %d\n", peek(root));

  return 0;
}

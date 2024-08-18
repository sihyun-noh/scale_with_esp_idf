// C program for linked list implementation of stack
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "LIFO_stack.h"
#include "syslog.h"

static const char* TAG = "Internal_Stack";

struct StackNode* newNode(judge_type_t type, float weight, uint16_t count, char* prod_name, char* time_date) {
  struct StackNode* stackNode = (struct StackNode*)malloc(sizeof(struct StackNode));
  stackNode->count = count;
  stackNode->weight = weight;
  stackNode->type = type;
  memset(stackNode->prod_name, 0x00, sizeof(stackNode->prod_name));
  memcpy(stackNode->prod_name, prod_name, strlen(prod_name));
  memset(stackNode->time_date, 0x00, sizeof(stackNode->time_date));
  memcpy(stackNode->time_date, time_date, strlen(time_date));
  stackNode->next = NULL;
  return stackNode;
}

int isEmpty(struct StackNode* root) {
  return !root;
}

void push(struct StackNode** root, judge_type_t type, float weight, uint16_t count, char* prod_name, char* time_date) {
  struct StackNode* stackNode = newNode(type, weight, count, prod_name, time_date);
  stackNode->next = *root;
  *root = stackNode;
  LOGI(TAG, "%.4f, %d, %d, %s ,%s pushed to stack\n", weight, type, count, prod_name, time_date);
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

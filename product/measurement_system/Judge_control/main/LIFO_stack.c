// C program for linked list implementation of stack
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "LIFO_stack.h"
#include "syslog.h"

static const char* TAG = "stack_";

struct StackNode* newNode(int data) {
  struct StackNode* stackNode = (struct StackNode*)malloc(sizeof(struct StackNode));
  stackNode->data = data;
  stackNode->next = NULL;
  return stackNode;
}

int isEmpty(struct StackNode* root) {
  return !root;
}

void push(struct StackNode** root, int data) {
  struct StackNode* stackNode = newNode(data);
  stackNode->next = *root;
  *root = stackNode;
  LOGI(TAG, "%d pushed to stack\n", data);
}

int pop(struct StackNode** root) {
  if (isEmpty(*root))
    return INT_MIN;
  struct StackNode* temp = *root;
  *root = (*root)->next;
  int popped = temp->data;
  free(temp);

  return popped;
}

int peek(struct StackNode* root) {
  if (isEmpty(root))
    return INT_MIN;
  return root->data;
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

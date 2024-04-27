#ifndef _LIFO_STACK_H_
#define _LIFO_STACK_H_

#ifdef _cplusplus
extern "C" {
#endif

// A structure to represent a stack
struct StackNode {
  int data;
  struct StackNode* next;
};

struct StackNode* newNode(int data);
int isEmpty(struct StackNode* root);
void push(struct StackNode** root, int data);
int pop(struct StackNode** root);
int peek(struct StackNode* root);
int create_stack();
#ifdef _cplusplus
}
#endif

#endif

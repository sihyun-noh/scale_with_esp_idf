#ifndef _LIFO_STACK_H_
#define _LIFO_STACK_H_

#ifdef _cplusplus
extern "C" {
#endif
#include "ui.h"

typedef enum {
  JUDGE_LACK = 0x01,
  JUDGE_NORMAL,
  JUDGE_OVER,
} judge_type_t;

// A structure to represent a stack
struct StackNode {
  uint16_t count;
  float weight;
  judge_type_t type;
  struct StackNode* next;
};

struct StackNode* newNode(judge_type_t type, float weight, uint16_t count);
int isEmpty(struct StackNode* root);
void push(struct StackNode** root, judge_type_t type, float weight, uint16_t count);
int pop(struct StackNode** root);
int peek(struct StackNode* root);
int create_stack();
#ifdef _cplusplus
}
#endif

#endif

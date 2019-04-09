#ifndef LINKED_LIST_HEADER
#define LINKED_LIST_HEADER

typedef struct node Node;

typedef struct list List;

struct node {
  SOCKET data;
  struct node * next;
};

struct list {
  int size;
  Node * head;
};

List * makelist();
void add(SOCKET data, List * list);
void delete(SOCKET data, List * list);
int size(List * list);
void display(List * list);
void reverse(List * list);
void destroy(List * list);

#endif

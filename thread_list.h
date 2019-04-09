#ifndef THREAD_LIST_HEADER
#define THREAD_LIST_HEADER

typedef struct node_t Node_t;

typedef struct list_t List_t;

struct node_t {
  pthread_t data;
  struct node_t * next;
};

struct list_t {
  int size;
  Node_t * head;
};

List_t * makelist_t();
void add_t(pthread_t data, List_t * list);
int size_th(List_t * list);
void delete_t(pthread_t data, List_t * list);
void display_t(List_t * list);
void reverse_t(List_t * list);
void destroy_t(List_t * list);

#endif

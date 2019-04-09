#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <pthread.h>
#include "thread_list.h"




Node_t * createnode_t(pthread_t data){
  Node_t * newNode = malloc(sizeof(Node_t));
  if (!newNode) {
    return NULL;
  }
  newNode->data = data;
  newNode->next = NULL;
  return newNode;
}

int size_th(List_t* list){
    return list->size;
}

List_t * makelist_t(){
  List_t * list = malloc(sizeof(List_t));
  if (!list) {
    return NULL;
  }
  list->size = 0;
  list->head = NULL;
  return list;
}

void display_t(List_t * list) {
  Node_t * current = list->head;
  if(list->head == NULL)
    return;

  for(; current != NULL; current = current->next) {
    printf("%d\n", current->data);
  }
}

void add_t(pthread_t data, List_t * list){
  Node_t * current = NULL;
  if(list->head == NULL){
    list->head = createnode_t(data);
  }
  else {
    current = list->head;
    while (current->next!=NULL){
      current = current->next;
    }
    current->next = createnode_t(data);
  }
  list->size ++;
}

void delete_t(pthread_t data, List_t * list){
  Node_t * current = list->head;
  Node_t * previous = current;
  while(current != NULL){
    if(current->data == data){
      previous->next = current->next;
      if(current == list->head)
        list->head = current->next;
      free(current);
      return;
    }
    previous = current;
    current = current->next;
  }
  list->size --;
}

void reverse_t(List_t * list){
  Node_t * reversed = NULL;
  Node_t * current = list->head;
  Node_t * temp = NULL;
  while(current != NULL){
    temp = current;
    current = current->next;
    temp->next = reversed;
    reversed = temp;
  }
  list->head = reversed;
}

void destroy_t(List_t * list){
  Node_t * current = list->head;
  Node_t * next = current;
  while(current != NULL){
    next = current->next;
    free(current);
    current = next;
  }
  free(list);
}

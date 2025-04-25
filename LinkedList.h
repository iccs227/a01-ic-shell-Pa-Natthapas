#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct Node{
    char* items;
    int Size;
    struct Node* next;
} Node;

Node* LinkedList();
void DeleteList(Node* LinkedList);
int getSize(Node* LinkedList);
char* getItems(Node* LinkedList, int index);
void addFirst(Node* LinkedList, char* item);
void addLast(Node* LinkedList, char* item);
void printLL(Node* LinkedList);


#endif
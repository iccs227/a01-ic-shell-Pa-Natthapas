#ifndef PROCESSTRACKER_H
#define PROCESSTRACKER_H

typedef struct Node{
    char* items;
    int Size;
    struct Node* next;
} Node;

Node* ProcessTracker();
void DeleteList(Node* Sentinel);
void addFirst(Node* Sentinel, int item);
void addLast(Node* Sentinel, int item);
void deleteNode(Node* Sentinel, int index);
void printLL(Node* Sentinel);
Node* copy(Node* Sentinel);

#endif
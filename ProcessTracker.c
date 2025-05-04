#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node{
    int items;
    int Order;

    int Size;
    struct Node* next;
    struct Node* prev;
} Node;

Node* ProcessTracker(){ // basically a LL
    Node* Sentinel = malloc(sizeof(Node));
    Sentinel->Size = 0;
    Sentinel->items = -11111111;
    Sentinel->next = NULL;
    Sentinel->prev = NULL;
    return Sentinel;
}

void DeleteList(Node* Sentinel){
    if (Sentinel == NULL){
        return;
    }
    Node* walk;
    int size = Sentinel->Size;
    for (int i = 0; i < size; i++){
        walk = Sentinel;
        Sentinel = Sentinel->next;
        free(walk);
    }
    free(Sentinel);
}

int getSize(Node* Sentinel){
    return Sentinel->Size;
}

void addFirst(Node* Sentinel, int item){
    Node* first = malloc(sizeof(Node));

    first->prev = Sentinel;
    first->next = Sentinel->next;
    

    if (Sentinel->Size == 0){
        Sentinel->next = first;
        Sentinel->prev = first;
        first->next = Sentinel;
    }
    else {
        Sentinel->next->prev = first;
        Sentinel->next = first;
        first->prev = Sentinel;
    }

    Sentinel->Size++;
    first->Order = Sentinel->Size;
}

void addLast(Node* Sentinel, int item){
    Node* last = malloc(sizeof(Node));
    last->next = Sentinel;
    last->prev = Sentinel->prev;
    if (Sentinel->Size = 0){
        Sentinel->next = last;
        Sentinel->prev = last;
        last->prev = Sentinel;
    }
    else {
        Sentinel->prev->next = last;
        Sentinel->prev = last;
        last->next = Sentinel;
    }

    Sentinel->Size++;
    last->Order = Sentinel->Size;
}

void deleteNode(Node* Sentinel, int index){
    Node* walk = Sentinel;
    if (index <= (Sentinel->Size)/2){
        for (int i = 0; i < index; i++){
            walk = walk->next;
        }
    }
    else if (index > (Sentinel->Size)/2){ 
        for (int i = 0; i <= ((Sentinel->Size)-index); i++){
            walk = walk->prev;
        }
    }
    walk->prev->next = walk->next;
    walk->next->prev = walk->prev;
    Sentinel->Size--;
    free(walk);
}

void printLL(Node* Sentinel){
    Node* walk = Sentinel->next;
    while (walk->next != NULL){
        printf("%s ", walk->items);
        walk = walk->next;
    }
}

Node* copy(Node* Sentinel){
    Node* copy = LinkedList();
    Node* walk = Sentinel;
    int index = 0;
    while (walk->next != NULL){
        addLast(copy, strdup(getItems(Sentinel, index)));
        // printf("%s", getItems(Sentinel, index));
        walk = walk->next;
        index++;
    }
    return copy;
}

// int main(){
//     Node* test = Sentinel();
//     addFirst(test,"six");
//     addFirst(test,"five");
//     addFirst(test,"four");
//     addFirst(test,"three");
//     addFirst(test,"two");
//     addFirst(test,"one");
//     addFirst(test,"zero");
//     addLast(test,"seven");
//     addLast(test,"eight");
//     addLast(test,"nine");
//     addLast(test,"ten");
//     addLast(test,"eleven");

//     printLL(test);
//     printf("\n%s", getItem(test, 0));
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node{
    char* items;
    int Size;
    struct Node* next;
} Node;

Node* LinkedList(){
    Node* Sentinel = malloc(sizeof(Node));
    Sentinel->Size = 0;
    Sentinel->items = "Sentinel";
    Sentinel->next = NULL;

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

char* getItems(Node* Sentinel, int index){
    Node* walk = Sentinel->next;
    if (index > getSize(Sentinel)){
        printf("Index out of range.\n");
        return NULL;
    }
    for (int i = 0; i < index; i++){
        walk = walk->next;
    }
    return walk->items;
}

void addFirst(Node* Sentinel, char* item){
    Node* first = malloc(sizeof(Node));
    first->items = item;

    if (Sentinel->Size == 0){
        first->next = NULL;
        Sentinel->next = first;
    }
    else {
        first->next = Sentinel->next;
        Sentinel->next = first;
    }
    Sentinel->Size++;
}

void addLast(Node* Sentinel, char* item){
    Node* last = malloc(sizeof(Node));
    last->items = item;
    last->next = NULL;

    if (Sentinel->Size == 0){
        Sentinel->next = last;
    }
    else{
        Node* walk = NULL;
        for (walk = Sentinel; walk->next!=NULL; walk = walk->next);
        walk->next = last;
    }
    Sentinel->Size++;
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


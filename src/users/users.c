#include "users.h"
#include <stdio.h>
#include <string.h>
#include "../logger/logger.h"


bool isEmpty();
void insertUserNode(char * name, char * password);
void deleteUserNode(char* name);
void removeAlluserNodes();


user_linked_list_singleton * createSingletonUserLinkedListInstance(){
    static user_linked_list_singleton instance;
    instance.head = NULL;
    instance.size = 0;
    return &instance;
}


static user_linked_list_singleton * getSingletonInstance(){
    static user_linked_list_singleton * instance = NULL;
    if (instance == NULL){
        instance = createSingletonUserLinkedListInstance();
    }
    return instance;
}



bool validUsername(char * username){
    user_linked_list_singleton * singletonPtr = getSingletonInstance();
    bool found = false;
    for ( user_node * userPtr; userPtr != NULL && !found ; userPtr = userPtr->next){
        if (strcmp(userPtr->data.name,username) == 0){
            found = true;
        }
    }
    return found;
}


bool validPassword(char * username, char * password){
    user_linked_list_singleton * singletonPtr = getSingletonInstance();
    bool found = false;
    bool found = false;
    for ( user_node * userPtr; userPtr != NULL && !found ; userPtr = userPtr->next){
        if (strcmp(userPtr->data.name,username) == 0 && strcmp(userPtr->data.password,password)){
            found = true;
        }
    }
    return found;
}


// Function to check if the linked list is empty
bool isEmpty() {
    user_linked_list_singleton * list = getSingletonInstance();
    return list->head == NULL;
}

// Function to insert a node at the beginning of the linked list
void insertUserNode(char * name, char * password) {
    user_linked_list_singleton * list = getSingletonInstance();
    user_node* newNode = (user_node*)malloc(sizeof(user_node));
    strcpy(newNode->data.name,name);
    strcpy(newNode->data.password,password);
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}

// Function to delete a node from the linked list by name
void deleteUserNode(char* name) {
    user_linked_list_singleton * list = getSingletonInstance();
    user_node* currentNode = list->head;
    user_node* prevNode = NULL;

    while (currentNode != NULL) {
        if (strcmp(currentNode->data.name, name) == 0) {
            if (prevNode == NULL) {
                // Node to be deleted is the head
                list->head = currentNode->next;
            } else {
                prevNode->next = currentNode->next;
            }

            free(currentNode);
            list->size--;
            return;
        }

        prevNode = currentNode;
        currentNode = currentNode->next;
    }
}

// Function to remove all nodes from the linked list
void removeAllUserNodes() {
    user_linked_list_singleton * list = getSingletonInstance();
    user_node* currentNode = list->head;
    user_node* nextNode = NULL;

    while (currentNode != NULL) {
        nextNode = currentNode->next;
        free(currentNode);
        currentNode = nextNode;
    }

    list->head = NULL;
    list->size = 0;
}


void initializeUserSingleton(int initialUsers, char ** names){
    char * delim = ":";
    for ( int i = 0; i < initialUsers ; i++){
        char * username = strtok(names[i],delim);
        char * password = strtok(NULL,delim);
        insertUserNode(username,password);
    }
}
//Aaron Gordon 0884023
#include "LinkedListAPI.h"

List initializeList(char *(*printFunction)(void *toBePrinted), void (*deleteFunction)(void *toBeDeleted), int (*compareFunction)(const void *first, const void *second)) {
  List list;

  list.deleteData = deleteFunction;
  list.compare = compareFunction;
  list.printData = printFunction;
  list.head = NULL;
  list.tail = NULL;
  list.length = 0;

  return list;
}

Node *initializeNode(void *data) {
  Node *node;

  if(!data) {
    return NULL;
  }

  node = (Node*)malloc(sizeof(Node));
  node->data = data;
  node->previous = NULL;
  node->next = NULL;

  return node;
}

void insertFront(List *list, void *toBeAdded) {
  Node *node;

  if(!list) {
    return;
  }

  node = initializeNode(toBeAdded);
  if(list->head) {
    list->head->previous = node;
    node->next = list->head;
    list->head = node;
  } else {
    list->head = node;
    list->tail = node;
  }
  
  list->length++;
}

void insertBack(List *list, void *toBeAdded) {
  Node *node;

  if(!list) {
    return;
  }

  node = initializeNode(toBeAdded);
  if(list->tail) {
    list->tail->next = node;
    node->previous = list->tail;
    list->tail = node;
  } else {
    list->head = node;
    list->tail = node;
  }
  
  list->length++;
}

void clearList(List *list) {
  Node *curr;
  Node *next;

  if(!list) {
    return;
  }

  curr = list->head;
  while(curr) {
    next = curr->next;
    list->deleteData(curr->data);
    free(curr);
    curr = next;
  }

  list->head = NULL;
  list->tail = NULL;
  list->length = 0;
}

void insertSorted(List *list, void *toBeAdded) {
  Node *node;
  Node *curr;

  if(!list) {
    return;
  }

  node = initializeNode(toBeAdded);
  curr = list->head;

  while(curr) {
    if(list->compare(toBeAdded, curr->data) < 0) {
      if(!curr->previous) {
        curr->previous = node;
        list->head = node;
        list->head->next = curr;
      } else {
        curr->previous->next = node;
        node->previous = curr->previous;
        curr->previous = node;
        node->next = curr;
      }

      break;
    }

    curr = curr->next;
  }

  if(!curr && !list->head) {
    list->head = node;
    list->tail = node;
  } else if(!curr) {
    list->tail->next = node;
    node->previous = list->tail;
    list->tail = node;
  }
  
  list->length++;
}

void *deleteDataFromList(List *list, void *toBeDeleted) {
  Node *curr;

  if(!list || !list->head || !toBeDeleted) {
    return NULL;
  }

  curr = list->head;
  while(curr) {
    if(list->compare(curr->data, toBeDeleted) == 0) {
      list->deleteData(curr->data);
      break;
    }

    curr = curr->next;
  }

  if(curr) {
    if(curr->previous) {
      curr->previous->next = curr->next;
    } else {
      list->head = curr->next;
    }

    if(curr->next) {
      curr->next->previous = curr->previous;
    } else {
      list->tail = curr->previous;
    }

    free(curr);
  } else {
    return NULL;
  }

  list->length--;
  return toBeDeleted;
}

void *getFromFront(List list) {
  if(!list.head || !list.head->data) {
    return NULL;
  }

  return list.head->data;
}

void *getFromBack(List list) {
  if(!list.tail || !list.tail->data) {
    return NULL;
  }

  return list.tail->data;
}

char *toString(List list) {
  char *str = NULL;
  char *tempStr;
  int numChars = 0;

  while(list.head) {
    tempStr = list.printData(list.head->data);
    numChars += strlen(tempStr);

    if(!str) {
      str = (char*)malloc(numChars + 1);
      strcpy(str, tempStr);
    } else {
      str = (char*)realloc(str, numChars + 1);
      strcat(str, tempStr);
    }

    list.head = list.head->next;
    free(tempStr);
  }

  return str;
}

ListIterator createIterator(List list) {
  ListIterator iter;

  iter.current = list.head;
  return iter;
}

void *nextElement(ListIterator *iter) {
  void *data;

  if(!iter || !iter->current) {
    return NULL;
  }

  data = iter->current->data;
  iter->current = iter->current->next;

  return data;
}

int getLength(List list) {
  return list.length;
}

void* findElement(List list, bool (*customCompare)(const void* first, const void* second), const void* searchRecord) {
  Node *node;
  
  node = list.head;
  while(node) {
    if(customCompare(node->data, searchRecord)) {
      return node->data;
    }
    
    node = node->next;
  }
  
  return NULL;
}

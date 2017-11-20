//Aaron Gordon 0884023
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "CalendarParser.h"
#include "LinkedListAPI.h"
#include "Helpers.h"

int getEventNum(Calendar *cal) {
  return getLength(cal->events);
}

void deleteEvent(Calendar* obj, int index) {
  int i;
  Event *event;
  Node *head = obj->events.head;
  
  index--;
  for(i = 0; i < index; i++) {
    head = head->next;
  }

  if(head->previous) {
    head->previous->next = head->next;
  } else {
    obj->events.head = head->next;
  }
  
  if(head->next) {
    head->next->previous = head->previous;
  } else {
    obj->events.tail = head->next;
  }

  obj->events.length--;
  event = (Event*)head->data;
  clearList(&event->properties);
  clearList(&event->alarms);
  free(event);
}

char *printEvent(Calendar *cal, int index) {
  char *toReturn;
  int i;
  Node *head = cal->events.head;
  
  index--;
  for(i = 0; i < index; i++) {
    head = head->next;
  }
  
  toReturn = printFuncEvent(head->data);
  return toReturn;
}

ICalErrorCode calOpenFail(char *filename) {
  Calendar *cal;
 
  return createCalendar(filename, &cal); 
}

Calendar *calOpen(char *filename) {
  Calendar *cal;
  ICalErrorCode err;

  err = createCalendar(filename, &cal);
  if(err != OK) {
    return NULL;
  }
    
  return cal;
}

Calendar *customCal(char args[2][1000]) {
  Calendar *cal;
  
  cal = (Calendar*)malloc(sizeof(Calendar));
  cal->version = 2.0;
  cal->properties.head = NULL;
  cal->properties.length = 0;
  cal->events.head = NULL;
  cal->events.length = 0;
  strcpy(cal->prodID, args[1]);
  
  return cal;
}

Calendar *customEvent(Calendar *cal, char args[4][1000]) {
  Event *event;
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  
  event = (Event*)malloc(sizeof(Event));
  event->properties = initializeList(&printFuncProp, &deleteFuncProp, &compareFuncProp);
  event->alarms = initializeList(&printFuncAlarm, &deleteFuncAlarm, &compareFuncAlarm);
  strcpy(event->UID, args[0]);
  
  strftime(event->creationDateTime.date, 100, "%Y%m%d", &tm);
  strftime(event->creationDateTime.time, 100, "%H%M%S", &tm);
  event->creationDateTime.UTC = 1;
  
  strcpy(event->startDateTime.date, args[1]);
  strcpy(event->startDateTime.time, args[2]);  
  event->startDateTime.UTC = atoi(args[3]);
  
  if(!cal->events.head) {
    cal->events = initializeList(&printFuncEvent, &deleteFuncEvent, &compareFuncEvent);
  }
  
  insertBack(&cal->events, (void*)event); 
  return cal;
}

Calendar *customAlarm(Calendar *cal, char args[2][1000], int index) {
  int i;
  Alarm *alarm;
  Event *event;
  Node *head = cal->events.head;
  
  alarm = (Alarm*)malloc(sizeof(Alarm));
  alarm->properties.head = NULL;
  alarm->trigger = (char*)malloc(sizeof(char) * (strlen(args[1]) + 1));
  
  strcpy(alarm->action, args[0]);
  strcpy(alarm->trigger, args[1]); 
  
  for(i = 0; i < index; i++) {
    head = head->next;
  }
  
  event = (Event*)head->data;
  if(!event->alarms.head) {
    event->alarms = initializeList(&printFuncAlarm, &deleteFuncAlarm, &compareFuncAlarm);
  }
  
  insertBack(&event->alarms, (void*)alarm); 
  return cal;
}

void getEventArr(const Calendar* obj, int index, char **strings) {
  char strArg[256];
  int i;
  void *element;
  Event *event;
  Node *head = obj->events.head;
  Property *summ;
  
  for(i = 0; i < index; i++) {
    head = head->next;
  }
  
  event = (Event*)head->data;
  strcpy(strings[0], "0");
  strcpy(strings[1], "0");
  strcpy(strings[2], "");

  if(event->properties.head) {
    sprintf(strings[0], "%d", getLength(event->properties));
  }
 
  if(event->alarms.length != 0) {
    sprintf(strings[1], "%d", getLength(event->alarms));
  }
  
  strcpy(strArg, "SUMMARY");
  element = findElement(event->properties, compareProp, strArg);
  if(element) {
    summ = (Property*)element;
    strcpy(strings[2], summ->propDescr);
  }
}

char *printFuncEvent(void *toBePrinted) {
  char *toReturn;
  char *temp;
  int len;
  Event *event;

  if(!toBePrinted) {
    return NULL;
  }

  event = (Event*)toBePrinted;
  toReturn = (char*)malloc(sizeof(char)
    * (strlen(event->UID) + strlen(event->creationDateTime.date) + strlen(event->creationDateTime.time) + 80));
    
  sprintf(toReturn, "Event\n\tUID = %s\n\tcreationDateTime = %s:%s, UTC=%d\n" , event->UID, event->creationDateTime.date, event->creationDateTime.time, event->creationDateTime.UTC);
  sprintf(toReturn, "%s\tstartDateTime = %s:%s, UTC=%d\n", toReturn, event->startDateTime.date, event->startDateTime.time, event->startDateTime.UTC);
    
  len = strlen(toReturn);
  
  if(event->alarms.head) {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + 10));
    strcat(toReturn, "\tAlarms:\n");
    len = strlen(toReturn);
  }
  
  temp = toString(event->alarms);
  if(temp) {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 1));
    strcat(toReturn, temp);
    len = strlen(toReturn);
    free(temp);
  }
  
  if(event->properties.head) {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + 20));
    strcat(toReturn, "\tOther Properties:\n");
    len = strlen(toReturn);
  }
  
  temp = toString(event->properties);
  if(temp) {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 1));
    strcat(toReturn, temp);
    len = strlen(toReturn);
    free(temp);
  }

  toReturn = (char*)realloc(toReturn, sizeof(char) * (len + 2));
  strcat(toReturn, "\n");

  return toReturn;
}

char* printFuncAlarm(void *toBePrinted) {
  char *toReturn;
  char *temp;
  int len;
  Alarm *alarm;

  if(!toBePrinted) {
    return NULL;
  }

  alarm = (Alarm*)toBePrinted;
  toReturn = (char*)malloc(sizeof(char) * (strlen(alarm->action) + strlen(alarm->trigger) + 38));
  sprintf(toReturn, "\t\tAction: %s\n\t\tTrigger: %s\n", alarm->action, alarm->trigger);
  len = strlen(toReturn);
  
  temp = toString(alarm->properties);
  if(temp) {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 30));
    strcat(toReturn, "\t\tProperties:\n");
    strcat(toReturn, temp);
    len = strlen(toReturn);
    free(temp);
  }

  toReturn = (char*)realloc(toReturn, sizeof(char) * (len + 2));
  strcat(toReturn, "\n");

  return toReturn;
}

char* printFuncProp(void *toBePrinted) {
  char *toReturn;
  Property *prop;

  if(!toBePrinted) {
    return NULL;
  }

  prop = (Property*)toBePrinted;
  toReturn = (char*)malloc(sizeof(char) * (strlen(prop->propName) + strlen(prop->propDescr) + 7));
  sprintf(toReturn, "\t\t- %s:%s\n", prop->propName, prop->propDescr);

  return toReturn;
}

int compareFuncEvent(const void *first, const void *second) {
  Event *event1;
  Event *event2;

  if(!first || !second) {
    return 0;
  }

  event1 = (Event*)first;
  event2 = (Event*)second;

  return strcmp((char*)event1->UID, (char*)event2->UID);
}

int compareFuncAlarm(const void *first, const void *second) {
  Alarm *alarm1;
  Alarm *alarm2;

  if(!first || !second) {
    return 0;
  }

  alarm1 = (Alarm*)first;
  alarm2 = (Alarm*)second;

  return strcmp((char*)alarm1->action, (char*)alarm2->action);
}

int compareFuncProp(const void *first, const void *second) {
  Property *prop1;
  Property *prop2;

  if(!first || !second) {
    return 0;
  }

  prop1 = (Property*)first;
  prop2 = (Property*)second;

  return strcmp((char*)prop1->propName, (char*)prop2->propName);
}

void deleteFuncEvent(void *toBeDeleted) {
  Event *event;
  
  if(!toBeDeleted) {
    return;
  }
  
  event = (Event*)toBeDeleted;
  clearList(&event->properties);
  clearList(&event->alarms);
  
  free(event);
}

void deleteFuncAlarm(void *toBeDeleted) {
  Alarm *alarm;

  if(!toBeDeleted) {
    return;
  }

  alarm = (Alarm*)toBeDeleted;
  clearList(&alarm->properties);

  if(alarm->trigger) {
    free(alarm->trigger);
  }

  free(toBeDeleted);
}

void deleteFuncProp(void *toBeDeleted) {
  if(!toBeDeleted) {
    return;
  }

  free(toBeDeleted);
}

bool compareProp(const void *first, const void *second) {
  char *title;
  Property *prop;

  prop = (Property*)first;
  title = (char*)second;
  
  if(strcmp((char*)prop->propName, title) == 0) {
    return true;
  } else {
    return false;
  }
}

bool findDiffProp(const void *first, const void *second) {
  Property *prop1;
  Property *prop2;

  prop1 = (Property*)first;
  prop2 = (Property*)second;
  
  return (strcmp((char*)prop1->propName, (char*)prop2->propName) == 0
    && strcmp((char*)prop1->propDescr, (char*)prop2->propDescr) != 0);
}

void clearSpaces(char *toClear) {
  char *firstChar;
  size_t lineLen;

  firstChar = toClear;
  /*while(*firstChar != '\0' && isspace(*firstChar)) {
    ++firstChar;
  }*/

  lineLen = strlen(firstChar) + 1;
  memmove(toClear, firstChar, lineLen);

  if(toClear[strlen(toClear) - 2] == '\r' && toClear[strlen(toClear) - 1] == '\n') {
    toClear[strlen(toClear) - 2] = '\0';
  } else if(toClear[strlen(toClear) - 1] == '\r' || toClear[strlen(toClear) - 1] == '\n') {
    toClear[strlen(toClear) - 1] = '\0';
  }
}

ICalErrorCode badError(Calendar *cal, FILE *file, Calendar **obj, ICalErrorCode err) {
  deleteCalendar(cal);
  if(file) {
    fclose(file);
  }

  obj = NULL;
  return err;
}

ICalErrorCode createCalendar(char* fileName, Calendar** obj) {
  char *token;
  char *tempFile;
  char strArg[2048];
  char data[2048];
  char foldedLine[2048];
  char line[2048];
  char temp[2048];
  char title[2048];
  char notFoldedChar = '0';
  int calEnded = 0;
  int inAlarm = 0;
  int inEvent = 0;
  Alarm *alarm;
  Calendar *cal = (Calendar*)malloc(sizeof(Calendar));
  Event *event;
  FILE *file;
  Property *prop;
  
  cal->version = 0.0;
  cal->events.head = NULL;
  cal->properties.head = NULL;
  strcpy(cal->prodID, "");
  
  if(!fileName || strcmp(fileName, "") == 0) {
    return badError(cal, NULL, obj, INV_FILE);
  }

  tempFile = (char*)malloc(sizeof(char) * (strlen(fileName) + 1));
  strcpy(tempFile, fileName);

  token = strtok(tempFile, ".");
  if(token != NULL) {
    token = strtok(NULL, ".");
  }

  if(!token || strcmp(token, "ics") != 0) {
    free(tempFile);
    return badError(cal, NULL, obj, INV_FILE);
  }

  free(tempFile);
  file = fopen(fileName, "r");
  
  if(!file) {
    return badError(cal, NULL, obj, INV_FILE);
  } else if(!fgets(line, sizeof(line), file)) {
    return badError(cal, file, obj, INV_FILE);
  }

  if(notFoldedChar != '0') {
    strcpy(temp, line);
    strcpy(line, "");
    
    line[0] = notFoldedChar;
    line[1] = '\0';
    strcat(line, temp);
    
    notFoldedChar = '0';
  }
  
  clearSpaces(line);
  while(isspace(notFoldedChar = fgetc(file))) {
    fgets(foldedLine, sizeof(foldedLine), file);
    clearSpaces(foldedLine);
    strcat(line, foldedLine);
  }
  
  if(strcmp(line, "BEGIN:VCALENDAR") != 0) {
    return badError(cal, file, obj, INV_CAL);
  }

  while(fgets(line, sizeof(line), file)) {
    if((line[strlen(line) - 2] != '\r' && line[strlen(line) - 2] != '\n') && inAlarm) {
      return badError(cal, file, obj, INV_ALARM);
    } else if((line[strlen(line) - 2] != '\r' && line[strlen(line) - 2] != '\n') && inEvent) {
      return badError(cal, file, obj, INV_EVENT);
    } else if(calEnded || (line[strlen(line) - 2] != '\r' && line[strlen(line) - 2] != '\n')) {
      return badError(cal, file, obj, INV_CAL);
    }

    if(notFoldedChar != '0') {
      strcpy(temp, line);
      strcpy(line, "");
      
      line[0] = notFoldedChar;
      line[1] = '\0';
      strcat(line, temp);
      
      notFoldedChar = '0';
    }
    
    if(line[0] == ';') {
      continue;
    }
    
    clearSpaces(line);
    while(isspace(notFoldedChar = fgetc(file))) {
      fgets(foldedLine, sizeof(foldedLine), file);
      clearSpaces(foldedLine);
      strcat(line, foldedLine);
    }

    token = strtok(line, ":;");
    if(!token && inAlarm) {
      return badError(cal, file, obj, INV_ALARM);
    } else if(!token && inEvent) {
      return badError(cal, file, obj, INV_EVENT);
    } else if(!token) {
      return badError(cal, file, obj, INV_CAL);
    }  
    strcpy(title, token);
    
    token = strtok(NULL, "");  
    if(!token && inAlarm) {
      return badError(cal, file, obj, INV_ALARM);
    } else if(!token && inEvent) {
      if(strcmp(title, "DTSTAMP") == 0) {
        return badError(cal, file, obj, INV_CREATEDT);
      } else {
        return badError(cal, file, obj, INV_EVENT);
      }
    } else if(!token) {
      if(strcmp(title, "VERSION") == 0) {
        return badError(cal, file, obj, INV_VER);
      } else if(strcmp(title, "PRODID") == 0) {
        return badError(cal, file, obj, INV_PRODID);
      } else {
        return badError(cal, file, obj, INV_CAL);
      }
    }
    strcpy(data, token);

    if(strcmp(title, "BEGIN") == 0) {
      if(strcmp(data, "VEVENT") == 0) {
        if(inEvent) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        inEvent = 1;           
        event = (Event*)malloc(sizeof(Event));
        event->properties.head = NULL;
        event->alarms.head = NULL;
        strcpy(event->UID, "");
        strcpy(event->creationDateTime.date, "");
        strcpy(event->startDateTime.date, "");
      } else if(strcmp(data, "VALARM") == 0) {
        if(inAlarm) {
          return badError(cal, file, obj, INV_ALARM);
        } else if(!inEvent) {
          return badError(cal, file, obj, INV_CAL);
        }
        
        inAlarm = 1;   
        alarm = (Alarm*)malloc(sizeof(Alarm));
        strcpy(alarm->action, "");
        alarm->trigger = NULL;
        alarm->properties.head = NULL;
      } else {
        if(inAlarm) {
          return badError(cal, file, obj, INV_ALARM);
        } else if(inEvent) {
          return badError(cal, file, obj, INV_EVENT);
        } else {
          return badError(cal, file, obj, INV_CAL);
        }
      }
    } else if(strcmp(title, "END") == 0) {
      if(strcmp(data, "VCALENDAR") == 0) {
        if(inAlarm) {
          return badError(cal, file, obj, INV_ALARM);
        } else if(inEvent) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        calEnded = 1;
      } else if(strcmp(data, "VEVENT") == 0) {
        if(!inEvent) {
          return badError(cal, file, obj, INV_CAL);
        } else if(inAlarm) {
          return badError(cal, file, obj, INV_ALARM);
        } else if(strcmp(event->UID, "") == 0 || strcmp(event->creationDateTime.date, "") == 0 || strcmp(event->startDateTime.date, "") == 0) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        if(!cal->events.head) {
          cal->events = initializeList(&printFuncEvent, &deleteFuncEvent, &compareFuncEvent);
        }
        
        inEvent = 0;
        insertBack(&cal->events, (void*)event);
        event = NULL;
      } else if(strcmp(data, "VALARM") == 0) {
        if(!inAlarm && inEvent) {
          return badError(cal, file, obj, INV_EVENT);
        } else if(!inAlarm && !inEvent) {
          return badError(cal, file, obj, INV_CAL);
        } else if(strcmp(alarm->action, "") == 0 || !alarm->trigger) {
          return badError(cal, file, obj, INV_ALARM);
        }
        
        strcpy(strArg, "DURATION");
        if(findElement(alarm->properties, compareProp, strArg)) {
          strcpy(strArg, "REPEAT");
          if(!findElement(alarm->properties, compareProp, strArg)) {
            return badError(cal, file, obj, INV_ALARM);
          }
        }
        
        strcpy(strArg, "REPEAT");
        if(findElement(alarm->properties, compareProp, strArg)) {
          strcpy(strArg, "DURATION");
          if(!findElement(alarm->properties, compareProp, strArg)) {
            return badError(cal, file, obj, INV_ALARM);
          }
        }
        
        if(!event->alarms.head) {
          event->alarms = initializeList(&printFuncAlarm, &deleteFuncAlarm, &compareFuncAlarm);
        }
        
        inAlarm = 0;
        insertBack(&event->alarms, (void*)alarm);
        alarm = NULL;
      } else {
        if(inAlarm) {
          return badError(cal, file, obj, INV_ALARM);
        } else if(inEvent) {
          return badError(cal, file, obj, INV_EVENT);
        } else {
          return badError(cal, file, obj, INV_CAL);
        }
      }
    } else if(strcmp(title, "VERSION") == 0) {
      if(cal->version != 0.0) {
        return badError(cal, file, obj, DUP_VER);
      } else if(inEvent) {
        return badError(cal, file, obj, INV_EVENT);
      } else if(inAlarm) {
        return badError(cal, file, obj, INV_ALARM);
      }
      
      cal->version = atof(data);
      if(cal->version == 0.0) {
        return badError(cal, file, obj, INV_VER);
      }
    } else if(strcmp(title, "PRODID") == 0) {
      if(strcmp(cal->prodID, "") != 0) {
        return badError(cal, file, obj, DUP_PRODID);
      } else if(inEvent) {
        return badError(cal, file, obj, INV_EVENT);
      } else if(inAlarm) {
        return badError(cal, file, obj, INV_ALARM);
      }

      strcpy(cal->prodID, data);
    } else if(strcmp(title, "CALSCALE") == 0 || strcmp(title, "METHOD") == 0) {
      if(inEvent) {
        return badError(cal, file, obj, INV_EVENT);
      } else if(inAlarm) {
        return badError(cal, file, obj, INV_ALARM);
      }
      
      strcpy(strArg, title);
      if(!cal->properties.head) {
        cal->properties = initializeList(&printFuncProp, &deleteFuncProp, &compareFuncProp);
      } else if(findElement(cal->properties, compareProp, strArg)) {
        return badError(cal, file, obj, INV_CAL);
      }
      
      prop = (Property*)malloc(sizeof(Property) + (sizeof(char) * strlen(token) + 1));
      strcpy(prop->propName, title);
      strcpy(prop->propDescr, data);
      insertBack(&cal->properties, (void*)prop);
    } else {
      if(!inEvent) {
        return badError(cal, file, obj, INV_CAL);
      }
      
      if(strcmp(title, "DTSTAMP") == 0) {
        if(strcmp(event->creationDateTime.date, "") != 0) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        token = strtok(data, "T");
        if(!token || strlen(token) != 8) {
          return badError(cal, file, obj, INV_CREATEDT);
        }

        strcpy(event->creationDateTime.date, token);
        token = strtok(NULL, "T");
        if(!token || (strlen(token) != 7 && strlen(token) != 6)) {
          return badError(cal, file, obj, INV_CREATEDT);
        }

        event->creationDateTime.UTC = 0;
        if(token[6] == 'Z') {
          event->creationDateTime.UTC = 1;
        }

        token[6] = '\0';
        strcpy(event->creationDateTime.time, token);
      } else if(strcmp(title, "DTSTART") == 0) {
        if(strcmp(event->startDateTime.date, "") != 0) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        token = strtok(data, "T");
        if(!token || strlen(token) != 8) {
          return badError(cal, file, obj, INV_CREATEDT);
        }

        strcpy(event->startDateTime.date, token);
        token = strtok(NULL, "T");
        if(!token || (strlen(token) != 7 && strlen(token) != 6)) {
          return badError(cal, file, obj, INV_CREATEDT);
        }

        event->startDateTime.UTC = 0;
        if(token[6] == 'Z') {
          event->startDateTime.UTC = 1;
        }

        token[6] = '\0';
        strcpy(event->startDateTime.time, token);
      
      } else if(strcmp(title, "UID") == 0) {
        if(strcmp(event->UID, "") != 0) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        strcpy(event->UID, data);
      } else if(strcmp(title, "CLASS") == 0 || strcmp(title, "CREATED") == 0 || strcmp(title, "DESCRIPTION") == 0
        || strcmp(title, "GEO") == 0 || strcmp(title, "LAST-MODIFIED") == 0 || strcmp(title, "LOCATION") == 0
        || strcmp(title, "ORGANIZER") == 0 || strcmp(title, "PRIORITY") == 0 || strcmp(title, "SEQUENCE") == 0
        || strcmp(title, "STATUS") == 0 || strcmp(title, "SUMMARY") == 0 || strcmp(title, "TRANSP") == 0
        || strcmp(title, "URL") == 0 || strcmp(title, "RECURRENCE-ID") == 0 || strcmp(title, "RRULE") == 0
        || strcmp(title, "DTSTART") == 0) {
        
        strcpy(strArg, title);
        if(findElement(event->properties, compareProp, strArg)) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        if(!event->properties.head) {
          event->properties = initializeList(&printFuncProp, &deleteFuncProp, &compareFuncProp);
        }
        
        prop = (Property*)malloc(sizeof(Property) + (sizeof(char) * strlen(data) + 1));
        strcpy(prop->propName, title);
        strcpy(prop->propDescr, data);
        insertBack(&event->properties, (void*)prop);
      } else if(strcmp(title, "DTEND") == 0 || (strcmp(title, "DURATION") == 0 && !inAlarm)) {
        strcpy(strArg, "DTEND");
        if(findElement(event->properties, compareProp, strArg)) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        strcpy(strArg, "DURATION");
        if(findElement(event->properties, compareProp, strArg)) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        if(!event->properties.head) {
          event->properties = initializeList(&printFuncProp, &deleteFuncProp, &compareFuncProp);
        }
        
        prop = (Property*)malloc(sizeof(Property) + (sizeof(char) * strlen(data) + 1));
        strcpy(prop->propName, title);
        strcpy(prop->propDescr, data);
        insertBack(&event->properties, (void*)prop);
      } else if(strcmp(title, "ATTENDEE") == 0 || strcmp(title, "COMMENT") == 0 || strcmp(title, "CATEGORIES") == 0
        || strcmp(title, "CONTACT") == 0 || strcmp(title, "EXDATE") == 0 || strcmp(title, "REQUEST-STATUS") == 0
        || strcmp(title, "RELATED-TO") == 0 || strcmp(title, "RESOURCES") == 0 || strcmp(title, "RDATE") == 0
        || (strcmp(title, "ATTACH") == 0 && !inAlarm)) {
        
        if(!event->properties.head) {
          event->properties = initializeList(&printFuncProp, &deleteFuncProp, &compareFuncProp);
        }
        
        prop = (Property*)malloc(sizeof(Property) + (sizeof(char) * strlen(data) + 1));
        strcpy(prop->propName, title);
        strcpy(prop->propDescr, data);
        insertBack(&event->properties, (void*)prop);
      } else {
        if(!inAlarm) {
          return badError(cal, file, obj, INV_EVENT);
        }
        
        if(strcmp(title, "ACTION") == 0) {
          if(strcmp(alarm->action, "") != 0) {
            return badError(cal, file, obj, INV_ALARM);
          }
          
          strcpy(alarm->action, data);
        } else if(strcmp(title, "TRIGGER") == 0) {
          if(alarm->trigger) {
            return badError(cal, file, obj, INV_ALARM);
          }
          
          alarm->trigger = (char*)malloc(sizeof(char) * strlen(token) + 1);
          strcpy(alarm->trigger, token);
        } else if(strcmp(title, "DURATION") == 0 || strcmp(title, "REPEAT") == 0 || strcmp(title, "ATTACH") == 0) {
          strcpy(strArg, title);
          if(findElement(alarm->properties, compareProp, strArg)) {
            return badError(cal, file, obj, INV_ALARM);
          }
          
          if(!alarm->properties.head) {
            alarm->properties = initializeList(&printFuncProp, &deleteFuncProp, &compareFuncProp);
          }
          
          prop = (Property*)malloc(sizeof(Property) + (sizeof(char) * strlen(data) + 1));
          strcpy(prop->propName, title);
          strcpy(prop->propDescr, data);
          insertBack(&alarm->properties, (void*)prop);
        } else {
          return badError(cal, file, obj, INV_ALARM);
        }
      }
    }
  }

  if(cal->version == 0.0 || strcmp(cal->prodID, "") == 0 || !cal->events.head || !calEnded) {
    return badError(cal, file, obj, INV_CAL);
  } else if(inEvent) {
    return badError(cal, file, obj, INV_EVENT);
  } else if(inAlarm) {
    return badError(cal, file, obj, INV_ALARM);
  }
  
  *obj = cal;
  fclose(file);
  return OK;
}

void deleteCalendar(Calendar* obj) {
  if(!obj) {
    return;
  }
  
  clearList(&obj->events);
  clearList(&obj->properties);
  free(obj);
}

char* printCalendar(const Calendar* obj) {
  char *toReturn;
  char *temp;
  int len;

  if(!obj) {
    return NULL;
  }
  
  toReturn = (char*)malloc(sizeof(char) * (strlen(obj->prodID) + 50));
  sprintf(toReturn, "Calendar: version = %f, prodID = %s\n", obj->version, obj->prodID);
  len = strlen(toReturn);
  
  temp = toString(obj->events);
  if(temp) {
    toReturn = (char*)realloc(toReturn, sizeof(char) * (len + strlen(temp) + 1));
    strcat(toReturn, temp);
    free(temp);
  }

  return toReturn;
}

char* printError(ICalErrorCode err) {
  char *str;
  char *toReturn;

  switch(err) {
    case OK:
      str = "OK";
      break;
    case INV_FILE:
      str = "Invalid File";
      break;
    case INV_CAL:
      str = "Invalid Calendar";
      break;
    case INV_VER:
      str = "Invalid Version";
      break;
    case DUP_VER:
      str = "Multiple Versions Detected";
      break;
    case INV_PRODID:
      str = "Invalid Product ID";
      break;
    case DUP_PRODID:
      str = "Multiple Product IDs Detected";
      break;
    case INV_EVENT:
      str = "Invalid Event";
      break;
    case INV_CREATEDT:
      str = "Invalid Time Created";
      break;
    case INV_ALARM:
      str = "Invalid Alarm";
      break;
    case WRITE_ERROR:
      str = "Write Error";
      break;
    case OTHER_ERROR:
      str = "Unknown Error";
      break;
    default:
      str = "Unknown Error";
      break;
  }

  toReturn = malloc((strlen(str) + 1) * sizeof(char));
  strcpy(toReturn, str);

  return toReturn;
}

ICalErrorCode writeCalendar(char* fileName, const Calendar* obj) {
  FILE *file;
  Alarm *alarm;
  Event *event;
  ListIterator iter;
  ListIterator subIter;
  ListIterator subIter2;
  Property *prop;
  
  if(!obj) {
    return WRITE_ERROR;
  }
  
  file = fopen(fileName, "w");
  if(!file) {
    return WRITE_ERROR;
  }
  
  fprintf(file, "BEGIN:VCALENDAR\r\nPRODID:%s\r\nVERSION:%f\r\n", obj->prodID, obj->version);
  if(obj->properties.head) {
    iter = createIterator(obj->properties);
    while(iter.current) {
      prop = (Property*)nextElement(&iter);
      fprintf(file, "%s:%s\r\n", prop->propName, prop->propDescr);
    }
  }
  
  iter = createIterator(obj->events);
  while(iter.current) {
    event = (Event*)nextElement(&iter);
    fprintf(file, "BEGIN:VEVENT\r\nUID:%s\r\nDTSTAMP:%sT%s", event->UID, event->creationDateTime.date, event->creationDateTime.time);
    if(event->creationDateTime.UTC) {
      fprintf(file, "Z");
    }
    
    fprintf(file, "\r\nDTSTART:%sT%s", event->startDateTime.date, event->startDateTime.time);
    if(event->startDateTime.UTC) {
      fprintf(file, "Z");
    }
    fprintf(file, "\r\n");
    
    if(event->properties.head) {
      subIter = createIterator(event->properties);
      while(subIter.current) {
        prop = (Property*)nextElement(&subIter);      
        fprintf(file, "%s:%s\r\n", prop->propName, prop->propDescr);
      }
    }
    
    if(event->alarms.head) {
      subIter = createIterator(event->alarms);
      while(subIter.current) {
        alarm = (Alarm*)nextElement(&subIter);
        fprintf(file, "BEGIN:VALARM\r\nTRIGGER:%s\r\nACTION:%s\r\n", alarm->trigger, alarm->action);
        
        if(alarm->properties.head) {
          subIter2 = createIterator(alarm->properties);
          while(subIter2.current) {
            prop = (Property*)nextElement(&subIter2);
            fprintf(file, "%s:%s\r\n", prop->propName, prop->propDescr);
          }
        }
        fprintf(file, "END:VALARM\r\n");
      }
    }
    fprintf(file, "END:VEVENT\r\n");
  }
  fprintf(file, "END:VCALENDAR\r\n");
  
  fclose(file);
  return OK;
}

ICalErrorCode validateCalendar(const Calendar* obj) {
  char strArg[256];
  void *searchRecord;
  Alarm *alarm;
  Event *event;
  ListIterator iter;
  ListIterator subIter;
  ListIterator subIter2;
  Property *prop;
  
  if(!obj || obj->version == 0.0 || strcmp(obj->prodID, "") == 0 || !obj->events.head) {
    return INV_CAL;
  }

  if(obj->properties.head) {
    iter = createIterator(obj->properties);
    while(iter.current) {
      searchRecord = nextElement(&iter);
      prop = (Property*)searchRecord;
      
      if((strcmp(prop->propName, "CALSCALE") != 0 && strcmp(prop->propName, "METHOD") != 0)
        || findElement(obj->properties, findDiffProp, searchRecord)) {
          
        return INV_CAL;
      }
    }
  }
  
  iter = createIterator(obj->events);
  while(iter.current) {
    event = (Event*)nextElement(&iter);
    if(strcmp(event->UID, "") == 0 || strlen(event->creationDateTime.date) != 8 || strlen(event->creationDateTime.time) != 6 || strlen(event->startDateTime.date) != 8 || strlen(event->startDateTime.time) != 6) {
      return INV_EVENT;
    }
    
    if(event->properties.head) {
      subIter = createIterator(event->properties);
      while(subIter.current) {
        searchRecord = nextElement(&subIter);
        prop = (Property*)searchRecord;
        
        if((strcmp(prop->propName, "CLASS") == 0 || strcmp(prop->propName, "CREATED") == 0 || strcmp(prop->propName, "DESCRIPTION") == 0
          || strcmp(prop->propName, "GEO") == 0 || strcmp(prop->propName, "LAST-MODIFIED") == 0 || strcmp(prop->propName, "LOCATION") == 0
          || strcmp(prop->propName, "ORGANIZER") == 0 || strcmp(prop->propName, "PRIORITY") == 0 || strcmp(prop->propName, "SEQUENCE") == 0
          || strcmp(prop->propName, "STATUS") == 0 || strcmp(prop->propName, "SUMMARY") == 0 || strcmp(prop->propName, "TRANSP") == 0
          || strcmp(prop->propName, "URL") == 0 || strcmp(prop->propName, "RECURRENCE-ID") == 0 || strcmp(prop->propName, "RRULE") == 0
          || strcmp(prop->propName, "DTSTART") == 0 || strcmp(prop->propName, "DTEND") == 0 || strcmp(prop->propName, "DURATION") == 0)) {
          
          if(findElement(event->properties, findDiffProp, searchRecord)) {
            return INV_EVENT;
          }
        } else if(strcmp(prop->propName, "DTEND") == 0) {
          strcpy(strArg, "DURATION");
          if(findElement(event->properties, compareProp, strArg)) {
            return INV_EVENT;
          }
        } else if(strcmp(prop->propName, "DURATION") == 0) {
          strcpy(strArg, "DTEND");
          if(findElement(event->properties, compareProp, strArg)) {
            return INV_EVENT;
          }
        } else if(strcmp(prop->propName, "ATTENDEE") != 0 && strcmp(prop->propName, "COMMENT") != 0 && strcmp(prop->propName, "CATEGORIES") != 0
          && strcmp(prop->propName, "CONTACT") != 0 && strcmp(prop->propName, "EXDATE") != 0 && strcmp(prop->propName, "REQUEST-STATUS") != 0
          && strcmp(prop->propName, "RELATED-TO") != 0 && strcmp(prop->propName, "RESOURCES") != 0 && strcmp(prop->propName, "RDATE") != 0
          && strcmp(prop->propName, "ATTACH") != 0) {
          
          return INV_EVENT;
        }
      }
    }
    
    if(event->alarms.head) {
      subIter = createIterator(event->alarms);
      while(subIter.current) {
        alarm = (Alarm*)nextElement(&subIter);
        if(strcmp(alarm->action, "") == 0 || !alarm->trigger) {
          return INV_ALARM;
        }
        
        if(alarm->properties.head) {
          subIter2 = createIterator(alarm->properties);
          while(subIter2.current) {
            searchRecord = nextElement(&subIter2);
            prop = (Property*)searchRecord;
            
            if(strcmp(prop->propName, "DURATION") == 0) {
              strcpy(strArg, "REPEAT");
              if(!findElement(alarm->properties, compareProp, strArg) || findElement(alarm->properties, findDiffProp, searchRecord)) {
                return INV_ALARM;
              }
            } else if(strcmp(prop->propName, "REPEAT") == 0) {
              strcpy(strArg, "DURATION");
              if(!findElement(alarm->properties, compareProp, strArg) || findElement(alarm->properties, findDiffProp, searchRecord)) {
                return INV_ALARM;
              }
            } else if(strcmp(prop->propName, "ATTACH") == 0) {
              if(findElement(alarm->properties, findDiffProp, searchRecord)) {
                return INV_ALARM;
              }
            } else {
              return INV_ALARM;
            }
          }
        }
      }
    }
  }
  
  return OK;
}
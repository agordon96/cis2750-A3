#ifndef CALENDARPARSER_H
#define CALENDARPARSER_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "LinkedListAPI.h"

typedef enum ers {OK, INV_FILE, INV_CAL, INV_VER, DUP_VER, INV_PRODID, DUP_PRODID, INV_EVENT, INV_CREATEDT, INV_ALARM, WRITE_ERROR, OTHER_ERROR } ICalErrorCode;

typedef struct dt {
	char date[9]; 
	char time[7]; 
	bool UTC;  
} DateTime;

typedef struct prop {
	char propName[200]; 
	char propDescr[]; 
} Property;

typedef struct alarm {
  char action[200];
  char* trigger;
  List properties;
} Alarm;

typedef struct evt {
	char UID[1000];
  DateTime creationDateTime;
  DateTime startDateTime;
	List properties;
  List alarms;
} Event;

typedef struct ical {
	float version;
	char prodID[1000];
	List events;
  List properties; 
} Calendar;

int getEventNum(Calendar *cal);
char *printEvent(Calendar *cal, int index);
ICalErrorCode calOpenFail(char *filename);
Calendar *calOpen(char *filename);
Calendar *customCal(char args[2][1000]);
Calendar *customEvent(Calendar *cal, char args[4][1000]);
Calendar *customAlarm(Calendar *cal, char args[2][1000], int index);
ICalErrorCode createCalendar(char* fileName, Calendar** obj);
void deleteCalendar(Calendar* obj);
char *printCalendar(const Calendar* obj); 
char *printError(ICalErrorCode err);
void getEventArr(const Calendar* obj, int index, char **strings);
ICalErrorCode writeCalendar(char* fileName, const Calendar* obj);
ICalErrorCode validateCalendar(const Calendar* obj);

#endif	

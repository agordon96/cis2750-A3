#include "CalendarParser.h"
#include <ctype.h>

char* printFuncEvent(void *toBePrinted);
char* printFuncAlarm(void *toBePrinted);
char* printFuncProp(void *toBePrinted);
int compareFuncEvent(const void *first, const void *second);
int compareFuncAlarm(const void *first, const void *second);
int compareFuncProp(const void *first, const void *second);
void deleteFuncEvent(void *toBeDeleted);
void deleteFuncAlarm(void *toBeDeleted);
void deleteFuncProp(void *toBeDeleted);
bool compareProp(const void *first, const void *second);
bool findDiffProp(const void *first, const void *second);
void clearSpaces(char *toClear);
ICalErrorCode badError(Calendar *cal, FILE *file, Calendar **obj, ICalErrorCode err);

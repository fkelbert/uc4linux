#ifndef UC_TYPES_H
#define UC_TYPES_H

#define EVENT_NAME_READ "Read"
#define EVENT_NAME_OPEN "Open"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct s_param {
	char *key;
	char *val;
};
typedef struct s_param param;

struct s_event {
	char *name;
	param **params;
	int cntParams;
	bool isActual;
	int iterParams;
};
typedef struct s_event event;


#define EVENT_STD_PARAMS_CNT 2
extern char *eventStdParams[];

param *createParam(char *key, char *val);
void destroyParam(param *p);
event *createEvent(char *name, int cntParams);
event *createEventWithStdParams(char *name, int cntParams);
void destroyEvent(event *e);
bool addParam(event *ev, param *p);

#endif


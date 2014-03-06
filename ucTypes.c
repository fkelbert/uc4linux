#include "ucTypes.h"

char *eventStdParams[] =
				{ "PEP", "Linux",
					"host", "machineA" };

param *createParam(char *key, char *val) {
	param *p = malloc(sizeof(param));
	p->key = strdup(key);
	p->val = strdup(val);
	return p;
}

void destroyParam(param *p) {
	free(p->key);
	free(p->val);
	free(p);
}

event *createEvent(char *name, int cntParams) {
	event *e = malloc(sizeof(event));
	e->name = strdup(name);
	e->isActual = true;
	e->cntParams = cntParams;
	e->iterParams = 0;
	e->params = malloc(cntParams * sizeof(param*));
	return e;
}

event *createEventWithStdParams(char *name, int cntParams) {
	event *e = createEvent(name, cntParams + EVENT_STD_PARAMS_CNT);

	int i;
	for (i = 0; i < EVENT_STD_PARAMS_CNT; i++) {
		addParam(e, createParam(eventStdParams[i*2], eventStdParams[i*2+1]));
	}

	return e;
}

bool addParam(event *ev, param *p) {
	if (ev->iterParams < ev->cntParams) {
		ev->params[ev->iterParams] = p;
		ev->iterParams++;
		return true;
	}
	return false;
}

void destroyEvent(event *e) {
	free(e->name);
	int i;
	for (i = 0; i < e->cntParams; i++) {
		destroyParam(e->params[i]);
	}
	free(e->params);
}


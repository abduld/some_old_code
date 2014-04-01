
#include	<wb.h>

static inline wbBool wbLogEntry_hasNext(wbLogEntry_t elem) {
	return wbLogEntry_getNext(elem) != NULL;
}

wbLogEntry_t wbLogEntry_new() {
	wbLogEntry_t elem;

	elem = wbNew(struct st_wbLogEntry_t);

	wbLogEntry_setMessage(elem, NULL);
	wbLogEntry_setTime(elem, uv_hrtime());
#ifndef NDEBUG
	wbLogEntry_setLevel(elem, wbLogLevel_TRACE);
#else
	wbLogEntry_setLevel(elem, wbLogLevel_off);
#endif
	wbLogEntry_setNext(elem, NULL);

	wbLogEntry_setLine(elem, -1);
	wbLogEntry_setFile(elem, NULL);
	wbLogEntry_setFunction(elem, NULL);

	return elem;
}

wbLogEntry_t wbLogEntry_initialize(wbLogLevel_t level, const char * msg, const char * file, const char * fun, int line) {
	wbLogEntry_t elem;

	elem = wbLogEntry_new();

	
	wbLogEntry_setLevel(elem, level);

	if (msg == NULL) {
		wbLogEntry_setMessage(elem, wbString_duplicate(""));
	} else {
		wbLogEntry_setMessage(elem, wbString_duplicate(msg));
	}
	
	wbLogEntry_setLine(elem, line);
	wbLogEntry_setFile(elem, file);
	wbLogEntry_setFunction(elem, fun);

	return elem;
}

void wbLogEntry_delete(wbLogEntry_t elem) {
	if (elem != NULL) {
		if (wbLogEntry_getMessage(elem) != NULL) {
			wbFree(wbLogEntry_getMessage(elem));
		}
		wbDelete(elem);
	}
	return ;
}

static inline const char * getLevelName(wbLogLevel_t level) {
	switch (level) {
		case wbLogLevel_unknown:
			return "Unknown";
		case wbLogLevel_OFF:
			return "Off";
		case wbLogLevel_FATAL:
			return "Fatal";
		case wbLogLevel_ERROR:
			return "Error";
		case wbLogLevel_WARN:
			return "Warn";
		case wbLogLevel_INFO:
			return "Info";
		case wbLogLevel_DEBUG:
			return "Debug";
		case wbLogLevel_TRACE:
			return "Trace";
	}
	return NULL;
}

json_t * wbLogEntry_toJSON(wbLogEntry_t elem) {
	if (elem != NULL) {
		json_t * res = json_object();
		json_t * type = json_string("LogEntry");
		json_t * level = json_string(getLevelName(wbLogEntry_getLevel(elem)));
		json_t * msg = json_string(wbLogEntry_getMessage(elem));
		json_t * fun = json_string(wbLogEntry_getFunction(elem));
		json_t * file = json_string(wbLogEntry_getFile(elem));
		json_t * line = json_integer(wbLogEntry_getLine(elem));
		json_t * time = json_integer(wbLogEntry_getTime(elem));

		json_object_set(res, "Type", type);
		json_object_set(res, "Level", level);
		json_object_set(res, "Message", msg);
		json_object_set(res, "File", file);
		json_object_set(res, "Function", fun);
		json_object_set(res, "Line", line);
		json_object_set(res, "Time", time);

		return res;
	}
	return NULL;
}

wbLogger_t wbLogger_new() {
	wbLogger_t logger;

	logger = wbNew(struct st_wbLogger_t);

	wbLogger_setLength(logger, 0);
	wbLogger_setHead(logger, NULL);
#ifndef NDEBUG
	wbLogger_setLevel(logger, wbLogLevel_TRACE);
#else
	wbLogger_setLevel(logger, wbLogLevel_OFF);
#endif
	
	return logger;
}

void wbLogger_delete(wbLogger_t logger) {
	if (logger != NULL) {
		wbLogger_clear(logger);
		wbDelete(logger);
	}
	return ;
}

void wbLogger_clear(wbLogger_t logger) {
	if (logger != NULL) {
		wbLogEntry_t tmp;
		wbLogEntry_t iter;

		iter = wbLogger_getHead(logger);
		while (iter != NULL) {
			tmp = wbLogEntry_getNext(iter);
			wbLogEntry_delete(iter);
			iter = tmp;
		}
		
		wbLogger_setLength(logger, 0);
		wbLogger_setHead(logger, NULL);
	}
}

void wbLogger_append(wbLogger_t logger, wbLogLevel_t level, const char * msg, const char * file, const char * fun, int line) {
	wbLogEntry_t elem;
	
	if (wbLogger_getLevel(logger) < level) {
		return ;
	}

	elem = wbLogEntry_initialize(level, msg, file, fun, line);

	if (wbLogger_getHead(logger) == NULL) {
		wbLogger_setHead(logger, elem);
	} else {
		wbLogEntry_t prev = wbLogger_getHead(logger);

		while (wbLogEntry_hasNext(prev)) {
			prev = wbLogEntry_getNext(prev);
		}
		wbLogEntry_setNext(prev, elem);
	}

	wbLogger_incrementLength(logger);

	return ;
}

json_t * wbLogger_toJSON(wbLogger_t logger) {
	if (logger != NULL) {
		wbLogEntry_t iter;

		json_t * res = json_object();
		json_t * type = json_string("Logger");
		json_t * elems = json_array();

		for (iter = wbLogger_getHead(logger); iter != NULL; iter = wbLogEntry_getNext(iter)) {
			json_array_append(elems, wbLogEntry_toJSON(iter));
		}

		json_object_set(res, "Type", type);
		json_object_set(res, "Elements", elems);

		return res;
	}
	return NULL;
}


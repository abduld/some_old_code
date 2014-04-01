
#ifndef __LOGGER_H__
#define __LOGGER_H__

typedef enum {
    wbLogLevel_unknown = -1,
	wbLogLevel_OFF = 0,
	wbLogLevel_FATAL,
	wbLogLevel_ERROR,
	wbLogLevel_WARN,
	wbLogLevel_INFO,
	wbLogLevel_DEBUG,
	wbLogLevel_TRACE
} wbLogLevel_t;

struct st_wbLogEntry_t {
	int line;
	char * msg;
	uint64_t time;
	const char * fun;
	const char * file;
	wbLogLevel_t level;
	struct st_wbLogEntry_t * next;
};

#define wbLogEntry_getMessage(elem)			((elem)->msg)
#define wbLogEntry_getTime(elem)			((elem)->time)
#define wbLogEntry_getLevel(elem)			((elem)->level)
#define wbLogEntry_getNext(elem)			((elem)->next)
#define wbLogEntry_getLine(elem)			((elem)->line)
#define wbLogEntry_getFunction(elem)		((elem)->fun)
#define wbLogEntry_getFile(elem)			((elem)->file)

#define wbLogEntry_setMessage(elem, val)	(wbLogEntry_getMessage(elem) = val)
#define wbLogEntry_setTime(elem, val)		(wbLogEntry_getTime(elem) = val)
#define wbLogEntry_setLevel(elem, val)		(wbLogEntry_getLevel(elem) = val)
#define wbLogEntry_setNext(elem, val)		(wbLogEntry_getNext(elem) = val)
#define wbLogEntry_setLine(elem, val)		(wbLogEntry_getLine(elem) = val)
#define wbLogEntry_setFunction(elem, val)	(wbLogEntry_getFunction(elem) = val)
#define wbLogEntry_setFile(elem, val)		(wbLogEntry_getFile(elem) = val)

extern wbLogEntry_t wbLogEntry_new();
extern wbLogEntry_t wbLogEntry_initialize(wbLogLevel_t level, const char * msg, const char * file, const char * fun, int line);
extern void wbLogEntry_delete(wbLogEntry_t elem);

extern json_t * wbLogEntry_toJSON(wbLogEntry_t elem);

struct st_wbLogger_t {
	int length;
	wbLogEntry_t head;
    wbLogLevel_t level;
};

#define wbLogger_getLength(log)				((log)->length)
#define wbLogger_getHead(log)				((log)->head)
#define wbLogger_getLevel(log)    			((log)->level)

#define wbLogger_setLength(log, val)		(wbLogger_getLength(log) = val)
#define wbLogger_setHead(log, val)			(wbLogger_getHead(log) = val)
#define wbLogger_setLevel(log, val)    		(wbLogger_getLevel(log) = val)

#define wbLogger_incrementLength(log)		(wbLogger_getLength(log)++)
#define wbLogger_decrementLength(log)		(wbLogger_getLength(log)--)

#define wbLog(logger, level, msg)			wbLogger_append(logger, wbLogLevel_##level, msg, wbFile, wbFunction, wbLine)

extern wbLogger_t wbLogger_new();
extern void wbLogger_delete(wbLogger_t logger);

extern void wbLogger_clear(wbLogger_t logger);
extern void wbLogger_append(wbLogger_t logger, wbLogLevel_t level, const char * msg, const char * file, const char * fun, int line);

extern json_t * wbLogger_toJSON(wbLogger_t logger);

#endif /* __LOGGER_H__ */

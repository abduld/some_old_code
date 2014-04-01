

#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum en_wbErrorCode_t {
#define wbError_define(err, ...)		wbError_##err,
#include "error_inc.h"
#undef wbError_define
	wbSuccess = wbError_success
} wbErrorCode_t;

#define wbSuccessQ(err)					(wbError_getCode(err) == wbSuccess || wbError_getCode(err) == wbError_uv)
#define wbFailQ(err)					(!(wbSuccessQ(err)))


struct st_wbError_t {
	int line;
	wbErrorCode_t code;
	const char * msg;
	const char * file;
	const char * function;
	wbContext_t context;
	uv_loop_t * loop;
};

#define wbError_getMessage(err)			((err)->msg)
#define wbError_getCode(err)			((err)->code)
#define wbError_getLine(err)			((err)->line)
#define wbError_getFile(err)			((err)->file)
#define wbError_getFunction(err)		((err)->function)
#define wbError_getContext(err)			((err)->context)
#define wbError_getLoop(err)			((err)->loop)

#define wbError_setMessage(err, val)	(wbError_getMessage(err) = val)	
#define wbError_setCode(err, val)		(wbError_getCode(err) = val)
#define wbError_setLine(err, val)		(wbError_getLine(err) = val)
#define wbError_setFile(err, val)		(wbError_getFile(err) = val)
#define wbError_setFunction(err, val)	(wbError_getFunction(err) = val)
#define wbError_setContext(err, val)	(wbError_getContext(err) = val)
#define wbError_setLoop(err, val)		(wbError_getLoop(err) = val)

#define wbError(err, code)				wbError_update(err, wbError_##code, wbFile, wbFunction, wbLine)
#define wbError_uv(err)					wbError(err, uv)

extern wbError_t wbError_new();
extern void wbError_delete(wbError_t err);

extern void wbError_update(wbError_t err, wbErrorCode_t code, const char * file, const char * fun, int line);

extern char * wbError_toLog(wbError_t err);
extern json_t * wbError_toJSON(wbError_t err);

#endif /* __ERROR_H__ */

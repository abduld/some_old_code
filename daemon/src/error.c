
#include	<wb.h>

static const char * wbErrorMessages[] = {
#define wbError_define(err, msg, ...)		msg,
#include "error_inc.h"
#undef wbError_define
};


wbError_t wbError_new() {
	wbError_t err;

	err = wbNew(struct st_wbError_t);
	wbError_setContext(err, NULL);
	wbError_setCode(err, wbSuccess);
	wbError_setMessage(err, wbErrorMessages[wbSuccess]);
	wbError_setFunction(err, NULL);
	wbError_setFile(err, NULL);
	wbError_setLine(err, -1);
	wbError_setLoop(err, NULL);

	return err;
}

void wbError_delete(wbError_t err) {
	if (err != NULL) {
		wbDelete(err);
	}
	return ;
}

void wbError_update(wbError_t err, wbErrorCode_t code, const char * file, const char * fun, int line) {
	wbContext_t ctx;
	const char * msg;

	if (err == NULL) {
		return ;
	}

	ctx = wbError_getContext(err);

	wbContext_lockMutex(ctx);

	if (code == wbError_uv) {
		uv_loop_t * loop;
		uv_err_t uvErr;
		
		if (wbError_getLoop(err) != NULL) {
			loop = wbError_getLoop(err);
		} else if (ctx != NULL && wbContext_getLoop(ctx)) {
			loop = wbContext_getLoop(ctx);
		} else {
			return ;
		}

		uvErr = uv_last_error(loop);
		if (uvErr.code == UV_OK) {
			wbError_setCode(err, wbSuccess);
			msg = wbErrorMessages[wbSuccess];
		} else {
			msg = uv_strerror(uvErr);
		}
	} else {
		wbAssert(code > 0);
		wbAssert(code < sizeof(wbErrorMessages));
		msg = wbErrorMessages[code];
	}

	wbError_setCode(err, code);
	wbError_setMessage(err, msg);
	wbError_setFile(err, file);
	wbError_setFunction(err, fun);
	wbError_setLine(err, line);
	
	wbContext_unlockMutex(ctx);

	return ;
}

const char * wbError_message(wbError_t err) {
	const char * res = NULL;
	if (err != NULL) {
		if (wbError_getMessage(err) == NULL) {
			wbAssert(wbError_getCode(err) > 0);
			wbAssert(wbError_getCode(err) < sizeof(wbErrorMessages));
			res = wbErrorMessages[wbError_getCode(err)];
		} else {
			res = wbError_getMessage(err);
		}
	}
	return res;
}

#define LINE_STR_LENGTH ((CHAR_BIT * sizeof(int) - 1) / 3 + 2)

char * wbError_toLog(wbError_t err) {
	if (err != NULL && wbFailQ(err)) {
		char lineStr[LINE_STR_LENGTH];

		wbStringBuffer_t buf = wbStringBuffer_new();

		wbStringBuffer_append(buf, "ERROR: (");
		wbStringBuffer_append(buf, wbError_getMessage(err));
		wbStringBuffer_append(buf, ") in ");
		wbStringBuffer_append(buf, wbError_getFile(err));
		wbStringBuffer_append(buf, "::");
		wbStringBuffer_append(buf, wbError_getFunction(err));
		wbStringBuffer_append(buf, " on line ");

		snprintf(lineStr, LINE_STR_LENGTH, "%d", wbError_getLine(err));
		wbStringBuffer_append(buf, lineStr);

		wbStringBuffer_append(buf, ".");
	}
	return NULL;
}

json_t * wbError_toJSON(wbError_t err) {
	if (err == NULL) {
		return NULL;
	} else {
		json_t * res = json_object();
		json_t * type = json_string("Error");
		json_t * code = json_integer(wbError_getCode(err));
		json_t * msg = json_string(wbError_getMessage(err));
		json_t * file = json_string(wbError_getFile(err));
		json_t * fun = json_string(wbError_getFunction(err));
		json_t * line = json_integer(wbError_getLine(err));

		json_object_set(res, "Type", type);
		json_object_set(res, "Code", code);
		json_object_set(res, "Message", msg);
		json_object_set(res, "File", file);
		json_object_set(res, "Function", fun);
		json_object_set(res, "Line", line);

		return res;
	}
}


#include	<wb.h>

typedef enum {
	wbQuery_unknown,
	wbQuery_context,
	wbQuery_log,
	wbQuery_compute,
} wbQuery_t;

typedef struct st_wbComputeData_t {
    wbRequest_t req;
	const char * code;
} * wbComputeData_t;

#define wbComputeData_getRequest(cd)		((cd)->req)
#define wbComputeData_getCode(cd)			((cd)->code)

#define wbComputeData_setRequest(cd, val)	(wbComputeData_getRequest(cd) = val)
#define wbComputeData_setCode(cd, val)		(wbComputeData_getCode(cd) = val)

static wbQuery_t getQueryType(const json_t * json) {
	json_t * jtype;
	const char * type;
	
	jtype = json_object_get(json, "Type");

	wbAssert(json_is_string(jtype));
	if (!json_is_string(jtype)) {
		return wbQuery_unknown;
	}

	type = json_string_value(jtype);
	/*
	if (wbString_sameQ(type, "Log")) {
		return wbQuery_log;
	}
	*/

	return wbQuery_compute;
}

static void wbRespose_afterWrite(uv_write_t * writeRequest, int status) {
	wbRequest_t req = (wbRequest_t) writeRequest->data;
	wbContext_t ctx = wbRequest_getContext(req);
	
	wbLog(wbRequest_getLogger(req), TRACE, "After writing.");
	wbContext_mutexed(ctx, {
		wbLog(wbContext_getLogger(ctx), TRACE, "After writing.");
	});

	uv_close((uv_handle_t*) writeRequest->handle, wbRequest_onClose);

	wbRequest_delete(req);
}

static inline uv_buf_t makeResponse(const char * text) {
	size_t len;
	char * data;
	uv_buf_t buf;
	char responseBase[256];
	
	sprintf(responseBase, "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %d\r\n\r\n", strlen(text));

	len = strlen(responseBase) + strlen(text) + 1;

	data = wbNewArray(char, len);
	strcpy(data, responseBase);
	strcat(data, text);

	buf.base = data;
	buf.len = (ULONG) len - 1;

	return buf;
}

static void writeResponse(wbRequest_t req, const char * text) {
	int res;

	wbRequest_setResponseBuffer(req, makeResponse(text));

	res = uv_write(
				&wbRequest_getWriteRequest(req),
				(uv_stream_t*)& wbRequest_getHandle(req),
				&wbRequest_getResponseBuffer(req),
				1,
				wbRespose_afterWrite);
	if (res) {
		wbContext_t ctx = wbRequest_getContext(req);
		wbLog(wbRequest_getLogger(req), ERROR, "Unable to write message");
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), ERROR, "Unable to write message");
		});
	}
	return ;
}

static char * wbResponse_createTemporaryDirectory(wbContext_t ctx) {
	uv_fs_t req;

	char * dir = wbDirectory_temporary();

	uv_fs_rmdir(wbContext_getLoop(ctx), &req, dir, NULL);
	uv_fs_mkdir(wbContext_getLoop(ctx), &req, dir, 0755, NULL);

	uv_fs_req_cleanup(&req);

	return dir;
}

static char * getLog(wbLogger_t logger) {
	json_t * jlog = wbLogger_toJSON(logger);
	char * log = json_dumps(jlog, JSON_INDENT(1));

	json_delete(jlog);

	return log;
}

static void writeLog(wbRequest_t req, wbLogger_t logger) {
	char * log = getLog(logger);
	wbAssert(log != NULL);

	wbLogger_clear(logger);

	if (log == NULL) {
		return ;
	}
	writeResponse(req, log);
	wbFree(log);

	return ;
}

static void writeProgramResponse(wbRequest_t req, wbRunner_t programRunner, wbRunner_t compileRunner) {
	json_t * type;
	json_t * obj;
	json_t * log;
	char * cout;
	char * pout;
	json_t * compileOutput;
	json_t * programOutput;
	char * res;
	
	log = wbLogger_toJSON(wbRequest_getLogger(req));
	wbAssert(log != NULL);

	wbLogger_clear(wbRequest_getLogger(req));

	if (log == NULL) {
		return ;
	}

	cout = wbStringBuffer_toCString(wbRunner_getStdout(compileRunner));
	pout = wbStringBuffer_toCString(wbRunner_getStdout(programRunner));

	type = json_string("ProgramOutput");
	compileOutput = json_string(cout);
	programOutput = json_string(pout);

	obj = json_object();

	
	json_object_set(obj, "Type", type);
	json_object_set(obj, "CompileOutput", compileOutput);
	json_object_set(obj, "ProgramOutput", programOutput);
	json_object_set(obj, "Log", log);

	res = json_dumps(obj, JSON_INDENT(1));

	writeResponse(req, res);
		
	json_delete(obj);

	wbFree(cout);
	wbFree(pout);
    wbFree(res);

	return ;
}

static void wbResponse_compute(wbRequest_t req, const char * code) {
	char * tmpDir;
	char * exe;
	wbRunner_t compileRunner = NULL;
	wbRunner_t programRunner = NULL;
	wbContext_t ctx;

	ctx = wbRequest_getContext(req);

	tmpDir = wbResponse_createTemporaryDirectory(ctx);

	compileRunner = wbRunner_new();
	wbRunner_setContext(compileRunner, ctx);
	wbRunner_setLogger(compileRunner, wbRequest_getLogger(req));
	wbRunner_setError(compileRunner, wbRequest_getError(req));

	wbCompiler_compile(compileRunner, tmpDir, code, &exe);

	while (wbRunner_getIsDone(compileRunner) == wbFalse) {
		continue ;
	}
	
	wbError_setLoop(wbRequest_getError(req), wbContext_getLoop(ctx));

	if (wbFailQ(wbRequest_getError(req))) {
		goto cleanup;
	}

	if (exe == NULL || !wbFile_existsQ(exe)) { /* error occurred in the compilation, so we return the stderr */
		wbLog(wbRequest_getLogger(req), ERROR, "Was not able to compile the program");
		writeLog(req, wbRequest_getLogger(req));
	} else {
		programRunner = wbRunner_new();
		
		wbRunner_setContext(programRunner, ctx);
		wbRunner_setProgram(programRunner, exe);
		
		wbRunner_setLogger(compileRunner, wbRequest_getLogger(req));
		wbRunner_setError(compileRunner, wbRequest_getError(req));

		wbRunner_run(programRunner, tmpDir);

		
		if (wbFailQ(wbRequest_getError(req))) {
			goto cleanup;
		}


		while (wbRunner_getIsDone(programRunner) == wbFalse) {
			continue ;
		}
		
		wbError_setLoop(wbRequest_getError(req), wbContext_getLoop(ctx));

		writeProgramResponse(req, programRunner, compileRunner);
	}

cleanup:
	wbError_setLoop(wbRequest_getError(req), wbContext_getLoop(ctx));
	if (exe != NULL) {
		wbDelete(exe);
	}
	wbDirectory_delete(tmpDir);
	wbDelete(tmpDir);
	wbRunner_delete(programRunner);
	wbRunner_delete(compileRunner);

	return ;
}


static void wbResponse_computeInThread(void * data) {
	wbRequest_t req;
	const char * prog;
	wbComputeData_t cd;

	cd = (wbComputeData_t) data;

	req = wbComputeData_getRequest(cd);
	prog = wbComputeData_getCode(cd);
	
	wbResponse_compute(req, prog);

	wbDelete(cd);
}

void wbRespose_new(wbRequest_t req) {
	wbContext_t ctx;
	wbQuery_t query;
	json_t * json;

	if (req == NULL) {
		return ;
	}

	ctx = wbRequest_getContext(req);
	wbAssert(ctx != NULL);
	
	wbError_setLoop(wbRequest_getError(req), wbContext_getLoop(ctx));

	wbAssert(wbRequest_getBody(req) != NULL);
	wbAssert(wbRequest_getJSONBody(req) != NULL);

	if (wbRequest_getBody(req) == NULL ||
	    wbRequest_getJSONBody(req) == NULL) {
		return ;
	}

	json = wbRequest_getJSONBody(req);

	if (!json_is_object(json)) {
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), ERROR, "Got something other than a JSON object");
		});
		return ;
	}

	query = getQueryType(json);

	switch (query) {
		case wbQuery_unknown:
			writeResponse(req, "");
			return ;
		case wbQuery_log:
			writeLog(req, wbContext_getLogger(ctx));
			return ;
		case wbQuery_compute:
			{
				json_t * jprog;
				const char * prog;
				wbComputeData_t cd;
				uv_thread_t thread;

				/*
				if ((jprog = json_object_get(json, "Program")) == NULL) {
					wbLog(wbRequest_getLogger(req), TRACE, "JSON did not contain Program field.");
					writeLog(req, wbRequest_getLogger(req));
					return ;
				}
				if (!json_is_string(jprog)) {
					return ;
				}
				if ((prog = json_string_value(jprog)) == NULL) {
					return ;
				}
				*/

				prog = "int main() { return 1; }";

				cd = wbNew(struct st_wbComputeData_t);

				wbComputeData_setRequest(cd, req);
				wbComputeData_setCode(cd, prog);
				
				if (uv_thread_create(&thread, wbResponse_computeInThread, cd)) {
					wbContext_mutexed(ctx, {
						wbLog(wbContext_getLogger(ctx), FATAL, "Thread Create Error.");
						wbError_uv(wbContext_getError(ctx));
					});
				}


			}
	}
	return ;
}



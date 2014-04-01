
#include	<wb.h>

wbRunner_t wbRunner_new() {
	wbRunner_t run;

	run = wbNew(struct st_wbRunner_t);

	wbRunner_setContext(run, NULL);
	wbRunner_setResponse(run, NULL);
	wbRunner_setRequest(run, NULL);
	wbRunner_setProgram(run, NULL);
	wbRunner_setArgc(run, 0);
	wbRunner_setArgs(run, NULL);
	wbRunner_setStdout(run, wbStringBuffer_new());
	wbRunner_setWorker(run, wbNew(uv_work_t));
	wbRunner_getWorker(run)->data = run;
	
	wbRunner_setLoop(run, NULL);
	wbRunner_setIsDone(run, wbFalse);
	wbRunner_setOutputFile(run, NULL);

	return run;
}

void wbRunner_delete(wbRunner_t run) {
	if (run != NULL) {
		wbStringBuffer_delete(wbRunner_getStdout(run));
		if (wbRunner_getWorker(run) != NULL) {
			wbDelete(wbRunner_getWorker(run));
		}
		if (wbRunner_getOutputFile(run) != NULL) {
			wbDelete(wbRunner_getOutputFile(run));
		}
		wbDelete(run);
	}
	return ;
}


static void wbRunner_onClose(uv_handle_t* handle) {
	wbRunner_t runner = (wbRunner_t) handle->data;
	wbContext_t ctx = wbRunner_getContext(runner);

	wbLog(wbRunner_getLogger(runner), TRACE, "Closing runner pipe.");
	wbError_uv(wbRunner_getError(runner));

	return ;
}

static void wbRunner_onExit(uv_process_t* proc, int status, int signal) {
	wbRunner_t runner = (wbRunner_t) proc->data;
	wbContext_t ctx = wbRunner_getContext(runner);

	wbLog(wbRunner_getLogger(runner), TRACE, "Exiting the process.");
	wbError_uv(wbRunner_getError(runner));

	uv_close((uv_handle_t*) proc, wbRunner_onClose);
	uv_unref((uv_handle_t *) wbRunner_getLoop(runner));
	return ;
}

static uv_buf_t wbRunner_onReadAlloc(uv_handle_t* handle, size_t size) {
	uv_buf_t buf;
	buf.base = wbNewArray(char, size);
	buf.len = (ULONG) size;
	return buf;
}

static void wbRunner_onRead(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
	wbRunner_t runner = (wbRunner_t) stream->data;
	wbContext_t ctx = wbRunner_getContext(runner);

	if (nread > 0) {
		char * tmp = wbNewArray(char, nread + 2);

		memcpy(tmp, buf.base, nread);
		tmp[nread] = '\n';
		tmp[nread + 1] = '\0';
		

		wbStringBuffer_append(wbRunner_getStdout(runner), tmp);

		wbLog(wbRunner_getLogger(runner), TRACE, "Got output from process");
		wbLog(wbRunner_getLogger(runner), TRACE, tmp);

		wbDelete(tmp);
	} else {

		wbLog(wbRunner_getLogger(runner), TRACE, "Failed to read from process.");
		wbError_uv(wbRunner_getError(runner));

		uv_close((uv_handle_t*) stream, wbRunner_onClose);
	}

	wbFree(buf.base);
	return ;
}

void wbRunner_run(wbRunner_t runner) {
	uv_pipe_t out;
	char * prog;
	char * args[2];
	uv_loop_t * loop;
	uv_process_t proc;
	uv_process_options_t opts = {0};
	uv_stdio_container_t stdio[2];
	wbContext_t ctx;
	char dir[1024];

	ctx = wbRunner_getContext(runner);

	prog = wbRunner_getProgram(runner);

	loop = uv_loop_new();

	wbRunner_setLoop(runner, loop);

	args[0] = prog;
	args[1] = NULL;

	uv_cwd(dir, 1024);

	opts.file = prog;
	opts.args = args;
	opts.cwd = dir;
	opts.exit_cb = wbRunner_onExit;
	opts.stdio = stdio;
	opts.stdio[0].flags = UV_IGNORE;
	opts.stdio[1].flags = (uv_stdio_flags) (UV_CREATE_PIPE | UV_WRITABLE_PIPE);
	opts.stdio[1].data.stream = (uv_stream_t*) &out;
	opts.stdio_count = 2;

	uv_pipe_init(loop, &out, 0);

	out.data = runner;
	proc.data = runner;
    
    wbLog(wbRunner_getLogger(runner), TRACE, "Running");
    wbLog(wbRunner_getLogger(runner), TRACE, prog);
	
	wbError_setLoop(wbRunner_getError(runner), loop);

	if (uv_spawn(loop, &proc, opts)) {
		wbLog(wbRunner_getLogger(runner), TRACE, "Failed to spawn process.");
		wbError_uv(wbRunner_getError(runner));
		return ;
	}

	if (uv_read_start((uv_stream_t*) &out, wbRunner_onReadAlloc, wbRunner_onRead)) {
		wbLog(wbRunner_getLogger(runner), TRACE, "Failed to start reading from process.");
		wbError_uv(wbRunner_getError(runner));
		return ;
	}

	if (uv_run(loop)) {
		wbLog(wbRunner_getLogger(runner), TRACE, "Failed to run the process loop.");
		wbError_uv(wbRunner_getError(runner));
		return ;
	}
}



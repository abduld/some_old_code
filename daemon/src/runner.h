
#ifndef __RUNNER_H__
#define __RUNNER_H__

struct st_wbRunner_t {
	wbBool isDone;
	wbContext_t ctx;
	wbResponse_t resp;
	wbRequest_t req;
	char * prog;
	int argc;
	char ** args;
	uv_work_t * worker;
	wbStringBuffer_t out;
	uv_loop_t * loop;
	char * outputFile;
	wbLogger_t logger;
	wbError_t err;
};

#define wbRunner_getIsDone(run)					((run)->isDone)
#define wbRunner_getContext(run)				((run)->ctx)
#define wbRunner_getResponse(run)				((run)->resp)
#define wbRunner_getRequest(run)				((run)->req)
#define wbRunner_getProgram(run)				((run)->prog)
#define wbRunner_getWorker(run)					((run)->worker)
#define wbRunner_getArgc(run)					((run)->argc)
#define wbRunner_getArgs(run)					((run)->args)
#define wbRunner_getStdout(run)					((run)->out)
#define wbRunner_getLoop(run)					((run)->loop)
#define wbRunner_getOutputFile(run)				((run)->outputFile)
#define wbRunner_getLogger(run)					((run)->logger)
#define wbRunner_getError(run)					((run)->err)


#define wbRunner_setIsDone(run, val)			(wbRunner_getIsDone(run) = val)
#define wbRunner_setContext(run, val)			(wbRunner_getContext(run) = val)
#define wbRunner_setResponse(run, val)			(wbRunner_getResponse(run) = val)
#define wbRunner_setRequest(run, val)			(wbRunner_getRequest(run) = val)
#define wbRunner_setProgram(run, val)			(wbRunner_getProgram(run) = val)
#define wbRunner_setWorker(run, val)			(wbRunner_getWorker(run) = val)
#define wbRunner_setArgc(run, val)				(wbRunner_getArgc(run) = val)
#define wbRunner_setArgs(run, val)				(wbRunner_getArgs(run) = val)
#define wbRunner_setStdout(run, val)			(wbRunner_getStdout(run) = val)
#define wbRunner_setLoop(run, val)				(wbRunner_getLoop(run) = val)
#define wbRunner_setOutputFile(run, val)		(wbRunner_getOutputFile(run) = val)
#define wbRunner_setLogger(run, val)			(wbRunner_getLogger(run) = val)
#define wbRunner_setError(run, val)				(wbRunner_getError(run) = val)

extern wbRunner_t wbRunner_new();
extern void wbRunner_delete(wbRunner_t run);

extern void wbRunner_run(wbRunner_t runner, const char * dir);

extern void wbRunner_onStart(uv_work_t *req);
extern void wbRunner_onStop(uv_work_t *req);

#endif /* __RUNNER_H__ */

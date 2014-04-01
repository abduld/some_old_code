

#include	<wb.h>

const char * basePaths[] = {
#ifdef _WIN32
	"C:\\CUDA\\bin"
#else /* _WIN32 */
#endif /* _WIN32 */
};

const char * baseEnvironments[] = {
#ifdef _WIN32
	"CUDA_PATH_V5_0",
	"CUDA_PATH_V4_2",
	"CUDA_PATH_V4_2",
	"CUDA_PATH_V4_1",
	"CUDA_PATH_V4_0",
	"CUDA_BIN_PATH",
#else /* _WIN32 */
	"CUDA_BIN"
#endif /* _WIN32 */
};

typedef struct st_wbCompile_t {
	char * nvccPath;
	char * prog;
	char * compileCommand;
	char * output;
} * wbCompile_t;

wbCompile_t wbCompile_new(const char * prog) {
	return NULL;
}


static inline char * findNVCC(wbContext_t ctx, const char * dir) {
	char * file;
	char * tmp;
	const char compilerName[] = 
#ifdef _WIN32
		"nvcc.exe";
#else
		"nvcc";
#endif

	file = wbFile_nameJoin(dir, compilerName);

	if (wbFile_existsQ(file)) {
		return file;
	}
		
	wbDelete(file);

	tmp = wbFile_nameJoin(dir, "bin");
	file = wbFile_nameJoin(dir, compilerName);

		
	if (wbFile_existsQ(file)) {
		return file;
	}

	wbDelete(file);
	wbDelete(tmp);

	return NULL;
}

static char * nvccPath = NULL;

char * wbCompile_find(wbContext_t ctx) {
	int ii;
	char * res = NULL;
	int pathCount = sizeof(basePaths) / sizeof(basePaths[0]);
	int envCount = sizeof(baseEnvironments) / sizeof(baseEnvironments[0]);
	const char compilerName[] = 
#ifdef _WIN32
		"nvcc.exe";
#else
		"nvcc";
#endif

	wbContext_lockMutex(ctx);

	if (nvccPath != NULL) {
		res = wbString_duplicate(nvccPath);
		goto cleanup;
	}

	for (ii = 0; ii < pathCount; ii++) {
		if ((res = findNVCC(ctx, basePaths[ii])) != NULL) {
			nvccPath = wbString_duplicate(res);
			goto cleanup;
		}
	}
	
	for (ii = 0; ii < envCount; ii++) {
		char * dir = wbEnvironment_get(baseEnvironments[ii]);
		if (dir != NULL && (res = findNVCC(ctx, dir)) != NULL) {
			nvccPath = wbString_duplicate(res);
			goto cleanup;
		}
	}

cleanup:
	wbContext_unlockMutex(ctx);

	return res;
}

#ifdef _WIN32
#define buildFileName		"build.bat"
#else
#define buildFileName		"build.sh"
#endif

static void writeProgramToFile(wbRunner_t run, const char * dir, const char * prog) {
	char * path;
	wbFile_t file;
	
	wbContext_t ctx = wbRunner_getContext(run);

	path = wbFile_nameJoin(dir, "program.cu");

	file = wbFile_new(path, O_RDWR | O_CREAT | O_TRUNC);
	wbFile_setContext(file, ctx);

	wbFile_write(file, prog);
	wbFile_write(file, "\r\n\r\n");

	wbFile_close(file);

	wbFile_delete(file);
}

#ifdef _WIN32
#define EXECUTABLE_NAME			"program.exe"
#else
#define EXECUTABLE_NAME			"program"
#endif

static char * writeBuildScriptToFile(wbRunner_t run, const char * dir) {
	char * path;
	char * nvccExe;
	char * out;
	wbFile_t file;
	char * nvccExeQuoted;


	wbContext_t ctx = wbRunner_getContext(run);
	
	nvccExe = wbCompile_find(ctx);
	nvccExeQuoted = wbString_quote(nvccExe);
	
	path = wbFile_nameJoin(dir, buildFileName);
	
	file = wbFile_new(path, O_RDWR | O_CREAT | O_TRUNC);

	wbFile_setContext(file, ctx);

#ifdef _WIN32
	wbFile_write(file, "echo Setting Visual Studio Environment.\r\n");
	wbFile_write(file, "call \"c:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\vcvarsall.bat\" amd64\r\n");
#endif
    
	wbFile_write(file, "echo Calling NVCC Compiler.\r\n");
	wbFile_write(file, nvccExeQuoted);
	wbFile_write(file, " program.cu -o ");
	out = wbFile_nameJoin(dir, "EXECUTABLE_NAME");
	wbFile_write(file, out);
	wbFile_write(file, " 2>&1");
	wbFile_write(file, "\r\n");
    
	wbFile_close(file);

	wbRunner_setOutputFile(run, out);

	wbDelete(nvccExe);
	wbDelete(nvccExeQuoted);
	wbFile_delete(file);

	return path;
}


static void onStart(uv_work_t *req) {
    wbRunner_t runner;

	runner = (wbRunner_t) req->data;
    
    uv_chdir(wbRunner_getWorkingDirectory(runner));
    
	wbRunner_run(runner);

	return ;
}

static void onStop(uv_work_t *req) {
	wbRunner_t runner = (wbRunner_t) req->data;
    
    if (wbRunner_getLoop(runner) != NULL) {
		uv_loop_delete(wbRunner_getLoop(runner));
	}
    
    wbLog(wbRunner_getLogger(runner), TRACE, "Finished running compile command.");
    
    if (wbRunner_getOutputFile(runner) && wbFile_exists(wbRunner_getOutputFile(runner))) {
        wbRunner_t proc = wbRunner_new();
        wbProcess_run(proc, wbRunner_getOutputFile(runner));
    }
    
	return ;
}

void invoke(wbRunner_t runner, const char * dir) {
	wbContext_t ctx;

	ctx = wbRunner_getContext(runner);

    wbRunner_setWorkingDirectory(runner, dir);
    
	uv_queue_work(wbContext_getLoop(ctx), wbRunner_getWorker(runner), onStart, onStop);

	return ;
}

wbRunner_t wbProcess_run(wbRunner_t runner, const char * dir, const char * prog, char ** exe) {
	char * toRun;
	wbContext_t ct = wbRunner_getContext(runner);

	toRun = writeBuildScriptToFile(runner, dir);
	writeProgramToFile(runner, dir, prog);

	wbRunner_setProgram(runner, toRun);

	invoke(runner, dir);

	if (exe != NULL) {
		char * tmp = wbFile_nameJoin(dir, EXECUTABLE_NAME);
		if (wbFile_existsQ(tmp)) {
			*exe = tmp;
		} else {
			wbDelete(tmp);
			*exe = NULL;
		}
	}

	return runner;
}



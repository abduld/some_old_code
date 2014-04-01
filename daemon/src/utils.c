

#include	<wb.h>

wbFile_t wbFile_new(const char * path, int flags) {
	wbFile_t file;
	
	if (path == NULL) {
		return NULL;
	}

	file = wbNew(struct st_wbFile_t);

	wbFile_setPath(file, wbString_duplicate(path));
	wbFile_setContext(file, NULL);
	wbFile_setFileHandle(file, -1);
	wbFile_setFlags(file, flags);
	wbFile_setOpenedQ(file, wbFalse);

	wbFile_setOffset(file, 0);

	return file;
}

void wbFile_delete(wbFile_t file) {
	if (file != NULL) {
		if (wbFile_getPath(file)) {
			wbDelete(wbFile_getPath(file));
		}
		if (wbFile_getFileHandle(file) != -1) {
			wbFile_close(file);
		}
		wbDelete(file);
	}
	return ;
}

void wbFile_open(wbFile_t file) {
	wbContext_t ctx;
	uv_loop_t * loop;
	
	if (wbFile_getOpenedQ(file) == wbTrue) {
		return ;
	}
	
	ctx = wbFile_getContext(file);
	wbAssert(ctx != NULL);

	loop = wbContext_getLoop(ctx);

	if (wbFile_existsQ(wbFile_getPath(file))) {
		uv_fs_t req;
		uv_fs_unlink(loop, &req, wbFile_getPath(file), NULL);
		uv_fs_req_cleanup(&req);
	}

	uv_fs_open(loop, &wbFile_getOpenRequest(file), wbFile_getPath(file), wbFile_getFlags(file),  S_IREAD | S_IWRITE, NULL);
	
	uv_fs_req_cleanup(&wbFile_getOpenRequest(file));
	
	wbFile_setFileHandle(file, (uv_file) wbFile_getOpenRequest(file).result);
	wbFile_setOpenedQ(file, wbTrue);

	wbContext_mutexed(ctx, {
		wbLog(wbContext_getLogger(ctx), ERROR, "Opening file.");
	});

	return ;
}

void wbFile_close(wbFile_t file) {
	wbContext_t ctx;
	uv_loop_t * loop;
	uv_fs_t closeRequest;

	if (wbFile_getOpenedQ(file) == wbFalse) {
		return ;
	}
	
	ctx = wbFile_getContext(file);
	wbAssert(ctx != NULL);

	loop = wbContext_getLoop(ctx);

	uv_fs_close(loop, &closeRequest, wbFile_getFileHandle(file), NULL);
	
	wbContext_mutexed(ctx, {
		wbLog(wbContext_getLogger(ctx), ERROR, "Closing file.");
	});

	wbFile_setFileHandle(file, -1);

	return ;
}

void wbFile_write(wbFile_t file, const char * text) {
	wbContext_t ctx;
	uv_loop_t * loop;
	size_t textLength;
	
	if (wbFile_getOpenedQ(file) == wbFalse) {
		wbFile_open(file);
	}

	if (!wbFile_existsQ(wbFile_getPath(file))) {
		return ;
	}

	ctx = wbFile_getContext(file);
	wbAssert(ctx != NULL);

	loop = wbContext_getLoop(ctx);
	
	uv_fs_req_cleanup(&wbFile_getOpenRequest(file));

	textLength = strlen(text);

	uv_fs_write(uv_default_loop(), &wbFile_getWriteRequest(file), wbFile_getFileHandle(file), (char *) text, textLength, wbFile_getOffset(file), NULL);
	
	wbContext_mutexed(ctx, {
		wbLog(wbContext_getLogger(ctx), ERROR, "Writing to file.");
	});
	
	wbFile_getOffset(file) += textLength;

	uv_fs_req_cleanup(&wbFile_getWriteRequest(file));

	return ;
}

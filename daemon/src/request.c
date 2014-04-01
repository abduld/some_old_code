

#include	<wb.h>


wbRequest_t wbRequest_new() {
	wbRequest_t req;

	req = wbNew(struct st_wbRequest_t);

	wbRequest_getParser(req).data = req;
	wbRequest_getHandle(req).data = req;

	wbRequest_setBody(req, NULL);
	wbRequest_setJSONBody(req, NULL);

	wbRequest_setError(req, wbError_new());

	http_parser_init(&wbRequest_getParser(req), HTTP_REQUEST);

	wbRequest_getWriteRequest(req).data = req;
	wbRequest_getResponseBuffer(req).base = NULL;

	wbRequest_setLogger(req, wbLogger_new());

	return req;
}

void wbRequest_delete(wbRequest_t req) {
	if (req != NULL) {
		if (wbRequest_getBody(req) != NULL) {
			wbFree(wbRequest_getBody(req));
		}
		if (wbRequest_getJSONBody(req) != NULL) {
			json_delete(wbRequest_getJSONBody(req));
		}
		if (wbRequest_getResponseBuffer(req).base != NULL) {
			wbDelete(wbRequest_getResponseBuffer(req).base);
		}
		wbError_delete(wbRequest_getError(req));
		wbDelete(req);
	}
	return ;
}

void wbRequest_onClose(uv_handle_t * handle) {
	wbRequest_t req = (wbRequest_t) handle->data;
	wbContext_t ctx = wbRequest_getContext(req);

	
	wbLog(wbRequest_getLogger(req), TRACE, "Closing Connection.");
	wbContext_mutexed(ctx, {
		wbLog(wbContext_getLogger(ctx), TRACE, "Closing Connection.");
	});

	wbRequest_delete(req);

	return ;
}

uv_buf_t wbRequest_onAlloc(uv_handle_t * client, size_t size) {
	uv_buf_t buf;
	buf.base = (char *) wbMalloc(size);
	buf.len = (ULONG) size;
	return buf;
}

void wbRequest_onRead(uv_stream_t* tcp, ssize_t nread, uv_buf_t buf) {
	wbRequest_t req;
	wbContext_t ctx;

	req = (wbRequest_t) tcp->data;
	ctx = wbRequest_getContext(req);

	if (nread >= 0) {
		ssize_t parsed;
		parsed = http_parser_execute(&wbRequest_getParser(req),
									 &wbContext_getParserSettings(ctx),
									 buf.base,
									 nread);
		if (parsed < nread) {
			wbLog(wbRequest_getLogger(req), FATAL, "HTTP Parse Error.");
			wbContext_mutexed(ctx, {
				wbLog(wbContext_getLogger(ctx), FATAL, "HTTP Parse Error.");
				uv_close((uv_handle_t*) &wbRequest_getHandle(req), wbRequest_onClose);
			});
		}
	} else {
		uv_err_t err = uv_last_error(wbContext_getLoop(ctx));
		if (err.code != UV_EOF) {
			wbLog(wbRequest_getLogger(req), FATAL, "HTTP Read Error.");
			wbContext_mutexed(ctx, {
				wbLog(wbContext_getLogger(ctx), FATAL, "HTTP Read Error.");
				wbError_uv(wbContext_getError(ctx));
			});
		}
	}

	wbFree(buf.base);

	return ;
}

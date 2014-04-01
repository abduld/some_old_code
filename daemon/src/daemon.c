

#include	<wb.h>


wbDaemon_t wbDaemon_new() {
	wbDaemon_t dm;

	dm = wbNew(struct st_wbDaemon_t);

	wbDaemon_setAddress(dm, "0.0.0.0");
	wbDaemon_setPort(dm, 8000);

	wbDaemon_setContext(dm, wbContext_new());
	wbContext_setDaemon(wbDaemon_getContext(dm), dm);

	wbDaemon_setLogger(dm, wbLogger_new());

	return dm;
}

void wbDaemon_delete(wbDaemon_t dm) {
	if (dm != NULL) {
		wbContext_delete(wbDaemon_getContext(dm));
		wbDelete(dm);
	}
	return ;
}

wbDaemon_t wbDaemon_initialize(const char * addr, int port) {
	struct sockaddr_in ip4Address;
	wbDaemon_t dm;
	wbContext_t ctx;
	
	dm = wbDaemon_new();
	ctx = wbDaemon_getContext(dm);

	wbContext_setLoop(ctx, uv_loop_new());
	if (uv_tcp_init(wbContext_getLoop(ctx), &wbContext_getServer(ctx))) {
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), TRACE, "Failed initializing tcp");
			wbError_uv(wbContext_getError(ctx));
		});
		goto err;
	}
	
	wbDaemon_setAddress(dm, addr);
	wbDaemon_setPort(dm, port);

	ip4Address = uv_ip4_addr(addr, port);

	if (uv_tcp_bind(&wbContext_getServer(ctx), ip4Address)) {
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), TRACE, "Failed binding tcp connection");
			wbError_uv(wbContext_getError(ctx));
		});
		goto err;
	}

	return dm;

err:
	wbDaemon_delete(dm);
	return NULL;
}

static void wbDaemon_onConnection(uv_stream_t * server, int status) {
	wbContext_t ctx;
	wbRequest_t request;

	if (server == NULL) {
		return ;
	}
	
	ctx = (wbContext_t) server->data;

	if (status) {
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), TRACE, "Failed getting connection");
			wbError_uv(wbContext_getError(ctx));
		});
		return ;
	}
	
	wbContext_mutexed(ctx,
		wbLog(wbContext_getLogger(ctx), TRACE, "Got connection")
	);

	request = wbRequest_new();
	wbError_setLoop(wbRequest_getError(request), wbContext_getLoop(ctx));

	wbContext_mutexed(ctx, {
		wbLog(wbContext_getLogger(ctx), TRACE, "Created new request handle");
		wbRequest_setId(request, wbContext_getNumberOfRequests(ctx));
		wbRequest_setContext(request, ctx);
		wbContext_incrementNumberOfRequests(ctx);
	});


	uv_tcp_init(wbContext_getLoop(ctx), &wbRequest_getHandle(request));

	if (uv_accept(server, (uv_stream_t*) &wbRequest_getHandle(request))) {
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), TRACE, "Failed accepting connection");
			wbError_uv(wbContext_getError(ctx));
		});
		return ;
	}

	uv_read_start((uv_stream_t*)& wbRequest_getHandle(request), wbRequest_onAlloc, wbRequest_onRead);

	return ;
}

void wbDaemon_listen(wbDaemon_t dm) {
	wbContext_t ctx = wbDaemon_getContext(dm);

	wbContext_getServer(ctx).data = ctx;
	if (uv_listen((uv_stream_t*) &wbContext_getServer(ctx), 128, wbDaemon_onConnection)) {
		wbError_uv(wbContext_getError(ctx));
	}
	return ;
}

void wbDaemon_run(wbDaemon_t dm) {
	wbContext_t ctx = wbDaemon_getContext(dm);
	uv_loop_t * loop = wbContext_getLoop(ctx);
	
	
	if (uv_run(loop)) {
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), TRACE, "Failed to run the process loop.");
			wbError_uv(wbContext_getError(ctx));
		});
		return ;
	}
}

json_t * wbDaemon_toJSON(wbDaemon_t dm) {
	if (dm != NULL) {
		json_t * res = json_object();
		json_t * type = json_string("Daemon");
		json_t * addr = json_string(wbDaemon_getAddress(dm));
		json_t * port = json_integer(wbDaemon_getPort(dm));
		json_t * ctx = wbContext_toJSON(wbDaemon_getContext(dm));

		json_object_set(res, "Type", type);
		json_object_set(res, "Address", addr);
		json_object_set(res, "Port", port);
		json_object_set(res, "Context", ctx);

		return res;
	}
	return NULL;
}

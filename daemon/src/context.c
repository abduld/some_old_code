
#include	<wb.h>


wbContext_t wbContext_new() {
	wbContext_t ctx;

	ctx = wbNew(struct st_wbContext_t);
	
	wbContext_setId(ctx, -1);
	wbContext_setNumberOfRequests(ctx, 0);

	wbContext_setLoop(ctx, NULL);

	wbContext_setError(ctx, wbError_new());
	wbError_setContext(wbContext_getError(ctx), ctx);

	uv_mutex_init(&wbContext_getMutex(ctx));

	wbContext_setLogger(ctx, wbLogger_new());

	wbContext_getParserSettings(ctx).on_body = wbHTTP_onBody;
	wbContext_getParserSettings(ctx).on_url = wbHTTP_onURL;
	wbContext_getParserSettings(ctx).on_header_field = wbHTTP_onHeaderField;
	wbContext_getParserSettings(ctx).on_header_value = wbHTTP_onHeaderValue;
	wbContext_getParserSettings(ctx).on_headers_complete = wbHTTP_onHeadersComplete;
	wbContext_getParserSettings(ctx).on_message_begin = wbHTTP_onMessageBegin;
	wbContext_getParserSettings(ctx).on_message_complete = wbHTTP_onMessageComplete;

	return ctx;
}

void wbContext_delete(wbContext_t ctx) {
	if (ctx != NULL) {
		wbError_delete(wbContext_getError(ctx));
		uv_mutex_destroy(&wbContext_getMutex(ctx));
		wbLogger_delete(wbContext_getLogger(ctx));
		wbDelete(ctx);
	}
	return ;
}

json_t * wbContext_toJSON(wbContext_t ctx) {
	if (ctx != NULL) {
		json_t * res = json_object();
		json_t * type = json_string("Context");
		json_t * id = json_integer(wbContext_getId(ctx));
		json_t * numRequests = json_integer(wbContext_getNumberOfRequests(ctx));
		json_t * err = wbError_toJSON(wbContext_getError(ctx));

		json_object_set(res, "Type", type);
		json_object_set(res, "Id", id);
		json_object_set(res, "NumberOfRequests", numRequests);
		json_object_set(res, "Error", err);

		json_object_set(res, "Log", wbLogger_toJSON(wbContext_getLogger(ctx)));

		return res;
	}
	return NULL;
}


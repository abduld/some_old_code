
#include	<wb.h>

int wbHTTP_onMessageBegin(http_parser * parser) {
	return HPE_OK;
}

int wbHTTP_onURL(http_parser * parser, const char *at, size_t length) {
	return HPE_OK;
}

int wbHTTP_onHeaderField(http_parser * parser, const char *at, size_t length) {
	return HPE_OK;
}

int wbHTTP_onHeaderValue(http_parser * parser, const char *at, size_t length) {
	return HPE_OK;
}

int wbHTTP_onHeadersComplete(http_parser * parser) {
	return HPE_OK;
}

static int parseJSON(wbRequest_t req) {
	json_error_t jerr;
	const char * str;
	json_t * json;
	
	str = wbRequest_getBody(req);
	if (str == NULL) {
		return 0;
	}

	json = json_loads(str, JSON_DECODE_ANY | JSON_REJECT_DUPLICATES, &jerr);
	if (wbRequest_getJSONBody(req) != NULL) {
		json_delete(wbRequest_getJSONBody(req));
	}
	wbRequest_setJSONBody(req, json);
	if (json == NULL) {
		wbContext_t ctx = wbRequest_getContext(req);
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), ERROR, "Was not able to parse JSON data");
			wbLog(wbContext_getLogger(ctx), ERROR, wbRequest_getBody(req));
		});
		return 0;
	}
	return 1;
}

int wbHTTP_onBody(http_parser * parser, const char *at, size_t length) {
	wbRequest_t req;

	if (at != NULL && length != 0) {
		wbContext_t ctx;

		req = (wbRequest_t) parser->data;

		if (wbRequest_getBody(req) != NULL) {
			wbFree(wbRequest_getBody(req));
		}

		wbRequest_setBody(req, wbNewArray(char, length + 1));
		strncpy(wbRequest_getBody(req), at, length);
		wbRequest_getBody(req)[length] = '\0';
		
		ctx = wbRequest_getContext(req);
		wbContext_mutexed(ctx, {
			wbLog(wbContext_getLogger(ctx), INFO, wbRequest_getBody(req));
		});

		if (parseJSON(req)) {
			wbRespose_new(req);
		}
	}
	return HPE_OK;
}

int wbHTTP_onMessageComplete(http_parser * parser) {
	return HPE_OK;
}




#ifndef __REQUEST_H__
#define __REQUEST_H__

struct st_wbRequest_t {
	int id;
	wbContext_t ctx;
	uv_tcp_t handle;
	http_parser parser;
	char * body;
	json_t * jsonBody;
	uv_write_t writeRequest;
	uv_buf_t responseBuffer;
	wbLogger_t logger;
	wbError_t err;
};

#define wbRequest_getId(req)					((req)->id)
#define wbRequest_getContext(req)				((req)->ctx)
#define wbRequest_getHandle(req)				((req)->handle)
#define wbRequest_getParser(req)				((req)->parser)
#define wbRequest_getBody(req)					((req)->body)
#define wbRequest_getJSONBody(req)				((req)->jsonBody)
#define wbRequest_getWriteRequest(req)			((req)->writeRequest)
#define wbRequest_getResponseBuffer(req)		((req)->responseBuffer)
#define wbRequest_getLogger(req)				((req)->logger)
#define wbRequest_getError(req)					((req)->err)

#define wbRequest_setId(req, val)				(wbRequest_getId(req) = val)
#define wbRequest_setContext(req, val)			(wbRequest_getContext(req) = val)
#define wbRequest_setHandle(req, val)			(wbRequest_getHandle(req) = val)
#define wbRequest_setParser(req, val)			(wbRequest_getParser(req) = val)
#define wbRequest_setBody(req, val)				(wbRequest_getBody(req) = val)
#define wbRequest_setJSONBody(req, val)			(wbRequest_getJSONBody(req) = val)
#define wbRequest_setWriteRequest(req, val)		(wbRequest_getWriteRequest(req) = val)
#define wbRequest_setResponseBuffer(req, val)	(wbRequest_getResponseBuffer(req) = val)
#define wbRequest_setLogger(req, val)			(wbRequest_getLogger(req) = val)
#define wbRequest_setError(req, val)			(wbRequest_getError(req) = val)

extern wbRequest_t wbRequest_new();
extern void wbRequest_delete(wbRequest_t req);


extern void wbRequest_onClose(uv_handle_t * handle);
extern uv_buf_t wbRequest_onAlloc(uv_handle_t* client, size_t suggested_size);
extern void wbRequest_onRead(uv_stream_t* tcp, ssize_t nread, uv_buf_t buf);

#endif /* __REQUEST_H__ */





#ifndef __HTTP_H__
#define __HTTP_H__

extern int wbHTTP_onMessageBegin(http_parser * parser);
extern int wbHTTP_onURL(http_parser * parser, const char *at, size_t length);
extern int wbHTTP_onHeaderField(http_parser * parser, const char *at, size_t length);
extern int wbHTTP_onHeaderValue(http_parser * parser, const char *at, size_t length);
extern int wbHTTP_onHeadersComplete(http_parser * parser);
extern int wbHTTP_onBody(http_parser * parser, const char *at, size_t length);
extern int wbHTTP_onMessageComplete(http_parser * parser);

#endif /* __HTTP_H__ */

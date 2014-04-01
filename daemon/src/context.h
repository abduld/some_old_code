
#ifndef __CONTEXT_H__
#define __CONTEXT_H__


struct st_wbContext_t {
	int id;
	int numRequests;
	uv_loop_t * loop;
	uv_tcp_t server;
	uv_mutex_t mutex;
	http_parser_settings parserSettings;
	wbError_t err;
	wbDaemon_t dm;
	wbLogger_t logger;
};

#define wbContext_getId(ctx)					((ctx)->id)
#define wbContext_getNumberOfRequests(ctx)		((ctx)->numRequests)
#define wbContext_getLoop(ctx)					((ctx)->loop)
#define wbContext_getServer(ctx)				((ctx)->server)
#define wbContext_getMutex(ctx)					((ctx)->mutex)
#define wbContext_getError(ctx)					((ctx)->err)
#define wbContext_getParserSettings(ctx)		((ctx)->parserSettings)
#define wbContext_getDaemon(ctx)				((ctx)->dm)
#define wbContext_getLogger(ctx)				((ctx)->logger)

#define wbContext_setId(ctx, val)				(wbContext_getId(ctx) = val)
#define wbContext_setNumberOfRequests(ctx, val)	(wbContext_getNumberOfRequests(ctx) = val)
#define wbContext_setLoop(ctx, val)				(wbContext_getLoop(ctx) = val)
#define wbContext_setServer(ctx, val)			(wbContext_getServer(ctx) = val)
#define wbContext_setMutex(ctx, val)			(wbContext_getMutex(ctx) = val)
#define wbContext_setError(ctx, val)			(wbContext_getError(ctx) = val)
#define wbContext_setParserSettings(ctx, val)	(wbContext_getParserSettings(ctx) = val)
#define wbContext_setDaemon(ctx, val)			(wbContext_getDaemon(ctx) = val)
#define wbContext_setLogger(ctx, val)			(wbContext_getLogger(ctx) = val)


#define wbContext_incrementNumberOfRequests(ctx) (wbContext_getNumberOfRequests(ctx)++)
#define wbContext_decrementNumberOfRequests(ctx) (wbContext_getNumberOfRequests(ctx)--)

extern wbContext_t wbContext_new();
extern void wbContext_delete(wbContext_t ctx);

extern json_t * wbContext_toJSON(wbContext_t ctx);

static inline void wbContext_lockMutex(wbContext_t ctx) {
	if (ctx != NULL) {
		uv_mutex_lock(&wbContext_getMutex(ctx));
	}
	return ;
}

static inline void wbContext_unlockMutex(wbContext_t ctx) {
	if (ctx != NULL) {
		uv_mutex_unlock(&wbContext_getMutex(ctx));
	}
	return ;
}

#define wbContext_mutexed(ctx, ...)				do {								\
													wbContext_lockMutex(ctx);		\
													{								\
														__VA_ARGS__;				\
													}								\
													wbContext_unlockMutex(ctx);		\
												} while (0)

#endif /* __CONTEXT_H__ */

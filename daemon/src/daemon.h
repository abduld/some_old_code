
#ifndef __DAEMON_H__
#define __DAEMON_H__



typedef struct st_wbDaemon_t {
	int port;
	const char * addr;
	wbContext_t ctx;
	wbLogger_t logger;
} * wbDaemon_t;

#define wbDaemon_getPort(dm)			((dm)->port)
#define wbDaemon_getAddress(dm)			((dm)->addr)
#define wbDaemon_getContext(dm)			((dm)->ctx)
#define wbDaemon_getLogger(dm)			((dm)->logger)

#define wbDaemon_setPort(dm, val)		(wbDaemon_getPort(dm) = val)
#define wbDaemon_setAddress(dm, val)	(wbDaemon_getAddress(dm) = val)
#define wbDaemon_setContext(dm, val)	(wbDaemon_getContext(dm) = val)
#define wbDaemon_setLogger(dm, val)		(wbDaemon_getLogger(dm) = val)

extern void wbDaemon_delete(wbDaemon_t dm);
extern wbDaemon_t wbDaemon_initialize(const char * addr, int port);
extern void wbDaemon_listen(wbDaemon_t dm);
extern void wbDaemon_run(wbDaemon_t dm);
extern json_t * wbDaemon_toJSON(wbDaemon_t dm);

#endif /* __DAEMON_H__ */

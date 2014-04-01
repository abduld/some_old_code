
#include	<wb.h>

int main(void) {
	wbDaemon_t dm;
	
	dm = wbDaemon_initialize("127.0.0.1", 8000);
	wbDaemon_listen(dm);
	wbDaemon_run(dm);
	return 0;
}


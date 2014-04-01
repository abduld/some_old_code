
#include	<wb.h>

struct st_wbStringBuffer_t {
	int id;
	char * buf;
	size_t curLen;
	size_t maxLen;
};

#define wbStringBuffer_initialLength					64
#define wbStringBuffer_expandMultiplier					2

#define wbStringBuffer_getId(sb)						((sb)->id)
#define wbStringBuffer_getCurrentLength(sb)				((sb)->curLen)
#define wbStringBuffer_getBuffer(sb)					((sb)->buf)
#define wbStringBuffer_getBufferChar(sb, ii)			(wbStringBuffer_getBuffer(sb)[ii])
#define wbStringBuffer_getMaxLength(sb)					((sb)->maxLen)

#define wbStringBuffer_setCurrentLength(sb, val)		(wbStringBuffer_getCurrentLength(sb) = val)
#define wbStringBuffer_setBuffer(sb, val)				(wbStringBuffer_getBuffer(sb) = val)
#define wbStringBuffer_setBufferChar(sb, ii, val)		(wbStringBuffer_getBufferChar(sb, ii) = val)
#define wbStringBuffer_setMaxLength(sb, val)			(wbStringBuffer_getMaxLength(sb) = val)


wbStringBuffer_t wbStringBuffer_new(void) {
	char * buf;
	size_t size = wbStringBuffer_initialLength;
	wbStringBuffer_t sb = wbNew(struct st_wbStringBuffer_t);
	wbStringBuffer_setId(sb, -1);
	wbStringBuffer_setCurrentLength(sb, 0);
	wbStringBuffer_setMaxLength(sb, size);
	buf = wbNewArray(char, wbStringBuffer_initialLength);
	assert(buf != NULL);
	wbStringBuffer_setBuffer(sb, buf);
	memset(buf, '\0', size);
	return sb;
}

wbStringBuffer_t wbStringBuffer_initialize(size_t sz) {
	wbStringBuffer_t sb = wbNew(struct st_wbStringBuffer_t);
	wbStringBuffer_setId(sb, -1);
	wbStringBuffer_setCurrentLength(sb, 0);
	wbStringBuffer_setMaxLength(sb, sz);
	wbStringBuffer_setBuffer(sb, wbReallocArray(char, NULL, sz));
	return sb;
}

void wbStringBuffer_delete(wbStringBuffer_t sb) {
	if (sb) {
		if (wbStringBuffer_getBuffer(sb)) {
			wbDelete(wbStringBuffer_getBuffer(sb));
		}
		wbDelete(sb);
	}
	return ;
}

static wbBool wbStringBuffer_expand(wbStringBuffer_t sb, size_t len) {
	if (sb) {
		if (wbStringBuffer_getCurrentLength(sb) + len < wbStringBuffer_getMaxLength(sb) - 1) {
			return wbTrue;
		} else {
			size_t oldSize = wbStringBuffer_getMaxLength(sb);
			size_t newSize = wbStringBuffer_expandMultiplier * (oldSize + 1);
			char * newMemory;

			while (newSize < len) {
				newSize *= wbStringBuffer_expandMultiplier;
			}
			
			newMemory = wbReallocArray(char, wbStringBuffer_getBuffer(sb), newSize);
			if (newMemory == NULL) {
				/* out of memory */
				return wbFalse;
			} else {
				size_t ii;
				char * buf = newMemory + oldSize;
				wbStringBuffer_setBuffer(sb, newMemory);
				for (ii = oldSize; ii < newSize; ii++) {
					*buf++ = '\0';
				}
				wbStringBuffer_setMaxLength(sb, newSize);
				return wbTrue;
			}
		}
	} else {
		return wbFalse;
	}
}

void wbStringBuffer_setId(wbStringBuffer_t sb, int id) {
	if (sb) {
		wbStringBuffer_getId(sb) = id;
	}
	return ;
}

int wbStringBuffer_id(wbStringBuffer_t sb) {
	if (sb) {
		return wbStringBuffer_getId(sb);
	} else {
		return -1;
	}
}

size_t wbStringBuffer_length(wbStringBuffer_t sb) {
	if (sb) {
		return wbStringBuffer_getCurrentLength(sb);
	} else {
		return 0;
	}
}

void wbStringBuffer_append(wbStringBuffer_t sb, const char * msg) {
	if (sb) {
		size_t slen = strlen(msg);

		if (wbStringBuffer_expand(sb, slen)) {
			size_t ii = 0;
			char * buf = wbStringBuffer_getBuffer(sb) + wbStringBuffer_getCurrentLength(sb);

			while (ii < slen) {
				*buf++ = *msg++;
				ii++;
			}
			*buf = '\0';

			wbStringBuffer_setCurrentLength(sb, wbStringBuffer_getCurrentLength(sb) + slen);

			assert(wbStringBuffer_getCurrentLength(sb) < wbStringBuffer_getMaxLength(sb));
		}
	}
	return ;
}

void wbStringBuffer_join(wbStringBuffer_t sb, wbStringBuffer_t from) {
	if (sb) {
		size_t slen = wbStringBuffer_length(from);

		if (wbStringBuffer_expand(sb, slen)) {
			size_t ii = 0;
			char * msg = wbStringBuffer_getBuffer(from);
			char * buf = wbStringBuffer_getBuffer(sb) + wbStringBuffer_getCurrentLength(sb);

			while (ii < slen) {
				*buf++ = *msg++;
				ii++;
			}
			*buf = '\0';

			wbStringBuffer_setCurrentLength(sb, wbStringBuffer_getCurrentLength(sb) + slen);

			assert(wbStringBuffer_getCurrentLength(sb) < wbStringBuffer_getMaxLength(sb));
		}
	}
	return ;
}

wbStringBuffer_t wbStringBuffer_reverse(wbStringBuffer_t from) {
	if (from) {
		wbStringBuffer_t to = wbStringBuffer_initialize(wbStringBuffer_getCurrentLength(from) + 1);
		char * toBuf, * fromBuf;
		size_t ii = 0, len;

		len = wbStringBuffer_getCurrentLength(from);

		toBuf = wbStringBuffer_getBuffer(to) + len - 1;
		fromBuf = wbStringBuffer_getBuffer(from);

		while (ii < len) {
			*toBuf-- = *fromBuf++;
			ii--;
		}
		toBuf[len] = '\0';
	}
	return NULL;
}

wbStringBuffer_t wbStringBuffer_take(wbStringBuffer_t from, size_t start, size_t end) {
	size_t ii;
	char * toBuf, * fromBuf;
	wbStringBuffer_t to = NULL;	

	if (from == NULL ||
		start >= wbStringBuffer_getCurrentLength(from) ||
		end >= wbStringBuffer_getCurrentLength(from)) {
		return NULL;
	}

	if (end < 0) {
		end += wbStringBuffer_getCurrentLength(from);
	}

	if (start < 0) {
		start += wbStringBuffer_getCurrentLength(from);
	}

	if (end == start) {
		to = wbStringBuffer_initialize(1);
		wbStringBuffer_setBufferChar(to, 0, '\0');
	} else if (end > start) {
		to = wbStringBuffer_initialize(end - start + 1);
		toBuf = wbStringBuffer_getBuffer(to);
		fromBuf = wbStringBuffer_getBuffer(from) + start;
		for (ii = start; ii < end; ii++) {
			*toBuf++ = *fromBuf++;
		}
	}

	return to;
}

char * wbStringBuffer_toCString(wbStringBuffer_t sb) {
	if (sb) {
		return wbStringBuffer_getBuffer(sb);
	} else {
		return NULL;
	}
}





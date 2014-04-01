


#ifndef __STRING_BUFFER_H__
#define __STRING_BUFFER_H__

wbStringBuffer_t wbStringBuffer_new(void);
wbStringBuffer_t wbStringBuffer_initialize(size_t sz);
void wbStringBuffer_delete(wbStringBuffer_t sb);
static wbBool wbStringBuffer_expand(wbStringBuffer_t sb, size_t len);
void wbStringBuffer_setId(wbStringBuffer_t sb, int id);
int wbStringBuffer_id(wbStringBuffer_t sb);
size_t wbStringBuffer_length(wbStringBuffer_t sb);
void wbStringBuffer_append(wbStringBuffer_t sb, const char * msg);
void wbStringBuffer_join(wbStringBuffer_t sb, wbStringBuffer_t from);
wbStringBuffer_t wbStringBuffer_reverse(wbStringBuffer_t from);
wbStringBuffer_t wbStringBuffer_take(wbStringBuffer_t from, size_t start, size_t end);
char * wbStringBuffer_toCString(wbStringBuffer_t sb);

#endif /* __STRING_BUFFER_H__ */


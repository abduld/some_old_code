
#ifndef __UTILS_H__
#define __UTILS_H__


#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>
#include	<string.h>

#include	<fcntl.h>
#include	<sys/stat.h>

#ifdef _MSC_VER
#include	<windows.h>
#include	<direct.h>
#include	<io.h>
#define inline						__inline
#define __func__					__FUNCTION__
#define snprintf					_snprintf
#define	mkdir(pth, mode)			_mkdir(pth)
#define lstat(pth, sb)				stat(pth, sb)
#define S_ISDIR(m)					(((m) & S_IFDIR)==S_IFDIR)
#define popen						_popen
#define pclose						_pclose
#endif /* _MSC_VER */

#define wbPrint(...)				printf(__VA_ARGS__)

#define wbAssert(cond)				assert(cond)
#define wbAssertMessage(msg, cond)	do {						\
										if (!(cond)) {			\
											wbPrint(msg);		\
											wbAssert(cond);		\
										}						\
									} while (0)


#define wbNew(type)					((type *) wbMalloc(sizeof(type)))
#define wbNewArray(type, len)		((type * ) wbMalloc((len) * sizeof(type)))
#define wbMalloc(sz)				xMalloc(sz)
#define wbDelete(var)				wbFree(var)
#define wbFree(var)					xFree(var)
#define wbRealloc(var, newSize)		xRealloc(var, newSize)
#define wbReallocArray(t, m, n)		((t*) xRealloc(m, n*sizeof(t)))


#define wbTrue						1
#define wbFalse						0

#define wbFile						__FILE__
#define wbFunction					__func__
#define wbLine						__LINE__

typedef int wbBool;

static inline void * xMalloc(size_t sz) {
	void * mem = NULL;
	if (sz != 0) {
		mem = malloc(sz);
	}
	return mem;
}

static inline void xFree(void * mem) {
	if (mem != NULL) {
		free(mem);
	}
	return ;
}

static inline void * xRealloc(void * mem, size_t sz) {
	if (mem == NULL) {
		return NULL;
	} else if (sz == 0) {
		xFree(mem);
		return NULL;
	} else {
		void * res = realloc(mem, sz);
		wbAssert(res != NULL);
		return res;
	}
}

static inline char * wbString_duplicate(const char * str) {
#ifdef _WIN32
	return _strdup(str);
#else
	return strdup(str)
#endif
}

static inline wbBool wbDirectory_existsQ(const char * dir) {
	struct stat sb;
	int err = lstat(dir, &sb);
	if (err < 0) {
		return wbFalse;
	} else {
		return S_ISDIR(sb.st_mode);
	}
}

static inline char * wbEnvironment_get(const char * name) {
	char * buffer;
#ifdef _WIN32
	_dupenv_s(&buffer, NULL, name);
#else /* _WIN32 */
	buffer = getenv(name);
#endif /* _WIN32 */
	return buffer;
}

static inline char * wbDirectory_getTemporary() {
	char * buffer;

#define _wbDirectory_getTemporary(name)				\
	buffer = wbEnvironment_get(name);				\
	if (buffer != NULL) {							\
		return buffer;								\
	}
	
	_wbDirectory_getTemporary("TEMP");
	_wbDirectory_getTemporary("TMPDIR");
	_wbDirectory_getTemporary("TMP");

#undef _wbDirectory_getTemporary

	return NULL;
}

#ifdef _WIN32
#define wbDirectory_slash							"\\"
#else
#define wbDirectory_slash							"/"
#endif

static inline char * wbFile_nameJoin(const char * dir, const char * file) {
	size_t len = strlen(dir) + 1 + strlen(file) + 1;
	char * res = wbNewArray(char, len);
	memset(res, '\0', len);
	strcpy(res, dir);
	strcat(res, wbDirectory_slash);
	strcat(res, file);
	return res;
}

static inline wbBool wbFile_existsQ(const char * path) {
	FILE * file;

	if (file = fopen(path, "r")) {
		fclose(file);
		return wbTrue;
	}
	return wbFalse;
}

struct st_wbFile_t {
	wbContext_t ctx;
	char * path;
	int flags;
	uv_file fh;
	uv_fs_t openRequest;
	uv_fs_t readRequest;
	uv_fs_t writeRequest;
	wbBool openedQ;
	size_t offset;
};

#define wbFile_getContext(fs)				((fs)->ctx)
#define wbFile_getPath(fs)					((fs)->path)
#define wbFile_getFlags(fs)					((fs)->flags)
#define wbFile_getFileHandle(fs)			((fs)->fh)
#define wbFile_getOpenRequest(fs)			((fs)->openRequest)
#define wbFile_getReadRequest(fs)			((fs)->readRequest)
#define wbFile_getWriteRequest(fs)			((fs)->writeRequest)
#define wbFile_getOpenedQ(fs)				((fs)->openedQ)
#define wbFile_getOffset(fs)				((fs)->offset)

#define wbFile_setContext(fs, val)			(wbFile_getContext(fs) = val)
#define wbFile_setPath(fs, val)				(wbFile_getPath(fs) = val)
#define wbFile_setFlags(fs, val)			(wbFile_getFlags(fs) = val)
#define wbFile_setFileHandle(fs, val)		(wbFile_getFileHandle(fs) = val)
#define wbFile_setOpenRequest(fs, val)		(wbFile_getOpenRequest(fs) = val)
#define wbFile_setReadRequest(fs, val)		(wbFile_getReadRequest(fs) = val)
#define wbFile_setWriteRequest(fs, val)		(wbFile_getWriteRequest(fs) = val)
#define wbFile_setOpenedQ(fs, val)			(wbFile_getOpenedQ(fs) = val)
#define wbFile_setOffset(fs, val)			(wbFile_getOffset(fs) = val)

extern wbFile_t wbFile_new(const char * path, int flags);
extern void wbFile_delete(wbFile_t file);
extern void wbFile_open(wbFile_t file);
extern void wbFile_close(wbFile_t file);
extern void wbFile_write(wbFile_t file, const char * text);

static inline char * wbString_escape(const char * data) {
	if (data == NULL) {
		return NULL;
	} else {
		size_t len = strlen(data);
		char * res = (char *) calloc(2*len + 1, sizeof(char));
		char * datap = (char *) data, *resp = res;

		if (res == NULL) {
			return NULL;
		}

		while(len-- > 0) {
			if (*datap == '"' || *datap == '\\') {
				*resp++ = '\\';
			}
			*resp++ = *datap++;
		}

		return res;
	}
}

static inline char * wbString_quote(const char * data) {
	if (data == NULL) {
		return NULL;
	} else {
		size_t len = strlen(data) + 3;
		char * res = (char *) calloc(len, sizeof(char));
		res[0] = '\"';
		memcpy(&res[1], data, strlen(data));
		res[strlen(data) + 1] = '\"';
		return res;
	}
}

static inline char * _tempDir() {
#ifdef _WIN32
	char * tmpDir = NULL;
	if ((tmpDir = wbEnvironment_get("TEMP")) != NULL) {
		return tmpDir;
	}
	if ((tmpDir = wbEnvironment_get("TMP")) != NULL) {
		return tmpDir;
	}
	if (wbDirectory_existsQ("C:\\temp")) {
		return "C:\\temp";
	}
	if (wbDirectory_existsQ("C:\\tmp")) {
		return "C:\\tmp";
	}
	return NULL;
#else
	return "/tmp"
#endif
}

static void _randString(char * s, const size_t len) {
	size_t ii;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (ii = 0; ii < len; ii++) {
        s[ii] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = '\0';
}

#define TMP_DIR_BASE_NAME		"wgCUDA-"
#define TMP_DIR_NAME_LENGTH		15

static inline char * wbDirectory_temporary() {
	char * tmpDir, * tmp = NULL;
	char dirName[TMP_DIR_NAME_LENGTH];
	size_t baseNameLength;


	tmpDir = _tempDir();
	if (tmpDir == NULL) {
		return NULL;
	}

	baseNameLength = strlen(TMP_DIR_BASE_NAME);

	memset(dirName, '\0', TMP_DIR_NAME_LENGTH);
	strcpy(dirName, TMP_DIR_BASE_NAME);

	do {
		if (tmp != NULL) {
			wbFree(tmp);
		}
		_randString(dirName + baseNameLength, TMP_DIR_NAME_LENGTH - baseNameLength - 1);
		tmp = wbFile_nameJoin(tmpDir, dirName);
	} while (wbDirectory_existsQ(tmp));

	return tmp;
}

static inline wbBool wbString_sameQ(const char * a, const char * b) {
	if (a == b) {
		return wbTrue;
	} else if (a == NULL && b == NULL) {
		return wbTrue;
	} else if (a == NULL || b == NULL) {
		return wbFalse;
	}
	return strcmp(a, b) == 0;
}

static inline void wbDirectory_delete(const char * dir) {
	return ; /* TODO: implement me. */
}

#endif /* __UTILS_H__ */


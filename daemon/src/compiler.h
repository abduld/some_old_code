
#ifndef __COMPILER_H__
#define __COMPILER_H__

extern char * wbCompile_find(wbContext_t ctx);

extern wbRunner_t wbCompiler_compile(wbRunner_t runner, const char * dir, const char * prog, char ** exe);

#endif /* __COMPILER_H__ */

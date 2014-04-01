

#ifndef wbError_define
#error "wbError_define is not defined
#endif /* wbError_define */

wbError_define(success, "success")
wbError_define(uv, "uv_error")
wbError_define(fail, "failure")
wbError_define(buildFailed, "BuildFailure")
wbError_define(unknown, "unknown")

#undef wgError_define

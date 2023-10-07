#ifndef __DFB_UTIL_COMMON_H__
#define __DFB_UTIL_COMMON_H__

#include <memory>

//////////////////////////////////////////////////////////////////////////

#define DFBCHECK(x...)\
{\
    DFBResult err = x;\
    if (err != DFB_OK) {\
    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );\
    DirectFBErrorFatal( #x, err );\
    }\
}

#define SAFE_RELEASE(PTR) if(PTR) {PTR->Release(PTR);} PTR=NULL;


#define SAFE_DELETE(PTR) if(PTR) {delete PTR; PTR = 0; }


#define FORBIDDEN_COPY(CLASSNAME) \
    CLASSNAME(CLASSNAME&);\
    CLASSNAME(const CLASSNAME&);\
    CLASSNAME& operator = (const CLASSNAME&);


#define DECL_PRIVATE(CLASS)\
    std::auto_ptr<CLASS##Private> p;


#define D_P(CLASS)\
    p = std::auto_ptr<CLASS##Private> (new CLASS##Private);


#define RESOURCE(REC)\
    "./rec/"#REC

#endif
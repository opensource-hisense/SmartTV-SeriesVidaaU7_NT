#ifndef __DFB_UTIL_TESTBENCH_H__
#define __DFB_UTIL_TESTBENCH_H__

#include "dfb_util_common.h"


#define ADD_TESTCASE(TESTCASE)\
    static TESTCASE _testcase

/////////////////////////////////////////////////////////////////////////
class CTestCase
{
public:
    CTestCase();
    virtual ~CTestCase();
    virtual void initResource(){} // Invoke before the "main" function
    virtual void destroyResource(){} // Invoke after the "main" function
    virtual int main( int argc, char *argv[] ) = 0;
};


/////////////////////////////////////////////////////////////////////////
class CTestBenchPrivate;
class CTestBench
{
    FORBIDDEN_COPY(CTestBench);
    DECL_PRIVATE(CTestBench);

public:
    ~CTestBench();

    static CTestBench& GetInstance();

    int Execute (int argc, char *argv[]);

    void Attach (CTestCase * testcase);

    void Detach (CTestCase * testcase);

private:
    CTestBench();
};



#endif
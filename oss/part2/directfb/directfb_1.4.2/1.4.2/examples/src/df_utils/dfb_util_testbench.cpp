#include <set>
#include "dfb_util_testbench.h"

using std::set;


CTestCase::CTestCase()
{
    CTestBench::GetInstance().Attach(this);
};

CTestCase::~CTestCase()
{
    CTestBench::GetInstance().Detach(this);
}

/////////////////////////////////////////////////////////////////

class CTestBenchPrivate
{
public:
    set<CTestCase*> m_TestCaseSet;
};

CTestBench& CTestBench::GetInstance()
{
    static CTestBench testbench;
    return testbench;
}

CTestBench::CTestBench()
{
    D_P(CTestBench);
    p->m_TestCaseSet.clear();
}


CTestBench::~CTestBench()
{

}

int CTestBench::Execute (int argc, char *argv[])
{
    int ret = 0;

    set<CTestCase*>::iterator iter = p->m_TestCaseSet.begin(), iter_end = p->m_TestCaseSet.end();

    for(;iter != iter_end; ++iter)
    {
        (*iter)->initResource();

        ret |= (*iter)->main(argc, argv);

        (*iter)->destroyResource();
    }
    return ret; 
}

void CTestBench::Attach (CTestCase * testcase)
{
    p->m_TestCaseSet.insert(testcase);
}

void CTestBench::Detach (CTestCase * testcase)
{
    p->m_TestCaseSet.erase(p->m_TestCaseSet.find(testcase));
}
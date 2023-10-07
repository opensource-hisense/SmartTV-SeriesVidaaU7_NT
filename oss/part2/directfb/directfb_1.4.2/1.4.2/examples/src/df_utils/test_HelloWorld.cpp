#include "dfb_util_testbench.h"
#include "dfb_util.h"


// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestHelloWorld : public CTestCase
{
public:

    virtual void initResource(){} // Invoke before the "main" function (optional)
    virtual void destroyResource(){} // Invoke after the "main" function (optional)

    // Must to override
    int main( int argc, char *argv[] )
    {
        // Do something here
        return 0;
    }
};

// Step 2: Add it into the Execution List
ADD_TESTCASE(TestHelloWorld);
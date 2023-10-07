#include "dfb_util_testbench.h"
#include "dfb_util.h"

#include <fusion/fusion.h>
#include <fusion/object.h>

FusionCallHandlerResult callback ( int caller,   /* fusion id of the caller */
                                   int call_arg, /* optional call parameter */
                                   void *call_ptr, /* optional call parameter */
                                   void *ctx,      /* optional handler context */
                                   unsigned int serial,
                                   int *ret_val )
{
    printf("[Andy] %s (%d)", __FUNCTION__, __LINE__);

    printf("[Andy] caller = %d\n", caller);
    printf("[Andy] call_arg = %d\n", call_arg);
    printf("[Andy] call_ptr = 0x%08x\n", call_ptr);
    printf("[Andy] ctx = 0x%08x\n", ctx);
    printf("[Andy] serial = %u\n\n", serial);
    
    *ret_val = 255;

    return FCHR_RETURN;
}

// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestFusionCallee : public CTestCase
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
ADD_TESTCASE(TestFusionCallee);
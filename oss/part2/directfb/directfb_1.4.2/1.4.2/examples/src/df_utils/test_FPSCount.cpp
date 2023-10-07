#include "dfb_util_testbench.h"
#include "dfb_util.h"
#include <unistd.h>

// Step 1: Declare Your Test Cast by inheriting "CTestCase" & don't forget to override the "main" function.
class TestFPSCount : public CTestCase
{
public:

    virtual void initResource(){} // Invoke before the "main" function (optional)
    virtual void destroyResource(){} // Invoke after the "main" function (optional)

    // Must to override
    int main( int argc, char *argv[] )
    {
        // Use-Case 1 : Scope FPS Count
        {
            // Declare a "local" PProbe object in front of the codes which needs to be evaluated the performance.

            // You can add the logs in the parameter of Constructor, then it will 
            // display together with the "FPS" after leaving this scope.
            PProbe probe (__FUNCTION__, __LINE__); 


            usleep (16667); // 60 FPS 

        }


        // Use-Case 2: Loop FPS Count
        float fps = 0;
        PProbe probe;

        // Assign an interval which you want to use to average the result. (more large, more precise)
        probe.setRepeatInterval(100);

        for(;;)
        {
            // Start counting
            probe.start();

            usleep (16667); // 60 FPS 

            // Stop counting
            probe.stop();

            // Calculate the FPS
            // If the returned value is 0, that means the calculation is invalid.
            printf("FPS = %s\n", probe.getStringFPS());
        }

        return 0;
    }
};

// Step 2: Add it into the Execution List
ADD_TESTCASE(TestFPSCount);
#include "dfb_util_testbench.h"

int main( int argc, char *argv[] )
{
    return CTestBench::GetInstance().Execute(argc, argv);
}

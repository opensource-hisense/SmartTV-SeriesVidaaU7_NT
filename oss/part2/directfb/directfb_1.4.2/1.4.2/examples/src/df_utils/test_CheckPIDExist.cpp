#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <directfb.h>

#if 1 // Method 1
int main( int argc, char *argv[] )
{
	int input, result;
	system("ps -T");
	printf("input pid¡G");
	scanf("%d", &input);
	printf("pid = %d \n", input);

	result = kill(input, 0);

	printf("result = %d \n", result);

	return 0;
}
#else // Method 2
#include <signal.h>

int main (int argc, char * argv[])
{
	printf ("%s\n", !kill (atoi(argv[1]), 0) ? "Running" : "Not Running");
	return 0;
}
#endif

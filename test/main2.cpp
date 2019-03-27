#include <stdio.h>

int test(int a, int b = 321)
{
    printf("Hello: %d %d\n", a, b);
}

int main()
{
    printf("Hello World\n");
    test(32);
}
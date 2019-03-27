#include <stdio.h>

typedef struct Point
{
    int x;
    int y;
} Point;

void test(int a, int b)
{
    printf("Test: %d %d\n", a, b);
}
void wooh(int a, int b) __attribute__((alias("test")));

int main(int argc, char **argv)
{
    Point point = {123, 321};
    point.x = 5;

    printf("Point: %d %d\n", point.x, point.y);

    test(123, 321);
    wooh(321, 123);

    return 0;
}
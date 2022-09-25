#include <stdio.h>

int example()
{
    return 10;
}

int main()
{
    int number1 = 0;
    int number2 = example();
    int number3 = 5;

    while(0)
    {
        example();
    }

    if(example() == 100)
    {
        printf("hmmmm\n");
    }

    printf("Example = %d \n", example());

    example();

    return 1;
}
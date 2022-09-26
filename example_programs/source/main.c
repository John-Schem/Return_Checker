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
        example(); //Warn
    }

    if(example() == 100)
    {
        int example1 = printf("hmmmm\n");
    }

    int example2 = printf("Example = %d \n", example());

    example(); //Warn

    return 1;
}
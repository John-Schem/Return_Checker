#include <iostream>

struct Structure
{
    int var = 5;

    Structure()
    {

    }

    int exampleMethod(int stuff)
    {
        int random = 10;

        return stuff;
    }
};

int main()
{
    Structure structure;
    structure.exampleMethod(true); //Warn

    structure.exampleMethod(structure.exampleMethod(true)); //Warn
    int example2 = structure.exampleMethod(true);
}

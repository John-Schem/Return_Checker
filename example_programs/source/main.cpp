#include <iostream>

struct Structure
{
    int var = 5;

    Structure()
    {

    }

    int exampleMethod(bool stuff)
    {
        int random = 10;

        return stuff;
    }
};

int main()
{
    Structure structure;
    structure.exampleMethod(true);

    //std::cout << "Hello :)";
}

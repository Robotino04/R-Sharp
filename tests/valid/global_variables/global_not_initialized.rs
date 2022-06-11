/*
return: 3
*/

foo: int;

main(): int {
    for (i: int = 0; i < 3; i = i + 1)
        foo = foo + 1;
    return foo;
}
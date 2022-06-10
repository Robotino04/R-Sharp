/*
return: 12
*/

foo(a: int): int;
bar(b: int): int;

main(): int {
    return foo(5);
}

foo(a: int): int {
    if (a <= 0) {
        return a;
    }

    return a + bar(a - 1);
}

bar(b: int): int {
    if (b <= 0) {
        return b;
    }

    return b + bar(b / 2);
}
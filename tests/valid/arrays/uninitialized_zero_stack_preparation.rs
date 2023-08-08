/*
compilationExitCode: 0
*/

fill_stack(): c_void{
    a: i8 = 1;
    b: i8 = 2;
    c: i8 = 3;
    d: i8 = 4;
    e: i8 = 5;
    f: i8 = 6;
    g: i8 = 7;
    h: i8 = 8;
    i: i8 = 9;
    j: i8 = 10;
}

foo(): i64{
    a: [i64, 3];

    return a[0] + a[1] + a[2];
}

main(): i32 {
    fill_stack();
    return foo();
}
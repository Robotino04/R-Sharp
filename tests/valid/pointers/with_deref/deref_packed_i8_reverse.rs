/*
executionExitCode: -4
*/

[extern] malloc(size: i64): *c_void;
[extern] memset(pointer: *c_void, value: i32, size: i64): *c_void;
[extern] free(pointer: *c_void): c_void;

main(): i32 {
    base: *c_void = malloc(8);
    memset(base, 255, 8);
    a: *i8 = base;
    b: *i8 = base - -1;
    c: *i8 = base - -2;
    d: *i8 = base - -3;
    e: *i8 = base - -4;
    f: *i8 = base - -5;
    g: *i8 = base - -6;
    h: *i8 = base - -7;
    *h = -8;
    *g = 7;
    *f = -6;
    *e = 5;
    *d = -4;
    *c = 3;
    *b = -2;
    *a = 1;
    return *a + *b + *c + *d + *e + *f + *g + *h;
}

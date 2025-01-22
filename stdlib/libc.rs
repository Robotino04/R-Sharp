[extern] malloc(size: i64): *c_void;
[extern] memset(pointer: *c_void, value: i32, size: i64): *c_void;
[extern] free(pointer: *c_void): c_void;
[extern] getchar(): i32;
[extern] putchar(character: i32): i32;

[extern] write(file: i32, pointer: *c_void, size: i32): i32;
[extern] read(file: i32, pointer: *c_void, size: i32): i32;

[extern] strlen(str: *c_void): i32;

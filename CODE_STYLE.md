# Language
All code is to be written in C.

## Functions
All functions will use **snake case**
```c 
void test_function() {}
```

## Variables 
All variables will use **snake case**
```c 
int variable_name; 
```

## Constants
All constants will use **screaming snake case**
```c
#define MAX_VALUE 1024 
const int MAX_VALUE = 1024;
constexpr int MAX_VALUE = 1024;
```

## Enum 
All enums will use **pascal case**  
Their members will use **screaming snake case**  
```c
typedef enum {
    FIRST_VALUE,
    SECOND_VALUE,
    THIRD_VALUE,
} MyEnum;
```

## Types
All types will use **pascal case**  
The only exception is redefining <stdint.h> types  
```c
typedef int TheNewType;
```

## Struct
All structs will use **pascal case**  
Their members will use **snake case**  
```c
typedef struct {
    int member_one;
    int member_two;
    int member_three;
} TheStruct;
```

## Other 
If it's an acronym, the following is done 

### Snake Case 
```c 
int pid;
``` 

### Pascal Case 
```c 
typedef int PID;
``` 

# PL/0 Programming Language

PL/0 is a programming language developed for a school project.
It was originally submitted as 4 separate assignments 
not meant to work together.

This will be improved upon as some of this code isn't my best work due to strange requirements etc.

The compiler generates code meant to run on a virtual machine 
defined in vm.c. vm.c needs to be completely reworked because our
insane professor would give us a 0 on the assignment if we defined
any functions other than main() and base(). 

Some students did actually get 0s for this.

### Basic Syntax
Whitespace is only used to separate identifiers and can be ignored
elsewhere.

Scoping works like most programming languages.

#### Code Blocks
Anywhere ```<statement>``` is listed can be a single statement.
If you want to do multiple instructions, replace ```<statement>```
with
```
begin
    <statement>;
    <statement>;
    ...
    <statement>
end;
```
#### Function/Procedure Declaration:
Procedures must be defined in a specific order. The procedure is declared with an identifier followed by constant declarations, variable declarations, then subprocedure declarations, and finally a statement or statement list.
```
procedure hello_world;
    const one := 1;
    var test_var;
    procedure print;
    begin
        ...
    end;
begin
    ...
end;
```

Main is implicit to every program and is declared the same way omitting ```procedure main;```
```
var test;
begin
    test := 1;
    write test;
end.
```
#### Calling Functions:
There are no parameters so anything we want to pass to a function needs to be a global variable.

Functions are called using ```call```
```
call hello_world
```

#### Variables/Constants
All variables and constants in PL/0 are integers. Constants are declared at initialization and variables are not.
```
const a := 1;
var b;
```
Both can be declared in lists
```
const a := 1, b := 2, c := 3;
var d, e, f;
```
#### If statements:
```
if <condional statement> then
    <statement>
else if <conditional> then
    <statement>
else
    <statement>
```
#### While loops:
```
while <condition> do
    <statement>
```
#### System I/O:
We can read from stdin into a variable using ```read``` and print to the screen using ```write```

Echo example:
```
var input;
begin
    read input;
    write input
end;
```

#### Math
Add, subtract, multiply, divide, and modulo work as they do in most languages.

Conditionals are the same as C except for not equals being ```<>```
```
const ten_pi = 314;
begin
    test := (ten_pi * 10 / 10 + 1 - 1) % 314;
    if test == 0 then
        <do something>
    else if test <> 0 then
        <do something else>
    
    if ten_pi >= 10 then
    ... 
end.
```

Full examples are in the examples folder!
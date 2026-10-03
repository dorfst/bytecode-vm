# Bytecode VM

# What is this project?
This project is a simple bytecode VM. It takes a source file in an assembly-like language, converts it into a binary format, which can then be read and executed by a virtual machine similar in nature to a real CPU.

This project gets updated from time-to-time, so what you see is not necessarily final.

C, custom instruction set design, assembler/compiler pipeline, dispatch-table, interpreter, hash tables

# If you're short on time
Check out these sections, as I think they're worth reading the most:
- [The symbol table redesign](DEBUGGING.md#symbol-table-layout)
- [`parse.c`](src/parse.c) + [`serialise.c`](src/serialise.c) for the label resolution split
- [`execute.c`](src/execute.c) for the dispatch table + the `execute()` loop
- [the `fclose` bug](DEBUGGING.md#file-pointers)
- [Limitations & assumptions](#limitations--assumptions)
# How do I use this?
The main program expects an already-existing source file and a name for the output file, as it will compile and then run the program.

```bash
make
./output/vm source_path/program.txt destination_path/program.bc
```
or alternatively, using one of the examples
```bash
make
./output/vm examples/example_source.txt destination_path/program.bc
```

# A working example
From the root of the project, try running `sum_to_five.txt`

```bash
make
./output/vm examples/sum_to_five.txt output/sum_to_five.bc
```
Or, rather
```bash
./quick-test.sh
```
They are equivalent.

You should get this output:
```
gp registers
15
6
0
0
.
.
.
0 (all zeroes)
heap memory
0
0
0
.
.
.
0
0
0 (all zeroes)
```

You can alternatively use `make release`, since `make` on its own produces a debug build by default.

If your system does not have `gcc`, which is what the Makefile expects by default, you can override `CC` and `CFLAGS` manually
to use whatever compiler you have available.

If you want to rebuild, run `make clean` and then `make`/`make release` again.

# Motivation
I thought this would pair nicely with my [AArch64 kernel project](https://github.com/dorfst/aarch64-kernel), which goes from wrangling the CPU (especially getting virtual memory to work), to this project, which is a slightly different angle of processing the text
made to do that.

To be honest, I wanted to do something that I had considered quite difficult, and I was certainly right in that prediction,
even at the scale that it is now. I wanted to do something I hadn't done before, something that I knew basically nothing about
in the beginning, and something that wasn't taught on my course (at least at the time of making this project).

It's a very simple motivation, but ultimately I wanted to become a better programmer from this experience.
I have definitely done some suboptimal things in this project but it definitely got me more comfortable about
writing full programs in C, as well as experiencing first-hand the problems that can come from a language that
is as permissive as C.

## Syntax
The language is an assembly-like language. It takes the following format:
```
operation arg1, arg2, arg3
```
where `arg1`, `arg2` and `arg3` are optional depending on the command

or optionally
```
operation arg1 arg2 arg3
```

`arg1`, `arg2` and `arg3` can either be a register from `r0`-`r30`, or they can be an immediate value up to 2^64 - 1, e.g. `#16384`

The syntax with the commas is preferred stylistically, but either works.

### Instruction set
- `NOP` - no-op, does nothing. increments program counter and continues
- `ADD rn, r/#x, r/#y` - add argument 2 and 3, store in `rn`. Arguments 2 and 3 can respectively be a register or an immediate value.
- `SUB rn, r/#x, r/#y` - subtraction, same format as ADD
- `MUL rn, r/#x, r/#y` - multiplication
- `DIV rn, r/#x, r/#y` - integer division
- `MOV rn, r/#x` - move `rx` or `#x` into `rn`
- `LDR rn, r/#x` - load whatever is in memory address `rx` or `#x` into `rn`. If using a register value the value inside `rx` is taken as a memory address
- `STR r/#n, r/#x` - store into memory address in `rn` or `#n` the value in `rx` or `#x`
- `CMP r/#n, r/#x` - compare the two arguments, which sets comparison flags (equal, greater than, less than)
- `JMP label` - unconditional branch to a label in your program source. The instruction can be placed before or after a label definition (in the form `label:`)
- `JGT label` - jump to `label` if `GT` flag set to `true`
- `JLT label` - jump to `label` if `LT` flag set to `true`
- `JEQ label` - jump to `label` if `EQ` flag set to `true`
- `JNE label` - jump to `label` if `EQ` flag set to `false`

Conditional jumps and comparison operators reset the comparison flags in the virtual machine.

### Labels
Forming a label is quite simple. There are no restrictions on starting or ending characters, but must follow the format
`label:`, where label is the name of your label, ending with a colon.

### Formatting
Tabs and spaces are _not_ allowed for the purpose of formatting, like 
```
label:
    ADD r1, r2, r3
    ...
    ...
```

It must instead be
```
label:
ADD r1, r2, r3
...
...
```

This is also _not_ allowed
```
A DD r 1, #3 21, #45 821 2
```
as an alternate format of

```
ADD r1, #321, #458212
```

The operation cannot have spaces between it, nor can the arguments.

Note that `operation`s must be in uppercase:
```
add r1, r2, r3
```
is not valid. It must be:
```
ADD r1, r2, r3
```

### Comments
Comments are noted by the `;` at the **beginning** of a line. Note that a comment must be its own line and comments
cannot be made on the same line as an instruction.

For example, you can do
```
; this is a comment
ADD r1, r2, r3
```
but you _cannot_ do
```
ADD r1, r2, r3 ; this is also a comment
```
rather, this makes the whole line a comment, silently removing the instruction from the program.

## Limitations & Assumptions
- Limited code validation. Checks register bounds, memory bounds, and unknown mnemonics, missing files and division by zero.
- Assumes you use the correct number of arguments. This is not checked.
- Assumes your program is at most 256 instructions long.
- Assumes there are no blank lines in the text file.
- Assumes that you don't use duplicate labels in your program. Otherwise, only the first instance of that label will work.
- Assumes you use the correct type of argument (register/immediate) in the correct position.
- Assumes that you do not use leading whitespaces at the beginning of your source on any line.
- Assumes that comments are a separate line that gets totally ignored.

This list is not necessarily exhaustive. There may have been things I've missed or only mentioned in the main document body.

# Components
## 1. Parsing
The first stage that happens is parsing. What happens in parsing is two-fold: the first thing to be done is to take a line of code and compress the information in it to a structure that is easy to use for when it is time to convert to a binary format, and the second is to form a symbol table, where jump instructions can use a label to denote a position to jump to rather than using line numbers.

### Instruction Parsing
Instruction parsing is relatively simple, since instructions follow a simple format: operation -> argument1 -> argument2 -> argument3, where arguments 1 to 3 are optional depending on what instruction this is. The goal is to convert this into something easily serialisable, and easily interpretable for later use.

#### Instruction Layout
Currently, each instruction takes 26 bytes (yes, bytes) of space. This is quite a lot compared to real-life modern architectures like AArch64, where their instructions are only 4 bytes (32 bits) big. The reason for this is mostly for simplicity, as well as the size of the arguments that I've chosen to allow. Let's go over the current design of an instruction in its binary form, and we'll see how it could be improved, even though for its purpose as a learning project it is sufficient.

```
  1 byte       1 byte          8 bytes         8 bytes        8 bytes
__________________________________________________________________________
|  opcode  |  argument info  |  argument 1  |  argument 2  |  argument 3  | 
__________________________________________________________________________
```

##### Opcode 
The opcode is 8 bits large. This is excessive, considering that I only have 14 instructions, but it does create a convenient alignment. This theoretically could be cut down to 4 bits instead.

##### Argument Information 
The argument info field is 8 bits large. At the moment it contains 2 bits for arity (which, given the design of the actual execution portion, is redundant), and 3 bits as register flags, i.e. is this argument referring to a register or an immediate value? This could theoretically be cut down to 3 bits. If I wanted to have convenient alignment I could have just one extra padding bit.

##### Arguments 
All arguments in their _final form_ are interpreted as 64-bit unsigned integers. Whether they are immediate values or refer to a register is decided by the argument information in the instruction. Here is where the most space could be saved. The simplest solution is to restrict the size of the arguments for every instruction, especially since it is very unlikely in any program that one would be using extremely large immediate values that would warrant so many bits. 

Another solution is to determine how many bits an immediate value could have (at the very least 5 bits since that's the number of registers in our virtual machine, much like the AArch64 architecture) based on the instruction, i.e. instructions that take fewer arguments could potentially take advantage of larger immediates if it were appropriate to do so, or similarly take away bits for immediate values and potentially use them in argument information if instructions were able to have some extra configuration or variation that would warrant extra argument information.

4-byte arguments would certainly reduce the instruction size to 14 bytes total.

Of course, for a virtual machine like this, instruction size doesn't matter that much besides the storage space it takes and the time taken to load a program into the virtual machine, since the instructions in a binary format do get informationally compressed into a struct that has the essential info needed. For real hardware the tradeoff between larger instructions and more complex decoding becomes much more relevant.

I will explain how jump instructions are a special case when it comes to arguments later.

#### How does an instruction get parsed?
The parser expects the instruction to be formatted in the order of `operation`, `argument 1`, `argument 2` and `argument 3`. Currently, any sort of formatting where the instruction is _not_ at the beginning of the line (i.e. spaces or tabs before the first instruction character) will not work with the parser.

An operation gets matched to an opcode (a number identifying the instruction) by a string comparison over an array of instruction mnemonics.

Operands (arguments 1-3) can be either a register or an immediate value, and when writing source code this is differentiated by an `r` prefix to denote a particular register number, and a `#` prefix for immediate values.
The parser will only scan for the number of arguments required once it has identified the operation, i.e. arithmetic operations require three operands, and so it will look for three, but memory operations require two arguments and the parser will only search for two.
In the event where an instruction does not need all three arguments, the unused arguments will be set to zero, and effectively function as padding for later.

##### Jump Instructions
The description above refers to how almost every instruction gets parsed, but jump instructions are different because their argument does not take a numeric form.

Conditional jumps reset the comparison flags in the virtual machine.

Every line of source gets put into a `line` struct, which contains the opcode, arguments, register flags (i.e. which arguments are a register number), an `is_label` flag, as well as a `label` attribute which
contains the string value of the label.

For jump instructions, the "argument" gets put into the `label` attribute rather than the `args[3]` attribute, but in the
serialisation stage that label turns into a program counter value placed in the same place as arguments in the regular case.

#### How do labels get parsed?
The syntax for defining a label is `label:` where `label` is the name of your label, with the last character being a colon.
Instructions cannot have colons in them so the presence of the colon character is key in deciding whether a line is a label or an instruction.

#### How are labels resolved?
In the parsing stage a full symbol table is formed. The symbol table is a hash table indexed by the label value, which results in the corresponding
program counter value associated with a label. In the parsing stage, the table is fully formed, and the references are resolved in serialisation. This is because it makes forward references
a non-issue.

## 2. Serialisation
In this stage, we take our `line` struct and convert it into a binary format (a `.bc` file). Per the instruction format from earlier, we have:

- opcode (1 byte)
- argument information (1 byte)
- argument 1 (8 bytes)
- argument 2 (8 bytes)
- argument 3 (8 bytes)

Each instruction is fixed in size, and formatted in the order described above (top to bottom, first to last).

### Opcode
This is fairly simple. It's just the opcode number written as an 8-bit integer.

### Argument Information
As of now, there are two distinct pieces of information held in the argument information section, one of which is now redundant.

#### Arity
Bits 3-4 (counting from 0) refer to the arity (number of arguments) of the instruction, ranging from 0 to 3. This is redundant since
the structure of the virtual machine naturally deals with the correct number of arguments for each operation.

#### Register flags
Bits 0-2 are flags that determine whether the argument refers to a register rather than an immediate value. Bit 2 is for argument 1,
bit 1 is for argument 2 and bit 0 is for argument 3. A flag at the value of 1 means that the specified argument _is_ a register.

### Arguments
Each argument is a 64-bit unsigned integer, and 3 of these are placed one after the other. Whether the argument refers to an
immediate value or a register is determined by the register flags. For example, the value `3` could either mean the immediate value
`#3` or it could refer to the register `r3`, and that is determined by the register flags in the argument information section.

### What about unused arguments?
Unused arguments are set to zero and simply don't get touched by the virtual machine, and only serve to have a consistent 3-argument
format (as we will see in the virtual machine section later.) That means that all instructions are 26 bytes even if it takes between 0 and 2 arguments.


## 3. Execution
The execution stage refers to setting up the virtual machine: initialising its state and loading the bytecode program into the virtual machine.

### Initialising the virtual machine
#### Registers & Memory
Registers (including the program counter) and memory are set to zero.

#### Loading the program into the virtual machine
This requires decoding the binary format created from the earlier section. The virtual machine stores a list of
`instruction` structs, similar to the `line` struct but without extraneous parsing-specific fields. The `instruction` struct
contains the opcode, arguments, and register flags.

This is where a fixed-size instruction becomes very convenient (hence keeping unused arguments), because it makes looping through
an output file (`.bc`) consistent. There is no need to reason about how many bytes to consider given a particular instruction.

### The execution loop
The execution loop is very simple: feed the VM state and the `instruction` struct pointer to a handler based on the opcode. Each handler
increments the program counter itself, rather than that happening in the execution loop. The reason for this is to have consistent behaviour across
handlers because the jump instruction specifically sets the program counter in a way that is not a simple increment. It's a little simpler
because you don't need to check if the current instruction is a jump on every loop to not increment the program counter then.

#### Operation handlers
The operation handlers are functions that perform the instruction it represents. The interface is the same across all of them: `void` return type, pass in a `vm_state*` and an `instruction*`.

#### How are operation handlers selected?
Pointers to the operation handlers are placed in a jump table, where the opcode matches to the operation handler.

For example, the current instruction may have an opcode of 1. The function pointer at index 1 of the jump table is chosen to run the function. This makes
choosing an operation handler efficient since there is no condition checking in each iteration.

# Debugging
This project wasn't a smooth ride, that's for sure. A lot of problems came up on the way, and I wanted to share ones that I thought
were noteworthy. The purpose of this section is to see what sort of problems came up, and how I figured them out
to see my thought process throughout this project. It is quite a long section, so I put it in a separate file called
[`DEBUGGING.md`](DEBUGGING.md).

# Conclusion
This was quite an interesting project and definitely out of my comfort zone, especially since dealing with strings in C
is no easy feat.

All I really have left to say is that what I would do differently is:
- consider a higher level language which compiles to this instruction set. A *much* bigger undertaking but worth exploring.
- more elaborate syntax rules (e.g. allowing comments on the same line as instructions) and generally a better implementation of said rules.
- design a more storage-efficient instruction format
- more elaborate validation



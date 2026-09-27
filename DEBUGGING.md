# Debugging Notes/Pitfalls
This section is a collection of problems I've come across along the way in making this project, not necessarily in chronological order.
The purpose of this section is just to show how getting to the current state of the project wasn't one smooth ride.

## Label Resolution
Before the current solution to label resolution, I had a different idea of how it would work.

I was under the impression that labels would be resolved on the fly in the serialisation stage, where I scan for
labels, and note `label_position - labels_encountered`, where `labels_encountered` is the number of labels not including the current
one that have already been encountered. I was under the impression that in a list of lines I'd have to delete them for the final
serialisation stage, which requires shifting where jump instructions point. As you can see, there's also just the option to ignore label lines
with counters.

### The solution
The solution to this conundrum was:
1. never count a label as something that takes up a line at all, and to think in terms of instructions rather than lines.
2. form the symbol table fully in the parsing stage, and then resolve in the serialisation stage.

To elaborate on point 1, I had previously thought in terms of lines since I considered labels and instructions taking up space,
so I thought in slightly more general terms. Instead, have a counter, say, `instruction_counter` (not actually called that in the source),
and only increment `instruction_counter` when hitting an instruction, rather than on both labels and instructions.

You may have noticed that earlier I referenced to having a list of `line` structs, but in the source the `serialise_line` function
actually only takes one line. I could have done it either way, but decided that it was a little simpler for `serialise_line` to work
on one line at a time. I don't count this as part of the solution, and I was just referring to how I _expected_ it to be organised.


Don't worry if all of this is hard to understand. It is quite a strange way to look at tracking positions and is ultimately overcomplicated compared to the current solution.

### Forward Referencing
In the current implementation of label reference resolution the symbol table is fully formed and then only in the serialisation stage are jump
instructions resolved to a program counter number. I had some test code from before that would parse a line and then immediately translate it, which caused quite strange output
where forward references would end up resolving to `2^64 - 1` because there was nothing that could be resolved yet.

I had the idea of separating the parsing and serialisation stages before (and had written it with that in mind), but forgot to change the structure of the actual test code to reflect that.

Just a note to make sure your test code actually tests correctly :)


## File Pointers
This one is quite an interesting one in my opinion because it can show how far a symptom can be from the cause
of a bug.

The bug starts in the `translate` function in [`serialise.c`](src/serialise.c). This function takes file names for the source and output file.
In this file, it creates two file pointers for the source and output respectively. To put it shortly, I forgot to run `fclose` on the file pointers,
and that caused a big problem down the line.


The symptom came up as this: I'd run the code to set up the virtual machine and load the program into it. It executed _zero_ instructions.
I could see that no state had changed in the virtual machine despite expecting different results.

I place some print statements just to make sure my operation handlers are actually getting called. They were not being called.

I look at the dispatch table. I see nothing wrong with it since that is how you're supposed to make one in C. I look at the way
I'm attempting to call functions from the dispatch table. Couldn't see any issue with it.

I go into GDB and find that the instruction count is zero and that the program counter is zero, and never
actually enters the execution loop because the condition in the `while` loop is always false.

"Strange...", I think, "I was literally reading and writing files correctly earlier". I check the code that reads how many instructions there are and
I check the code that then reads the bytecode file to get the instructions. I mean, it was the same as before, and it was working earlier. What's wrong with it now?

Those functions take in a file pointer to the bytecode file as an argument, so the file is expected to already be open.

What seemed to have been happening is that because the `translate` function never closed the file,
the written contents of the file were never actually written to disk, which requires a call to `fclose`.

What happens then is that `init_vm_state` then tries to load an empty program because the file itself is empty. So, you have an empty program with zero instructions, which causes the condition for the execution loop
(that is, there are still instructions to run) to be false, and nothing ever happens.

It's funny how simple the fix was but produced such confusing results.

## Determining a register vs. an immediate
Essentially, the operation handlers extract whether an argument is a register or an immediate
from the argument info section as detailed earlier.
Let's just say that I wasn't using named constants and eventually started
to use the wrong bit offsets for the actual arguments I needed. I was also relying
on truthy values evaluating correctly which isn't a problem per se
but in general it's probably better for the result to be a strict
1/0 (true/false).

What's interesting about this one though is that the symptom this bug produced an infinite loop. I had
thought it was something wrong with maybe the `CMP` instruction or the conditional jump instructions. The logic itself for `CMP` wasn't
wrong but the part where the arguments get sorted out most certainly was. At some point I had even suspected that somehow
my label resolution code had gone wrong, or that maybe the serialisation stage was writing incorrect output, but it was not so. I had to
meticulously check each and every output for each and every stage.

I also then realised I did this exact same mistake for multiple other instructions and then had to fix it. I also settled for making
the result either a 1 or a 0 just because in general relying on a truthy value is only safe insofar as wherever it is used
_accepts_ truthy values rather than requiring a strict boolean. So generally speaking it's safer to convert to a strict boolean
value.

Now, if you've read the source, you may have noticed "but you used named constants in the dispatch table... why not in the operation
handler code?" Well... I thought that the dispatch table required extra care that warranted the use of preprocessor directives, and it made
the code a little clearer.

Turns out, no constant is too small to have a name attached to it if it is particularly important or is used multiple times.

## Symbol table layout
The original symbol table layout was one like `struct hash_table_node symbol_table[256]`, which then got revised to `struct
hash_table_node* symbol_table[256]` in an attempt to fix a whole load of leaks and potential bugs regarding memory allocation.

The issue was this: in the previous layout, the head of each linked list of `hash_table_node`s gets copied by value, while
the rest that are linked after that would be _required_ to be allocated on the heap. Given the way parsing works this becomes quite
complicated, and I would then have to restructure parsing to account that _as well as_ fix the memory leak itself.

When it's an array of pointers instead, I can simply `malloc` a `hash_table_node` at parse time, write the _pointer_ to the array,
and then I can clean up all the structs by iteratively traversing through the linked list where one exists.

This was also made slightly problematic by the fact that when a line would occur in the form of
`label:`, i.e. a label declaration, the `line` struct _and_ the `hash_table_node` struct would share the _same_ pointer to the same label string.
This was a bit of a headache to unwrap, and it wouldn't be a practical problem if I free the memory taken up by labels
after the symbol table is no longer needed, but in general this could easily go wrong as it is complicated
trying to track the lifetime of one pointer across multiple structs used in different ways.

Anyway, I decided to not store a label upon a label declaration like `label` and to only store it in jump instructions
like `JNE loop` where it is necessary and I had previously designed it so that the label stored in the `line` struct would
be separate from what is stored in the symbol table (i.e. identical string value, different pointers) specifically when processing a
`JMP` instruction or its variants.

The lesson for this one is to think about the lifetime of `malloc`ed
memory _before_ the whole program is basically written. Thankfully modern operating systems
reclaim all that memory anyway, but it's good practice to free pointers at the point where they are no longer needed.

## Headers & Compilation
These two bugs aren't really anything as major as the label resolution problem or the file pointer problem, but I did think that they were
at least somewhat amusing and would be something nice to close with.

### Circular Dependencies
I had two headers [`headers/vm_state.h`](headers/vm_state.h) and [`headers/execute.h`](headers/execute.h) which `#include`d each other. They both had references
to structs which weren't defined in their own file. The compiler gave a warning:

`
warning: ‘struct vm_state’ declared inside parameter list will not be visible outside of this definition or declaration 
`

Resolution: put both structs in one header file and just make sure only one of them imports the other. `headers/execute.h` includes `headers/vm_state.h`,
but not the other way round.

This was sort of amusing, and to be honest I'm not even sure how I managed to do that.

### Compilation Errors
This one was very simple. I had an error `undefined reference to init_vm_state`. I looked at the code,
and it seemed that I `#include`d everything properly, and that everything was spelt correctly.

Turns out I just forgot to compile [`src/vm_state.c`](src/vm_state.c). This is what Makefiles are for, people.
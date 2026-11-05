[English](glossary.md) | [العربية](glossary.ar.md) | [Deutsch](glossary.de.md) | [Español](glossary.es.md) | [Français](glossary.fr.md)

# Glossary

Core terms used throughout the course. Entries are added as modules introduce new vocabulary.

**addressing mode**: The rule an instruction uses to say where an operand is: inside the instruction, in a register, or at a computed memory address.

**assembler**: A program that translates assembly language source code into machine code stored in an object file.

**assembly (.NET)**: A .NET program or library: a PE file that holds intermediate language and metadata.

**back edge**: An edge in the control-flow graph from a block to a block that dominates it; it marks a loop.

**basic block**: A run of instructions with one entry and one exit: control enters at the first instruction and leaves after the last.

**breakpoint**: A marker that makes a debugger stop the program when execution reaches a chosen address.

**bytecode**: A compact instruction format that an interpreter reads and carries out; it is data to the machine that runs it, not machine code.

**calling convention**: The agreement about where a caller places arguments, where the callee returns its result, and which registers each side must preserve.

**checksum**: A value computed from the bytes of a file or message that lets a reader detect damage or changes.

**chunk**: A tagged piece of a file: a tag that says what it is, a length, and a payload.

**control-flow flattening**: An obfuscation that replaces the graph of a function by a loop with a dispatcher and a state variable that holds the number of the next block.

**control-flow graph**: The blocks of a function as nodes and the jumps between them as edges.

**debugger**: A tool that runs a program under control so you can pause it, step through it and inspect registers and memory.

**decompiler**: A tool that turns machine code back into approximate high-level source code, such as C.

**detection note**: A short description for defenders of a behavior to look for: what is observed and where, why it is suspicious, what looks similar but is harmless, and which technique it maps to.

**disassembler**: A tool that turns the bytes of machine code back into assembly instructions.

**dispatch loop**: The central loop of an interpreter: it fetches an opcode, selects the handler for it and runs it, then repeats.

**dynamic analysis**: Studying a program by running it and observing its behavior, usually under a debugger.

**endianness**: The order in which the bytes of a multi-byte value are stored in memory.

**entry point**: The address of the first instruction executed when a program starts.

**epilogue**: The instructions at the end of a function that restore saved registers and the stack, then return.

**export**: A function or variable that a library makes available to other programs, listed in its dynamic symbol table.

**flag**: A single bit in the flags register that records a property of the last result, such as zero or carry.

**indicator**: An observable fact, such as a string, a hash, an imported function or a behavior, that points to a program, a family or an activity.

**instruction**: A single command the processor can execute, made of an opcode and zero or more operands.

**integer overflow**: An arithmetic result that does not fit its type and wraps around, so that a size or index ends up much smaller than intended.

**intermediate language**: The instruction set that managed compilers produce, also called IL or CIL; a runtime compiles it to machine code when it is needed.

**interpreter**: A program that reads instructions in some other format, such as bytecode, and carries them out one by one.

**just-in-time compilation**: Translation of intermediate language to machine code while the program runs, one method at a time, when the method is first used.

**lifting**: The decompiler stage that turns machine instructions into operations on variables, with flags turned into conditions.

**linker**: A program that combines object files and libraries into one executable and resolves references between symbols.

**liveness analysis**: The data-flow analysis that finds which values are read later; assignments to values that are never read are dead and can be removed.

**magic value**: A fixed identifier at the start of a file format that tells a reader which format it is looking at.

**managed code**: Code that runs under a runtime, such as .NET, which loads it, compiles it and manages its memory.

**memory address**: A number that identifies one byte in memory.

**metadata**: Tables and heaps in a managed assembly that describe its types, methods, fields, references and literals.

**obfuscation**: Changing a program so that it is harder to read without changing what it does.

**off-by-one error**: A limit that is wrong by one, such as a check that accepts a length equal to the capacity when the copy also needs room for a terminator.

**opaque handle**: A pointer that a library returns and expects back, whose contents the caller never reads or writes; it identifies a resource that must be released.

**opaque predicate**: A condition that always has the same value but is hard to see at a glance, so that one side of the branch is dead code.

**opcode**: The number in an instruction that selects the operation to perform.

**operand**: A value an instruction works on: a constant, a register or a memory location.

**output parameter**: A pointer argument that a function only writes through, so that the caller receives a result in a variable it owns.

**persistence**: A way for a program to start again after a restart or a logoff, such as a registry Run value or a scheduled task.

**prologue**: The instructions at the start of a function that save registers and set up its stack frame.

**register**: A small, very fast storage location inside the processor.

**reverse engineering**: Recovering how a program or product works from the finished product itself, without its design documents.

**sanitizer**: A compiler feature that adds checks to a program and stops it with a report at the first bad memory access.

**signature**: The return type, name and parameter types of a function; for an undocumented function it is recovered from how its code uses the registers.

**sink**: A place in a program where a value decides how much memory is touched, such as a copy, an index, an allocation or a free.

**stack**: A region of memory used as a last-in, first-out structure for return addresses, arguments and local variables.

**stack frame**: The part of the stack that belongs to one function call.

**stack string**: A string that the code builds byte by byte from constants, so that it never exists as one piece in the file.

**static analysis**: Studying a program without running it.

**stripped binary**: An executable from which symbol names and debugging information have been removed.

**system call**: A request from a program to the operating system kernel to perform a privileged operation.

**triage**: A quick first examination of a file, without running it, to identify what it is and decide where to look next.

**two's complement**: The standard way computers represent signed integers: to negate a number, invert its bits and add one.

**use after free**: Using memory through a pointer after the memory was released.

**virtual machine**: A program that behaves like a processor, with its own program counter, storage and instruction set, and runs bytecode.

**YARA rule**: A rule with strings and a condition that matches files against patterns, used to find programs of one kind.

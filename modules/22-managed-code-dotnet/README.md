[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 22. Managed code: .NET

Time: about 2 hours. Prerequisites: [21. Virtual machines and bytecode](../21-virtual-machines-bytecode/README.md)

## Why this matters

A large part of the software that you will meet is not compiled to machine code. Programs written for .NET are compiled to an intermediate language that a runtime translates while the program runs. Such a file keeps most of what the author wrote: the names of types, methods and fields, the signatures, and the structure of the code. That makes managed code much easier to read than native code, and it is why a decompiler can often return source that is close to the original. It also means that the habits of native analysis do not transfer unchanged: there are no registers to follow and no stack frames to rebuild, and the useful evidence sits in metadata tables and an instruction set for a stack machine. This module teaches how a managed program is organized, how to read its metadata and its intermediate code, how to use a decompiler, and what the decompiler gets right and wrong. All examples are harmless programs written for the lesson.

## Concepts

### Managed code and native code

The table lists what changes when the target is managed.

| | Native code | Managed code (.NET) |
|---|---|---|
| What the file holds | Machine instructions for one processor | Intermediate language (IL, also called CIL) and metadata, for any processor |
| Who runs it | The processor, after the loader maps the file | A runtime that loads the file, compiles the IL to machine code when a method first runs (just-in-time compilation), and runs the result |
| Names | Removed by stripping | Kept in the metadata, because the runtime needs them for types, methods and fields |
| Types | Recovered by analysis | Recorded: every method has a typed signature, every local has a type |
| Memory | The program manages it | The runtime manages it (a garbage collector); there is no explicit release |
| Reading it | Disassembler, then reasoning about registers and stack | Metadata browser, IL listing and a decompiler that works well |

Some managed programs are compiled ahead of time to machine code, and some contain native parts that they call. Those cases need the native methods of the earlier modules. This module covers the common case of a pure IL assembly.

### Assemblies and the file container

A .NET program or library is an assembly. On Windows it is a PE file like any other, with a small addition: a pointer to a CLR header, which holds the version of the runtime and the location of the metadata. The file is easy to recognize in triage:

```bash
make
file build/Parcels.dll
python3 22_meta.py build/Parcels.dll
```

```text
build/Parcels.dll: PE32 executable (console) Intel 80386 Mono/.Net assembly, for MS Windows, 3 sections
CLR header: rva 0x2008, size 72
  runtime version 2.5, flags 0x1, entry point token 0x06000006
metadata: signature BSJB, version string v4.0.30319
  stream #~       offset 0x0006c  size 816
  stream #Strings offset 0x0039c  size 892
  stream #US      offset 0x00718  size 184
  stream #GUID    offset 0x007d0  size 16
  stream #Blob    offset 0x007e0  size 332
table rows: Module 1, TypeRef 19, TypeDef 8, Field 8, MethodDef 8, Param 7, MemberRef 19, CustomAttribute 7, Assembly 1, AssemblyRef 2
user strings (#US heap):
  +0x002  ' parcel, '
  +0x016  ' g, cost '
  +0x02a  'NE'
  +0x030  'NW'
  +0x036  'SE'
  +0x03c  'north'
  +0x048  'south'
  +0x054  'east'
  +0x05e  'west'
  +0x068  'northeast'
  +0x07c  'northwest'
  +0x090  'southeast'
  +0x0a4  'unknown'
  +0x0b4  ','
```

The signs of a managed assembly are: `file` says `Mono/.Net assembly`, the import table has a single entry from `mscoree.dll` (`_CorExeMain` for a program), the data directory number 14 is not empty, and the metadata starts with the signature `BSJB` followed by a version string. The script `examples/22_meta.py` reads those structures with a PE library and nothing else.

### Metadata

The metadata is a set of tables and heaps (the streams in the output above). The tables describe what the assembly contains:

| Table | Holds | Rows in the example |
|---|---|---|
| TypeDef | The types defined here: classes, structures, enumerations | 8 |
| MethodDef | The methods defined here, with their signature and the location of their IL | 8 |
| Field | The fields defined here, with their types | 8 |
| TypeRef, MemberRef | Types and members that the assembly uses from other assemblies | 19 and 19 |
| AssemblyRef | The other assemblies that it references | 2 |
| CustomAttribute | Attributes attached to types and methods, many added by the compiler | 7 |

The heaps hold the data that the tables point to. `#Strings` holds names (types, methods, fields) in UTF-8, so a plain `strings` run finds them, as it found `WriteLine`, `AppendFormatted` and `<>c__DisplayClass1_0` in the example. `#US` holds the string literals of the program, the user strings, in UTF-16, which is why the literals of a managed program do not appear in the output of `strings` with its default settings. Use `strings -el` to see UTF-16 text, or read the heap with the script as above. `#Blob` holds signatures and constant data, and `#GUID` holds identifiers.

Three consequences matter for analysis. First, the references to other assemblies show what the program can do: a reference to the networking, file or process libraries is the managed equivalent of an import table, and the list of `MemberRef` rows names the exact methods called. Second, because names are data in `#Strings`, a program that wants to hide its purpose renames its types and methods to meaningless short names; this changes how it reads but not what it does. Third, the compiler adds many things that the author did not write, and their names show it: a name that starts with `<` or contains `>` is generated, for example `<>c__DisplayClass1_0` and `<Surcharges>b__0` in the example, which are the helper class and the method of a lambda.

### The intermediate language

IL is the instruction set of a stack machine, which makes it close to the machine of module 21. Operands are pushed on an evaluation stack, an instruction takes its inputs from the top of the stack and pushes its result, and the instructions have no register operands. Arguments and local variables live in numbered slots and are moved onto the stack and back. The code of the method `Cost` in the example program starts like this (the full listing is produced in the next section):

```text
IL_0000: ldarg.0
IL_0001: ldfld int32 Parcels.Parcel::weightGrams
IL_0006: ldc.i4 499
IL_000b: add
IL_000c: ldc.i4 500
IL_0011: div
IL_0012: stloc.0
```

| Instruction | Effect | Stack after |
|---|---|---|
| `ldarg.0` | Push argument 0, which is `this` in an instance method | this |
| `ldfld ...::weightGrams` | Pop the object, push the value of its field | weightGrams |
| `ldc.i4 499` | Push the 32-bit constant 499 | weightGrams, 499 |
| `add` | Pop two values, push their sum | weightGrams + 499 |
| `ldc.i4 500` | Push 500 | weightGrams + 499, 500 |
| `div` | Pop two values, push the quotient | the quotient |
| `stloc.0` | Pop into local variable 0 | (empty) |

This is the line `int units = (weightGrams + 499) / 500;` of the source. The groups of instructions that you will see most often:

| Group | Instructions | Meaning |
|---|---|---|
| Load and store | `ldarg`, `ldloc`, `stloc`, `ldfld`, `stfld`, `ldc.i4`, `ldstr` | Move arguments, locals, fields, constants and string literals to and from the stack |
| Arithmetic | `add`, `sub`, `mul`, `div`, `rem`, `neg`, `and`, `or`, `xor`, `shl`, `shr` | Work on the top one or two values |
| Branches | `br`, `brtrue`, `brfalse`, `beq`, `bgt`, `blt`, `switch` | Jump, unconditionally or on a condition; targets are IL offsets |
| Calls | `call`, `callvirt`, `newobj`, `ret` | Call a method (the metadata token says which), create an object, return |
| Conversions and arrays | `conv.i8`, `ldelem`, `stelem`, `ldlen` | Change the width of a number, access arrays |

The `.s` suffix (for example `ldloc.s`, `br.s`) means a short form with a one-byte operand, and `ldarg.0`, `ldc.i4.1` and the like are short forms with the operand in the opcode. They do not change the meaning. IL carries types: locals have declared types, and `ldfld` and `call` carry the full name of the field or method. In a build without debug information the locals have no names, only slot numbers.

#### Reading a method

The whole of `Cost` is below, produced by the decompiler's IL mode. Read it with the structure of the source in mind:

```bash
ilspycmd -il -t Parcels.Parcel build/Parcels.dll 2>/dev/null | sed -n '/instance int32 Cost/,/end of method Parcel::Cost/p'
```

```text
        instance int32 Cost () cil managed 
    {
        // Method begins at RVA 0x2068
        // Header size: 12
        // Code size: 85 (0x55)
        .maxstack 2
        .locals init (
            [0] int32,
            [1] int32,
            [2] int32,
            [3] int32,
            [4] valuetype Parcels.Zone,
            [5] int32
        )

        IL_0000: ldarg.0
        IL_0001: ldfld int32 Parcels.Parcel::weightGrams
        IL_0006: ldc.i4 499
        IL_000b: add
        IL_000c: ldc.i4 500
        IL_0011: div
        IL_0012: stloc.0
        IL_0013: ldarg.0
        IL_0014: ldfld valuetype Parcels.Zone Parcels.Parcel::zone
        IL_0019: stloc.s 4
        IL_001b: ldloc.s 4
        IL_001d: ldc.i4.1
        IL_001e: beq.s IL_0027

        IL_0020: ldloc.s 4
        IL_0022: ldc.i4.2
        IL_0023: beq.s IL_002b

        IL_0025: br.s IL_002f

        IL_0027: ldc.i4.3
        IL_0028: stloc.3
        IL_0029: br.s IL_0032

        IL_002b: ldc.i4.5
        IL_002c: stloc.3
        IL_002d: br.s IL_0032

        IL_002f: ldc.i4.s 9
        IL_0031: stloc.3

        IL_0032: ldloc.3
        IL_0033: stloc.1
        IL_0034: ldc.i4.0
        IL_0035: stloc.2
        IL_0036: ldc.i4.0
        IL_0037: stloc.s 5
        IL_0039: br.s IL_004e
        // loop start (head: IL_004e)
            IL_003b: ldloc.2
            IL_003c: ldloc.1
            IL_003d: add
            IL_003e: stloc.2
            IL_003f: ldloc.s 5
            IL_0041: ldc.i4.4
            IL_0042: blt.s IL_0048

            IL_0044: ldloc.2
            IL_0045: ldc.i4.1
            IL_0046: add
            IL_0047: stloc.2

            IL_0048: ldloc.s 5
            IL_004a: ldc.i4.1
            IL_004b: add
            IL_004c: stloc.s 5

            IL_004e: ldloc.s 5
            IL_0050: ldloc.0
            IL_0051: blt.s IL_003b
        // end loop

        IL_0053: ldloc.2
        IL_0054: ret
    } // end of method Parcel::Cost
```

The structure is visible even without the source. Local 0 receives the unit count. The two comparisons of the zone value with 1 and 2 select the constants 3 and 5, and the fall-through case stores 9: that is a three-way choice with the zone values as keys, stored in local 3 and copied to local 1, the rate. The jump at IL_0039 goes to the loop test at IL_004e, which compares local 5 with local 0 and jumps back to IL_003b while the counter is smaller. The body adds the rate (local 1) to the total (local 2), and the `blt.s` at IL_0042 skips the extra increment while the counter is below 4. Local 2 is returned. The slots do not say which local is called what; the roles come from how they are used, the same way you named registers in native code.

The `.maxstack 2` line states the deepest the evaluation stack gets, which is a check on your reading: no sequence in the method leaves more than two values on the stack.

### Decompiling with ILSpy

A decompiler turns IL back into a high-level language, usually C#. The lab image has `ilspycmd`, the command-line form of ILSpy; the graphical ILSpy gives the same results with a tree of types and a click-through to the definitions. Useful commands:

| Command | Output |
|---|---|
| `ilspycmd Assembly.dll` | The whole assembly as C# on standard output |
| `ilspycmd -l c Assembly.dll` | A list of the classes |
| `ilspycmd -t Namespace.Type Assembly.dll` | One type |
| `ilspycmd -il -t Namespace.Type Assembly.dll` | The IL of one type |
| `ilspycmd -p -o dir Assembly.dll` | A whole project, to read in an editor |
| `ilspycmd -usepdb Assembly.dll` | Use the names of local variables from the debug file next to the assembly |

The classes of the example program:

```bash
ilspycmd -l c build/Parcels.dll 2>/dev/null
```

```text
Class <Module>
Class Parcels.Parcel
Class Parcels.Routes
Class Parcels.Program
Class <PrivateImplementationDetails>
Class Parcels.Routes.<>c__DisplayClass1_0
```

Three of the six are generated by the compiler: `<Module>` holds module-level data, `<PrivateImplementationDetails>` holds constant data such as the array initializer, and the display class holds the captured variable of the lambda. The decompiled method `Cost`:

```bash
ilspycmd -t Parcels.Parcel build/Parcels.dll 2>/dev/null | sed -n '/public int Cost/,/^\t}/p'
```

```text
    public int Cost()
    {
        int num = (weightGrams + 499) / 500;
        int num2 = zone switch
        {
            Zone.Local => 3, 
            Zone.Regional => 5, 
            _ => 9, 
        };
        int num3 = 0;
        for (int i = 0; i < num; i++)
        {
            num3 += num2;
            if (i >= 4)
            {
                num3++;
            }
        }
        return num3;
    }
```

This is the method of the source with three differences: the locals are called `num`, `num2`, `num3`, the comment is gone, and `total += 1` has become `num3++`. The structure, including the choice by zone and the loop, is the same. The decompiler found the loop and the choice from the branches shown in the IL above.

### What a decompiler recovers and what it loses

The comparison of the source with the decompiled program is the best way to learn what to trust. From the example:

| Element | In the source | In the decompiled output | Why |
|---|---|---|---|
| Type, method and field names | `Parcel`, `Cost`, `weightGrams` | Identical | They are metadata |
| Signatures and types | `int Cost()` | Identical | They are metadata |
| Control flow | `for`, `if`, a `switch` expression | Recovered as the same constructs | The branches map back to them |
| Local variable names | `units`, `rate`, `total` | `num`, `num2`, `num3` | Names of locals are in the debug file, not in the assembly |
| Comments | Present | Absent | They never reach the compiled file |
| `foreach` over an array | `foreach (Parcel p in parcels)` | `for (int i = 0; i < array.Length; i++)` with `array[i]` | The compiler already turned it into an indexed loop |
| `total += 1` | As written | `num3++` | The compiler emits the same IL; the decompiler picks one form |
| String switch | `switch (code) { case "N": ... }` | A `switch` expression with the same cases | The compiler tested the length and then the characters; the decompiler recognizes that pattern and restores the strings |
| Lambda | A lambda that captures `limit` | A lambda `(int w) => ...` | The decompiler hides the display class; the IL shows it |

When the same program is built with a debug file, the decompiler can take the names of the locals from it:

```bash
ilspycmd -usepdb -t Parcels.Parcel build/debug/Parcels.dll 2>/dev/null | sed -n '/public int Cost/,/^\t}/p'
```

```text
    public int Cost()
    {
        int units = (weightGrams + 499) / 500;
        int rate = zone switch
        {
            Zone.Local => 3, 
            Zone.Regional => 5, 
            _ => 9, 
        };
        int total = 0;
        for (int i = 0; i < units; i++)
        {
            total += rate;
            if (i >= 4)
            {
                total++;
            }
        }
        return total;
    }
```

The names `units`, `rate` and `total` return. Analysts almost never have the debug file of a sample, so expect the generic names, and name the variables yourself as you work out what they do.

One more detail comes from the user strings in the first listing. The switch has seven cases, and the strings `N`, `S`, `E` and `W` do not appear in the `#US` heap, although `NE`, `NW` and `SE` do. The compiler compares single-character strings by their length and their character instead of storing a literal, and the decompiler turns the comparison back into the literal in its output. The decompiled source therefore contains literals that the string heap does not. A search for strings in the file is a good first step and not a complete list.

### Reading obfuscated managed code

The methods of module 20 apply with changes. Managed obfuscators commonly rename types, methods and fields to short meaningless names, encrypt string literals so that they are decoded by a method at run time, flatten control flow, and add junk. Compare these to the native cases:

- **Renamed symbols.** The decompiler output has names such as `a`, `b`, `c` or unprintable characters. The structure and the types are still there. Rename as you learn: ILSpy lets you search by signature, and the references from `MemberRef` still tell you what the program calls in the framework.
- **Encrypted strings.** The `#US` heap has few literals and the code has many calls to one small method that takes an integer or a byte array and returns a string. This is the decoder of module 20, written in IL. You can read the method and reimplement it, or run it: the assembly is a program, and a small script or a test harness in the isolated lab can call the decoder with the arguments you read from the call sites.
- **Flattened control flow.** The same dispatcher-and-state structure as in module 20, readable in IL as a `switch` over a local variable inside a loop.
- **Calls through reflection.** A program can call a method by a name that it builds at run time. The `MemberRef` list will not show it; look for the framework calls that find a type or a method by name.

Because the runtime can recover the IL of any method, nothing about the logic is hidden from a determined reader; the work is in removing the noise. Run an unknown assembly only in the isolated lab of module 19.

### A working order

1. **Triage.** Confirm that the file is managed. List the classes, the referenced assemblies and the `MemberRef` rows. Read the strings from both the `#Strings` and the `#US` heaps.
2. **Decompile.** Read the C# for the entry point first (the entry point token in the CLR header names the method), then follow the calls.
3. **Check names.** If the names are meaningless, rename as you go and note what each type does.
4. **Check the output against the IL** wherever the decompiled code looks odd, or when a detail decides the answer.
5. **Look for generated code.** Names with `<` and `>` are compiler helpers: lambdas, iterators, asynchronous methods. Read them as the code they implement.
6. **Handle protection.** Find the string decoder, reimplement or call it, and note the result in the report.

## Try it

Run the lab commands from `modules/22-managed-code-dotnet/examples`.

1. Build with `make` and run `dotnet build/Parcels.dll`. Run `22_meta.py` on it and compare the number of rows of `MethodDef` with the number of methods in the source (constructors and the compiler-generated method count).
2. Run `strings -n 5` and `strings -el -n 4` on the assembly. Which literals of the program appear in which output? Why does `N` not appear in either?
3. Decompile the whole assembly. Write down three differences from the source in `22_parcels/Program.cs`.
4. Print the IL of `Routes.Region`. Find the call that reads the length of the string and the calls that read single characters. Explain in a sentence how the compiler turned the string switch into comparisons.
5. Print the IL of `Routes.Surcharges`. Find where the helper class is created and where the captured variable is stored. Which instruction loads the address of the helper's method into the delegate?
6. Decompile `build/debug/Parcels.dll` with and without `-usepdb` and compare the names of the locals.
7. Change `Cost` in the source so that the extra cent starts after the sixth unit, rebuild, and find the changed constant in the IL.

## Common pitfalls

- **Searching for literals with an ASCII search.** The literals of a managed program are UTF-16 in a separate heap. Use `strings -el` or read the heap.
- **Trusting local names.** Without a debug file the names are generated. A name like `num2` says nothing about the role of the variable.
- **Forgetting that the compiler writes code.** A class with `<` in its name is generated. A reading that treats it as the author's design will be confused.
- **Reading only the decompiled code.** The decompiler makes choices. When a result looks wrong or a detail matters, read the IL for that method.
- **Assuming names mean what they say.** Names are free text. A method called `Verify` can do anything.
- **Running an unknown assembly outside the lab.** Reading an assembly never runs it. Running it does, and a managed program has the same access as a native one.

## Summary

- Managed code is IL plus metadata, run by a runtime that compiles it when needed; the file keeps the names, types and structure of the program.
- A managed assembly is a PE file with a CLR header, a single `mscoree.dll` import and metadata tables and heaps; literals are UTF-16 in `#US` and names are in `#Strings`.
- IL is a stack machine with typed locals and fields; it reads like the bytecode of module 21, and the roles of locals come from how they are used.
- A decompiler such as ILSpy returns C# that keeps types, signatures and control flow, and loses comments and, without a debug file, the names of locals.
- Check decompiled output against the IL when a detail matters, and treat names with `<` and `>` as compiler-generated code.
- Obfuscated managed code is read by the methods of module 20: rename, find the string decoder, and flatten back the control flow.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 23, decompilers and their limits.

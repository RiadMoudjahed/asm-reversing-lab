[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 14. Shape shifter

Level 4. Time: about 4 hours. Architecture: x86-64 Linux (C++). Modules: [15. Recovering data](../../modules/15-recovering-data/README.md)

## Scenario

A C++ program, `gadgets`, builds objects of several related classes and calls their virtual functions. Its symbols and its run-time type information were removed, and the source is lost. A plug-in written in assembly has to create objects of these classes itself and call their virtual functions, so it must reproduce their exact layout and use their vtables correctly. You first recover the class hierarchy. Then you write the plug-in.

## Objectives

- Find the classes in a stripped C++ binary from the constructors that store a vtable pointer.
- Recover the object layout, the vtable contents and the inheritance relations.
- Build objects by hand and call virtual functions through a vtable from assembly.

## Part 1: Reverse

The binary is `bin/gadgets`. It reads lines `kind a b c m`, creates one object of the class selected by `kind`, and prints three numbers. A function with a `switch` on `kind` creates the objects; it calls `operator new` for each class and stores the vtable pointer and the fields.

```bash
printf '2 7 3 800 10\n' | bin/gadgets
objdump -d -M intel bin/gadgets
objdump -s -j .rodata bin/gadgets
```

None of the classes has a virtual destructor, and the program was built without run-time type information, so each vtable starts with two zero words and then holds only the addresses of the virtual functions. The vtable pointer stored in an object points just past those two words.

Find these six values:

1. `N`: the number of classes, which is the number of different vtables, in decimal.
2. `S`: the number of virtual functions in each vtable, in decimal.
3. `Z`: the size in bytes of the objects of the class whose objects are the largest, in decimal.
4. `P`: the offset in bytes of the last field of the objects of that largest class, in decimal.
5. `E`: the number of pairs of classes that hold the same function address in the same slot, counting each pair and each slot once, in decimal.
6. `F`: the number of different function addresses in all the vtables together, in decimal.

```text
ARL{shape_<N>_<S>_<Z>_<P>_<E>_<F>}
```

For example, the values 3, 2, 32, 24, 1 and 5 would give `ARL{shape_3_2_32_24_1_5}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement two functions in the System V calling convention:

```c
void gadget_init(void *mem, int kind, int a, int b, int c);
int  gadget_call(const void *obj, int slot, int arg);

/* Provided by the tests. */
const void *gadget_vptr(int kind);
```

- `gadget_init` builds an object of class `kind` in the buffer `mem`. `kind` is the value that the object-creating function of `bin/gadgets` switches on. The first word of the object is `gadget_vptr(kind)`. The values `a`, `b` and `c` fill the data fields of the class in the order they appear in the object, starting with the lowest offset; values for which the class has no field are ignored.
- `gadget_call` calls a virtual function of an object through its own vtable: slot 0 is the first function in the vtable, slot 1 the next and slot 2 the last. It passes the object as the first argument and `arg` as the second, and returns the result.

The tests build objects with your `gadget_init`, then compare what the library's own virtual calls return for them with the results of `bin/gadgets`.

## Rules

- Use the vtable: `gadget_call` must load the vtable pointer from the object and call through it.
- Write only the bytes that belong to the object; the tests check that nothing after the first 32 bytes of the buffer changes.
- Preserve the callee-saved registers `rbx`, `rbp` and `r12` to `r15`.
- Do not call anything except `gadget_vptr`.

## Check your work

```bash
make check EX=14 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. Each Part 2 test prints three results followed by `ok` or a list of what went wrong: `vptr`, `call`, `canary` or `regs`.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.

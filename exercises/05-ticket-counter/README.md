[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 05. Ticket counter

Level 2. Time: about 2 hours. Architecture: x86 (32-bit) Linux. Modules: [01. Data representation](../../modules/01-data-representation/README.md), [03. Your first program](../../modules/03-first-program/README.md), [05. Arithmetic and logic](../../modules/05-arithmetic-and-logic/README.md), [06. Conditions, branches and loops](../../modules/06-conditions-and-loops/README.md)

## Scenario

A ticket office adds up the sales of two counters with a legacy 32-bit tool called `ticket_counter`. The numbers are typed as text and can be longer than a 32-bit register can hold: a 12-digit number is larger than 2^32. The office is moving to a new system and needs the tool's behavior documented exactly, including what it does with bad input, before it can be replaced.

## Objectives

- Work out the rules of a program from its disassembly and from experiments.
- Read the ASCII digit test and the exit status in 32-bit code.
- Add numbers given as text, digit by digit, with a carry, without converting them to integers.

## Part 1: Reverse

The binary is `bin/ticket_counter`. It reads two lines from standard input and prints a result.

```bash
printf '12\n34\n' | bin/ticket_counter ; echo "exit status: $?"
objdump -d -M intel bin/ticket_counter
strace bin/ticket_counter
```

Experiment with the inputs and read the code to answer these questions:

1. How many digits is the longest input the program accepts?
2. What exit status does it return when an input is invalid?
3. What exit status does it return when the sum is too large to print?
4. What does it print for the inputs `000` and `0000`? (Check your answer by running it.)

Build the flag from your answers:

```text
ARL{ticket_<longest input>_<status for invalid input>_<status for a sum that is too large>}
```

For example, if the longest input were 8 digits, the first status 3 and the second status 4, the flag would be `ARL{ticket_8_3_4}`.

## Part 2: Assemble

Edit `starter/solution.asm` to write a complete 32-bit program that behaves exactly like `bin/ticket_counter`: the same text on standard output and the same exit status for every input. Build and try it from the `starter` folder:

```bash
make run
```

The checker runs your program on 80 inputs: ordinary sums, carries that run through every digit, leading zeros, inputs at and just beyond every limit, and malformed input of several kinds.

## Rules

- Write 32-bit code that uses the `int 0x80` interface and no C library.
- Do not convert the numbers to a single register. A 12-digit number does not fit in 32 bits, so add digit by digit.
- Your program must not run `ticket_counter`.

## Check your work

```bash
make check EX=05 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.

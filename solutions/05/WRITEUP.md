[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 05. Ticket counter

The flag is `ARL{ticket_12_1_2}`. The program accepts at most 12 digits per number, returns exit status 1 for invalid input, and returns exit status 2 when the sum has 13 digits. It adds the numbers as text, one digit at a time, from the right. The reference solution does the same.

## Part 1: Reverse

### Experiments

```text
$ printf '12\n34\n' | bin/ticket_counter ; echo $?
46
0
$ printf '000\n0000\n' | bin/ticket_counter ; echo $?
0
0
$ printf '12a\n5\n' | bin/ticket_counter ; echo $?
ERR
1
$ printf '999999999999\n1\n' | bin/ticket_counter ; echo $?
OVERFLOW
2
$ printf '1234567890123\n1\n' | bin/ticket_counter ; echo $?
ERR
1
```

Leading zeros are removed (`000` plus `0000` prints `0`). A 13-digit input is rejected as invalid, which suggests a length limit on each number. A sum of two 12-digit numbers that needs 13 digits prints a different message and a different status. Experiments show the behavior, and the disassembly shows the exact limits.

### Read the checking routine

```text
parse_line:
  mov    edi,esi
  xor    ecx,ecx
.scan:
  movzx  eax,BYTE PTR [esi]
  cmp    al,0xa                 ; newline: the line ends
  je     .end
  sub    al,0x30                ; '0' is 0x30
  cmp    al,0x9
  ja     fail                   ; not '0'..'9'
  inc    esi
  inc    ecx                    ; count the digits
  jmp    .scan
.end:
  inc    esi
  test   ecx,ecx
  je     fail                   ; empty line
  cmp    ecx,0xc                ; 12
  ja     fail                   ; too long
  ret
```

- **The digit test.** `sub al, 0x30` turns `0` to `9` into 0 to 9, and anything else into a number above 9 (because the subtraction wraps around for characters below `0`). `cmp al, 9` with `ja` sends all of those to `fail`.
- **The length limit.** `cmp ecx, 0xc` with `ja fail`: 12 digits is the longest accepted input.
- **A missing newline.** At the end of the input the buffer holds a zero byte. It is not a newline and not a digit, so the scan ends in `fail`.

### The exit statuses

Both ending paths write their text and then load `ebx` before the exit call (`eax` = 1, `int 0x80`):

```text
overflow:  ...  mov ebx,0x2   -> exit status 2
fail:      ...  mov ebx,0x1   -> exit status 1
```

The status of a successful run is 0, with `ebx` cleared. The answers are 12, 1 and 2, and the flag is `ARL{ticket_12_1_2}`.

## Part 2: Assemble

The reference solution is `solution.asm`. It works in four steps:

1. **Read** up to 63 bytes from standard input with `read` and put a zero byte after them.
2. **Check** each of the two lines with a routine like `parse_line`, recording the start and the length of each number.
3. **Add** from the right in 13 positions. For each position, take the digit of each number (0 once a number runs out), add them and the carry, subtract 10 and set the carry if the sum is 10 or more, and store the character into the buffer.
4. **Print** the digits without leading zeros, or the overflow text if the first of the 13 positions is not `0`.

The core of the addition is the digit arithmetic:

```nasm-fragment
    add     eax, ebx                    ; digit of a + digit of b
    add     eax, edx                    ; plus the carry from the previous position
    xor     edx, edx
    cmp     eax, 10
    jb      .store
    sub     eax, 10                     ; 10 or more: keep the low digit
    mov     edx, 1                      ; and carry 1 into the next position
```

## Mistakes that cost time

- **Converting the numbers to integers.** A 12-digit value exceeds 32 bits. Add the text digit by digit.
- **A limit that is one too high or one too low.** Accepting 13 digits fails 18 of the 80 cases; the tests use inputs of exactly 12 and exactly 13 digits.
- **Not removing leading zeros.** The program then prints results such as `0004294967296`; 53 of the 80 cases fail.
- **The wrong exit status.** An error that exits with 3 instead of 1 fails 21 cases even though the text is right.
- **Accepting input without a final newline.** The original treats it as an error; the test inputs include it.
- **An empty program.** The linker produces a file with no code, and the checker reports that it could not start it.

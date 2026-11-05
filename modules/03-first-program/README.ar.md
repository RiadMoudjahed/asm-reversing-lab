<!-- source: README.md @ sha256:2dd2136139ab -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

<div dir="rtl">

# 03. برنامجك الأول

الوقت: نحو ساعتين. المتطلبات السابقة: [01. تمثيل البيانات](../01-data-representation/README.ar.md)، [02. المعالج وسجلاته](../02-cpu-and-registers/README.ar.md)

## لماذا هذا مهم

استدعاء النظام هو الطريقة الوحيدة التي يستطيع بها برنامج بلا مكتبات أن يفعل أي شيء مرئي: الطباعة، قراءة ملف، الإنهاء. وحين تستطيع إصدار استدعاءات النظام من لغة التجميع، تستطيع كتابة برامج حقيقية، وتستطيع كذلك قراءة الاستدعاءات نفسها في أي ملف تنفيذي تحلله. يبدأ المهندسون العكسيون كثيراً من تحقيقاتهم بسؤال: ما استدعاءات النظام التي ينفذها هذا البرنامج؟

## المفاهيم

### تشريح العبارة

لكل سطر في شيفرة NASM المصدرية الأجزاء الاختيارية نفسها:

</div>

```text
label:    mnemonic   operands        ; comment
```

<div dir="rtl">

- **الوسم (label)** يسمّي عنواناً. `_start:` و`message:` وسمان.
- **الرمز الاختصاري (mnemonic)** يسمّي تعليمة، مثل `mov` أو `syscall`.
- **المعاملات (operands)** هي القيم التي تعمل عليها التعليمة، مفصولة بفواصل. تأتي الوجهة أولاً: `mov eax, 1` تضع 1 في `eax`.
- **التعليق** يبدأ بـ `;` ويمتد إلى نهاية السطر. يتجاهله المجمّع.

التوجيهات (directives) مثل `section` و`global` و`db` و`equ` تبدو مشابهة لكنها تعليمات إلى المجمّع وليست إلى المعالج. هي لا تنتج شيفرة آلة بنفسها.

### الأقسام

يُخزَّن البرنامج في أقسام لها أغراض وصلاحيات مختلفة:

| القسم | ما يحتويه | الصلاحيات | هل يشغل مساحة في الملف |
|---|---|---|---|
| `.text` | الشيفرة | قراءة، تنفيذ | نعم |
| `.rodata` | الثوابت | قراءة | نعم |
| `.data` | المتغيرات المهيأة | قراءة، كتابة | نعم |
| `.bss` | المتغيرات التي تبدأ بصفر | قراءة، كتابة | لا |

لا يشغل `.bss` مساحة في الملف لأنه يحمل حجماً فقط؛ والمحمّل (loader) هو من يوفر الذاكرة المصفّرة. يمكنك رؤية هذه الخصائص في جدول الأقسام للبرنامج المثال:

</div>

```text
$ readelf -S -W build/03_data_bss
  [ 1] .text    PROGBITS  0000000000401000 001000 000039 00  AX  0   0 16
  [ 2] .rodata  PROGBITS  0000000000402000 002000 000001 00   A  0   0  4
  [ 3] .data    PROGBITS  0000000000403004 002004 000001 00  WA  0   0  4
  [ 4] .bss     NOBITS    0000000000403008 002005 000008 00  WA  0   0  4
```

<div dir="rtl">

يعرض العمود `Flg` الرموز `A` (محجوز)، و`X` (قابل للتنفيذ)، و`W` (قابل للكتابة). `PROGBITS` تعني أن للقسم بايتات في الملف؛ و`NOBITS` تعني أنه لا بايتات له.

### تعريف البيانات

| التوجيه | المعنى |
|---|---|
| `db`, `dw`, `dd`, `dq` | يضع بايتات أو كلمات (2 بايت) أو كلمات مزدوجة (4) أو كلمات رباعية (8) في الملف |
| `resb`, `resw`, `resd`, `resq` | يحجز مساحة دون وضع بايتات (يُستخدم في `.bss`) |
| `name equ expression` | يعرّف ثابتاً؛ ولا يستهلك ذاكرة |
| `$` | عنوان الموضع الحالي |

النمط `message_len equ $ - message`، حين يُكتب مباشرة بعد سلسلة نصية، يجعل المجمّع يحسب طول السلسلة. استخدمه بدل العدّ اليدوي. السلاسل في لغة التجميع بايتات عادية. ليس لها محرف إنهاء ما لم تضفه بنفسك، فالرقم `10` (سطر جديد) مجرد بايت آخر تضعه بنفسك.

### نقطة الدخول

يبدأ البرنامج عند الوسم `_start`. يجعل التوجيه `global _start` الوسم مرئياً للرابط، فيسجّل عنوانه نقطةَ دخول.

### التجميع والربط

أداتان تحوّلان الشيفرة المصدرية إلى برنامج:

</div>

```bash
nasm -f elf64 prog.asm -o prog.o      # assemble: source to object file
ld prog.o -o prog                     # link: object file to executable
```

<div dir="rtl">

في الشيفرة ذات 32 بت، غيّر الأمرين معاً:

</div>

```bash
nasm -f elf32 prog.asm -o prog.o
ld -m elf_i386 prog.o -o prog
```

<div dir="rtl">

### استدعاءات النظام

يطلب استدعاء النظام من نواة نظام التشغيل أن تفعل شيئاً نيابةً عنك. يعتمد الاصطلاح على Linux على الوضع:

| | 64 بت | 32 بت |
|---|---|---|
| التعليمة | `syscall` | `int 0x80` |
| رقم الاستدعاء | `rax` | `eax` |
| الوسائط من 1 إلى 6 | `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9` | `ebx`, `ecx`, `edx`, `esi`, `edi`, `ebp` |
| النتيجة | `rax` | `eax` |

أرقام الاستدعاءات تختلف بين الوضعين أيضاً. `write` هو 1 في الشيفرة ذات 64 بت و4 في الشيفرة ذات 32 بت. و`exit` هو 60 و1. الجدول الكامل في [ورقة مرجعية لاستدعاءات نظام Linux](../../cheatsheets/syscalls-linux.ar.md).

`write(fd, buffer, length)` ترسل `length` بايتاً بدءاً من `buffer` إلى واصف الملف `fd`. الواصف 1 هو الخرج القياسي و2 هو خرج الأخطاء القياسي. وتعيد عدد البايتات المكتوبة.

### البرنامج الكامل

هذا هو [03_hello64.asm](examples/03_hello64.asm). كل سطر معلَّق لأن هذا برنامجك الأول.

</div>

```nasm
bits 64
default rel

SYS_WRITE   equ 1               ; system call numbers (64-bit table)
SYS_EXIT    equ 60
STDOUT      equ 1               ; file descriptor of standard output

section .rodata
message:    db "Hello, assembly!", 10       ; 10 is the newline byte
message_len equ $ - message                 ; bytes from 'message' to here

section .text
global _start                   ; make the entry point visible to the linker

_start:
    mov     eax, SYS_WRITE      ; rax = call number
    mov     edi, STDOUT         ; rdi = argument 1: file descriptor
    lea     rsi, [message]      ; rsi = argument 2: address of the text
    mov     edx, message_len    ; rdx = argument 3: number of bytes
    syscall                     ; enter the kernel: write(1, message, len)

    mov     eax, SYS_EXIT       ; rax = call number
    xor     edi, edi            ; rdi = argument 1: exit status 0
    syscall                     ; exit(0)
```

<div dir="rtl">

تحمّل `lea rsi, [message]` عنوان `message`؛ ولا تقرأ النص. تغطي الوحدة 04 التعليمة `lea` والعنونة بالكامل. و`xor edi, edi` هي الطريقة المعتادة لتصفير سجل.

ينفذ [03_hello32.asm](examples/03_hello32.asm) الشيء نفسه في الشيفرة ذات 32 بت باستخدام `int 0x80`. ويعرض [03_data_bss.asm](examples/03_data_bss.asm) الأقسام الأربعة في برنامج واحد.

### منظور الهندسة العكسية

تحفظ لغة التجميع بنيتك لكنها لا تحفظ أسماءك. افتح البرنامج الذي بنيته للتو:

</div>

```text
$ objdump -d -M intel build/03_hello64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <message>
  401011:  mov  edx,0x11
  401016:  syscall
  401018:  mov  eax,0x3c
  40101d:  xor  edi,edi
  40101f:  syscall
```

<div dir="rtl">

ما بقي: التعليمات والوسمان `_start` و`message` (يحتفظ بهما جدول الرموز). وما لم يبقَ: الثوابت `SYS_WRITE` و`STDOUT` و`message_len`، التي صارت الأرقام 1 و1 و0x11 (17)، وكل التعليقات. هذا ما سترى في البرامج التي بلا مصدر: أرقام حيث كان المؤلف قد استخدم أسماء. إن التعرف على أن `mov eax, 1` قبل `syscall` تعني `write` هو أول مهارة عكسية في هذه الدورة.

## جرّب بنفسك

ابنِ الأمثلة وشغّلها:

</div>

```bash
cd modules/03-first-program/examples
make run
```

<div dir="rtl">

الناتج المتوقع:

</div>

```text
== 03_hello64
Hello, assembly!
== 03_data_bss
A
== 03_hello32
Hello, assembly!
```

<div dir="rtl">

راقب الآن تغيّر السجلات في مصحح الأخطاء:

</div>

```bash
gdb -q build/03_hello64
```

<div dir="rtl">

عند موجّه `gef>` اكتب هذه الأوامر واحداً تلو الآخر:

</div>

```text
break _start
run
stepi 4
info registers rax rdi rdx rip
x/s $rsi
```

<div dir="rtl">

يطبع GEF لوحة كبيرة بعد كل توقف. ابحث عن أسطر السجلات. بعد أربعة أوامر `stepi` تكون التعليمات الأربع الأولى قد نُفِّذت، وترى:

</div>

```text
rax            0x1                 0x1
rdi            0x1                 0x1
rdx            0x11                0x11
rip            0x401016            0x401016 <_start+22>
0x402000 <message>: "Hello, assembly!\n,"
```

<div dir="rtl">

تحمل السجلات الوسائط الثلاثة ورقم الاستدعاء، ويشير `rip` إلى تعليمة `syscall`. تظهر في السلسلة فاصلة زائدة `,` بعد السطر الجديد: يطبع `x/s` حتى يجد بايتاً صفرياً، و`message` لا تحتوي واحداً، فيقرأ بايتاً واحداً مما يليها. نفّذ الآن `stepi` مرة أخرى. يظهر النص على شاشتك وتتغير السجلات:

- `rax` صار الآن `0x11`: نتيجة `write`، أي 17 بايتاً مكتوباً.
- يحمل `rcx` القيمة `0x401018`، عنوان التعليمة التالية، ويحمل `r11` القيمة `0x302`. تكتب تعليمة `syscall` فوق كليهما. لا تحتفظ أبداً بقيمة في `rcx` أو `r11` عبر `syscall`.

أخيراً، شاهد البرنامج نفسه من الخارج:

</div>

```bash
strace ./build/03_hello64
```

```text
execve("./build/03_hello64", ["./build/03_hello64"], ...) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

<div dir="rtl">

يعرض تشغيل `strace ./build/03_hello32` الاستدعاءات نفسها ويضيف السطر `[ Process PID=... runs in 32 bit mode. ]`.

## أخطاء شائعة

- **نسيان `global _start`.** يحذّر الرابط بالعبارة `cannot find entry symbol _start; defaulting to ...` ويختار بداية `.text`. قد يعمل البرنامج رغم ذلك، بالصدفة، ثم يتعطل حين تضيف شيفرة قبل `_start`. عامل التحذير كخطأ.
- **خلط الصيغ.** استخدام `nasm -f elf32` مع `ld` العادي ينتج خطأ ربط أو برنامجاً خاطئاً. استخدم `-f elf32` مع `ld -m elf_i386`، و`-f elf64` مع `ld` العادي.
- **نسيان `exit`.** بعد آخر تعليمة يُنفِّذ المعالج ما يليها من بايتات وينتهي البرنامج بخطأ تجزئة (segmentation fault).
- **استخدام `int 0x80` في شيفرة 64 بت.** ما زال يعمل على Linux، لكنه يستخدم جدول الاستدعاءات ذا 32 بت ووسائط 32 بت، فتفشل العناوين فوق 4 غيغابايت. في شيفرة 64 بت استخدم `syscall`.
- **خطأ في عدّ سلسلة نصية.** السطر الجديد الذي تضيفه بـ `10` هو بايت إضافي. استخدم `$ - label`.
- **افتراض أن السلاسل تنتهي بصفر.** هي لا تنتهي. مرّر الطول إلى `write`، وضع `0` بنفسك حين تتوقعه روتين لاحق.

## الخلاصة

- العبارة هي `label: mnemonic operands ; comment`؛ والتوجيهات تخاطب المجمّع ولا تنتج شيفرة بنفسها.
- تقع الشيفرة في `.text`، والثوابت في `.rodata`، والمتغيرات في `.data` و`.bss`.
- يستخدم Linux ذو 64 بت `syscall` مع الرقم في `rax` والوسائط في `rdi` و`rsi` و`rdx`؛ ويستخدم ذو 32 بت `int 0x80` مع `eax` و`ebx` و`ecx` و`edx`.
- تختفي الثوابت والتعليقات عند التجميع؛ وتبقى الأرقام.
- يحسب `$ - label` الطول؛ ويجب أن ينتهي كل برنامج بـ `exit`.

## الخطوات التالية

- أجرِ [الاختبار القصير](quiz.ar.md).
- تابع مع الوحدة 04، نقل البيانات وأنماط العنونة.
- التمارين التي تستخدم هذه الوحدة: [التمرين 01](../../exercises/01-signal-flare/README.ar.md)، [التمرين 02](../../exercises/02-two-worlds/README.ar.md)، [التمرين 05](../../exercises/05-ticket-counter/README.ar.md).

</div>

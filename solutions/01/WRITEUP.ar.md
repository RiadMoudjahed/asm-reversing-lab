<!-- source: WRITEUP.md @ sha256:312a383646e6 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

<div dir="rtl">

# شرح الحل: التمرين 01. إشارة الإنذار

العلم هو `ARL{flare_42_23_7}`. يكتب البرنامج 42 بايتاً على الخرج القياسي و23 بايتاً على خرج الأخطاء القياسي، وينتهي بحالة خروج 7. يبيّن هذا الشرح كيف تصل إلى ذلك وكيف تعيد بناءه.

## الجزء 1: Reverse

### تحديد الملف

</div>

```text
$ file bin/signal_flare
bin/signal_flare: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

<div dir="rtl">

"statically linked" تعني أنه لا تُحمَّل أي مكتبات. و"not stripped" تعني أن أسماء الرموز باقية، ولهذا يظهر `_start` في التفكيك.

### مراقبة استدعاءات النظام

</div>

```text
$ strace ./bin/signal_flare
execve("./bin/signal_flare", ["./bin/signal_flare"], ...) = 0
write(1, "FLARE LAUNCHED\nCOLOR: RED\nALTITU"..., 42) = 42
write(2, "[flare] burn time 30 s\n", 23[flare] burn time 30 s
) = 23
exit(7)                                 = ?
+++ exited with 7 +++
```

<div dir="rtl">

يطبع `strace` أول 32 بايتاً من كل مخزن مؤقت، لذلك تُقتطع الرسالة الأولى، لكن عدد البايتات (الوسيط الأخير) والنتيجة كاملان. تظهر الرسالة الثانية داخل سطر strace لأن خرج الأخطاء القياسي وخرج strace نفسه يشتركان في الطرفية. هذا طبيعي وليس خللاً في البرنامج.

الإجابات الثلاث ظاهرة بالفعل: 42 بايتاً إلى الواصف 1، و23 بايتاً إلى الواصف 2، وحالة الخروج 7.

### التأكد في التفكيك

يبيّن `strace` ما حدث في تشغيل واحد. أما التفكيك فيبيّن ما تفعله الشيفرة، وهذا مهم حين يتصرف برنامج بشكل مختلف مع مدخلات أخرى.

</div>

```text
$ objdump -d -M intel bin/signal_flare
0000000000401000 <_start>:
  401000:  b8 01 00 00 00        mov    eax,0x1
  401005:  bf 01 00 00 00        mov    edi,0x1
  40100a:  48 8d 35 ef 0f 00 00  lea    rsi,[rip+0xfef]        # 402000 <report>
  401011:  ba 2a 00 00 00        mov    edx,0x2a
  401016:  0f 05                 syscall
  401018:  b8 01 00 00 00        mov    eax,0x1
  40101d:  bf 02 00 00 00        mov    edi,0x2
  401022:  48 8d 35 01 10 00 00  lea    rsi,[rip+0x1001]       # 40202a <notice>
  401029:  ba 17 00 00 00        mov    edx,0x17
  40102e:  0f 05                 syscall
  401030:  b8 3c 00 00 00        mov    eax,0x3c
  401035:  bf 07 00 00 00        mov    edi,0x7
  40103a:  0f 05                 syscall
```

<div dir="rtl">

قراءتها كثلاثة استدعاءات:

| الاستدعاء | `eax` | `edi` | `rsi` | `edx` | المعنى |
|---|---|---|---|---|---|
| 1 | 0x1 | 0x1 | عنوان `report` | 0x2a = 42 | `write` لـ 42 بايتاً إلى الخرج القياسي |
| 2 | 0x1 | 0x2 | عنوان `notice` | 0x17 = 23 | `write` لـ 23 بايتاً إلى خرج الأخطاء القياسي |
| 3 | 0x3c = 60 | 0x7 | غير مستخدم | غير مستخدم | `exit` بحالة 7 |

العلم هو `ARL{flare_42_23_7}`.

## الجزء 2: Assemble

الحل المرجعي هو `solution.asm`. يتبع هيكله الجدول أعلاه: كتلة واحدة لكل استدعاء.

</div>

```nasm-fragment
_start:
    mov     eax, SYS_WRITE      ; write(STDOUT, banner, banner_len)
    mov     edi, STDOUT
    lea     rsi, [banner]
    mov     edx, banner_len
    syscall
    ...
    mov     eax, SYS_EXIT       ; exit(7)
    mov     edi, 7
    syscall
```

<div dir="rtl">

تفصيلان يجعلان الأطوال صحيحة دون عدّ يدوي:

- يوضع `banner_len equ $ - banner` مباشرة بعد آخر سطر `db` من النص. `$` هو العنوان الحالي، فالفرق هو عدد البايتات بين الوسم وتلك النقطة.
- الرقم `10` في نهاية كل سطر هو بايت السطر الجديد، ويُحتسب ضمن الطول. النص الأول يتكون من 14 + 1 و10 + 1 و15 + 1 بايتاً، أي 42.

## أخطاء تضيّع الوقت

- **عدّ النص بالعين.** من السهل نسيان بايتات السطر الجديد. استخدم `$ - label`.
- **نسيان `exit`.** يواصل المعالج تنفيذ البايتات التالية لآخر تعليمة ويموت البرنامج بخطأ تجزئة (segmentation fault). يبلّغ المدقق "terminated by signal 11".
- **تبديل الواصفين.** الواصف 1 هو الخرج القياسي و2 هو خرج الأخطاء القياسي. شغّل `./solution 2>/dev/null` لترى الخرج القياسي فقط وتتأكد.
- **قراءة `edx` على أنه عشري.** `0x2a` هو 42 وليس 2 أو 20.

</div>

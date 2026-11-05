<!-- source: WRITEUP.md @ sha256:6e65700b9119 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

<div dir="rtl">

# شرح الحل: التمرين 03. دفتر السجلات

العلم هو `ARL{ledger_37000_1010_1100_0011}`. عند `mark1` يحمل السجل `rax` القيمة 37000؛ وعند النقاط الثلاث التالية تكون الرايات CF وSF، ثم CF وZF، ثم SF وOF. يلتقط الحل المرجعي الرايات بـ `setc` و`setz` و`sets` و`seto` ويدمجها بالإزاحات و`or`.

## الجزء 1: Reverse

### اقرأ البرنامج أولاً

</div>

```text
$ objdump -d -M intel --no-show-raw-insn bin/ledger
  mov    eax,0x3e8
  imul   rax,rax,0x25
<mark1>:
  sub    rax,0x9c40
<mark2>:
  mov    ecx,0xffffffff
  add    ecx,0x1
<mark3>:
  movabs rdx,0x7fffffffffffffff
  add    rdx,0x1
<mark4>:
```

<div dir="rtl">

أربعة حسابات قصيرة، يتبع كلاً منها وسم. توقّع الرايات قبل تشغيل البرنامج هو جوهر التمرين: افعل ذلك عند كل وسم، ثم تحقق بمصحح الأخطاء. كتب المجمّع `mov rax, 1000` على هيئة `mov eax, 0x3e8`، وهي تصفّر أيضاً النصف الأعلى من `rax`؛ و`movabs` هي صيغة `mov` التي تحمل ثابتاً ذا 64 بت.

### التأكد في مصحح الأخطاء

</div>

```text
$ gdb -q bin/ledger
(gdb) break mark1
(gdb) break mark2
(gdb) break mark3
(gdb) break mark4
(gdb) run
(gdb) p/d $rax
(gdb) continue
(gdb) p/x $rax
(gdb) p $eflags
...
```

<div dir="rtl">

القيم عند كل وسم، من تشغيل حقيقي:

| الوسم | التعليمة التي فوقه مباشرة | النتيجة | الرايات التي يعرضها GDB |
|---|---|---|---|
| `mark1` | `imul rax, rax, 37` | `rax` = 37000 (`0x9088`) | لا تُسجَّل (تترك `imul` الرايتين SF وZF غير معرّفتين) |
| `mark2` | `sub rax, 40000` | `rax` = `0xfffffffffffff448` = -3000 | CF PF SF |
| `mark3` | `add ecx, 1` | `ecx` = 0، `rcx` = 0 | CF PF AF ZF |
| `mark4` | `add rdx, 1` | `rdx` = `0x8000000000000000` | PF AF SF OF |

قراءة الرايات:

- **`mark2`:** 37000 ناقص 40000 يحتاج استلافاً، لأن 37000 أصغر من 40000 كعددين بلا إشارة (CF). والنتيجة سالبة (SF). ولا فيض بإشارة (OF مصفّرة). و PF هي تكافؤ البايت الأدنى ولم تُطلب.
- **`mark3`:** `0xFFFFFFFF + 1` يلتف إلى 0 في 32 بت مع حمل خارج (CF) ونتيجة صفرية (ZF). ولأن الكتابة في السجل كانت بعرض 32 بت، يُصفَّر النصف الأعلى من `rcx` أيضاً، فتكون `rcx` صفراً.
- **`mark4`:** `0x7FFFFFFFFFFFFFFF + 1` هو أكبر عدد موجب بإشارة زائد واحد. وكنتيجة بإشارة التفّ إلى أصغر عدد سالب (OF وSF). وكجمع بلا إشارة لم يخرج أي حمل (CF مصفّرة).

بالترتيب CF وZF وSF وOF: `mark2` هو `1010`، و`mark3` هو `1100`، و`mark4` هو `0011`. والعلم هو `ARL{ledger_37000_1010_1100_0011}`.

## الجزء 2: Assemble

الحل المرجعي هو `solution.asm`. تنفذ كل دالة عمليتها، وتخزّن النتيجة بـ `mov` (التي لا تمس الرايات)، ثم تقفز إلى شيفرة مشتركة بين الدالتين:

</div>

```nasm-fragment
ledger_add:
    add     rdi, rsi
    mov     [rdx], rdi
    jmp     collect_flags

collect_flags:
    setc    al                          ; all four setcc first,
    setz    cl                          ; while the flags are still those of the operation
    sets    r8b
    seto    r9b
    movzx   eax, al
    movzx   ecx, cl
    shl     ecx, 1
    or      eax, ecx
    ; ... SF at bit 2 and OF at bit 3 the same way
    ret
```

<div dir="rtl">

تعيد `ledger_sub(37000, 40000)` للطرح في الجزء 1 القيمة `5`: CF في البت 0 وSF في البت 2.

## أخطاء تضيّع الوقت

- **قراءة الرايات بعد حساب آخر.** تعيد `or` و`shl` و`add` كتابة الرايات جميعها. نفّذ كل تعليمات `set` أولاً ثم ادمج البايتات بعدها. مع `add rdx, 0` شاردة قبل الجمع تفشل 6 من 17 حالة اختبار.
- **استخدام `cmp` أو `test` لإعادة بناء الرايات.** يبدو ذلك مكافئاً، لكن المدقق يمنعه، كما أن `cmp` تكتب فوق الرايات التي ضبطتها العملية للتو.
- **استخدام `lea` للجمع.** تحسب `lea rax, [rdi + rsi]` المجموع لكنها لا تضبط أي راية.
- **نسيان أن `ledger_sub` تستلف.** تعني CF بعد `sub` أن المعامل الأول كان أصغر من الثاني كعددين بلا إشارة.
- **عدم تخزين النتيجة.** تملأ الاختبارات `*out` بقيمة حارسة أولاً؛ وتجاوز التخزين يُفشل كل الحالات.

</div>

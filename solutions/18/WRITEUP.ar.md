<!-- source: WRITEUP.md @ sha256:936cb9e94e24 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

<div dir="rtl">

# شرح الحل: التمرين 18. تقرير صيد الأخطاء

العلم هو `ARL{bughunt_16_128_8_64_5}`: يقبل معالج الأسماء طولاً حتى 16 في مخزن من 16 بايتاً، ويمرر معالج الأعداد كل بايت طول من 128 فصاعداً عبر فحصه بإشارة، ويقبل معالج الوسوم قيمة وسم حتى 8 لجدول من 8 مدخلات، ويلتف حجم كتلة البيانات عند 8 بتات ابتداءً من عدد 64، ويحرر نوع السجل 5 ذاكرة تُستخدم بعد ذلك.

## الجزء 1: Reverse

### التوجه

الملف التنفيذي مجرد من الرموز وقد دُمجت شيفرة التحليل في الدالة الرئيسية. الخطوة الأولى هي إيجاد بنية البرنامج بدل قراءته من الأعلى:

- يُقرأ الملف بـ `fread` واحد لما يصل إلى 4096 بايتاً في مخزن ساكن عند 0x404080. وبعد الاستدعاء لا يُستخدم الطول المعاد أبداً: لا شيء يقارن موضعاً بحجم البيانات. وهذه أول قرينة على عبور السجلات.
- تُوزَّع السجلات عبر جدول قفز:

</div>

```text
401465:  mov    rbx,rdi
401468:  cmp    al,0x5                       ; type above 5: unknown, error -6
40146a:  ja     40153f
401470:  movzx  eax,al
401473:  jmp    QWORD PTR [rax*8+0x402088]   ; table for the types 1 to 5
```

<div dir="rtl">

يحوي الجدول عند 0x402088 للأنواع من 1 إلى 5 العناوين 0x401316 و0x401362 و0x4012e4 و0x4013bc و0x401438. كل عنوان هو شيفرة نوع سجل، وكل منها مصب يجب تدقيقه. وتؤكد التجربة بملفات دخل صغيرة ما تفعله: النوع 1 يضبط الاسم، والنوع 2 يضيف أعداداً، والنوع 3 يعدّ الوسوم، والنوع 4 يمسك كتلة بيانات، والنوع 5 يتخلى عنها.

### عبور السجلات (كل الأنواع)

</div>

```text
401440:  add    rbp,0x1                      ; one more record
401444:  movzx  eax,BYTE PTR [rbx]           ; type
401447:  movzx  edx,BYTE PTR [rbx+0x1]       ; length
40144b:  movzx  r8d,dl
40144f:  lea    rsi,[rbx+0x2]                ; payload
401453:  movzx  ecx,dl
401456:  lea    rdi,[rsi+rcx*1]              ; the next record = payload + length
40145a:  mov    r14,rbx
40145d:  test   al,al
40145f:  je     4012b6                       ; type 0 ends the loop
401465:  mov    rbx,rdi
```

<div dir="rtl">

تقرأ الحلقة بايتي ترويسة وتحسب الموضع التالي، ولا يُقارن الموضع في أي نقطة بنهاية البيانات. فملف يعلن آخر سجل فيه طولاً لا يتسع يجعل الشيفرة تقرأ بعد نهاية المخزن، وملف بلا سجل من النوع 0 يجعلها تقرأ حتى تصادف واحداً، أينما كان. والصنف قراءة خارج الحدود (CWE-125).

### الأسماء (النوع 1، 0x401316)

</div>

```text
401316:  pxor   xmm0,xmm0
40131a:  movaps XMMWORD PTR [rsp+0x80],xmm0  ; the 16-byte local name, cleared
401322:  cmp    r8d,0x10
401326:  ja     401356                       ; length above 16: error -2
401328:  lea    rax,[rsp+0x80]
...
401335:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]    ; copy length bytes
40133a:  mov    BYTE PTR [rsp+r8*1+0x80],0x0            ; terminator at name[length]
```

<div dir="rtl">

للمخزن 16 بايتاً، والحد 16، ويُكتب المنهي عند الفهرس `length`. وعند الطول 16 يكون المنهي هو البايت السابع عشر، أي واحداً بعد النهاية: خطأ بمقدار واحد يكتب بايتاً صفرياً خارج المخزن (CWE-193، كتابة خارج الحدود). إذن `N` يساوي 16، والحد الصحيح 15.

### الأعداد (النوع 2، 0x401362)

</div>

```text
401362:  cmp    dl,0x20
401365:  jg     4013b0                       ; signed: above 32 is error -3
401367:  lea    rax,[rsp+0x20]               ; a local array of 8 integers (32 bytes)
40136c:  movsx  rcx,dl                       ; the size, sign-extended to 64 bits
401370:  mov    rdi,rax
401373:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

<div dir="rtl">

تستخدم المقارنة السجل ذا 8 بتات `dl` وقفزة بإشارة. تمر بايتات الطول من 0 إلى 32، وكذلك كل البايتات من 128 إلى 255 التي تكون سالبة حين تُقرأ أعداداً من 8 بتات بإشارة. ويوسّع `movsx` التالي القيمة السالبة إلى عدد ضخم من 64 بتاً لـ `rep movs`. إذن `V` يساوي 128. وينقص الفحص أيضاً اختبار أن الطول من مضاعفات 4. وهذا خطأ تحويل من إشارة إلى بلا إشارة (CWE-195) يؤدي إلى كتابة خارج الحدود. ويقارن الإصلاح الطول بلا إشارة ويفحص مضاعف 4.

### الوسوم (النوع 3، 0x4012e4)

</div>

```text
4012f6:  movzx  eax,BYTE PTR [rdx]           ; the tag byte
4012f9:  cmp    al,0x8
4012fb:  ja     40154b                       ; above 8: error -4
401301:  movzx  eax,al
401304:  add    DWORD PTR [rsp+rax*4],0x1    ; histogram[tag]++
```

<div dir="rtl">

المدرج التكراري ثمانية عدادات من 32 بتاً في أسفل الإطار (صُفّرت بـ `rep stos` سابق لثماني كلمات مزدوجة). يمرر الفحص القيمة 8، وتمس الزيادة عند الفهرس 8 البايتات الأربعة التي بعد الجدول: خطأ بمقدار واحد مع كتابة (CWE-193). إذن `T` يساوي 8؛ والحد الصحيح 7. وفي هذا البناء يكون الجار هو مصفوفة معالج الأعداد، فيكون الضرر صامتاً، وهذا ما يجعل هذا النوع من الأخطاء خطراً.

### كتل البيانات (النوع 4، 0x4013bc)

</div>

```text
4013bc:  movzx  r13d,BYTE PTR [r14+0x2]      ; count = first payload byte
4013c1:  mov    rdi,r12
4013c4:  call   free                         ; the previous block
4013c9:  lea    edi,[r13*4+0x0]
4013d1:  movzx  edi,dil                      ; the size is truncated to 8 bits
4013d5:  call   malloc
4013da:  mov    r12,rax
4013dd:  movzx  eax,r13b
4013e1:  shl    eax,0x2                      ; the copy size is count * 4 in 32 bits
...                                          ; a copy of that many bytes into the block
```

<div dir="rtl">

حجم الحجز هو `count * 4` مقطوعاً إلى 8 بتات، وحجم النسخ هو `count * 4` بـ 32 بتاً. يتفقان ما دام العدد دون 64. ومن العدد 64 فصاعداً يبلغ الناتج 256 ويلتف حجم الحجز (64 يعطي `malloc(0)` و65 يعطي `malloc(4)`)، بينما يظل النسخ يكتب أربعة أضعاف العدد: فيض في الكومة (CWE-190 يؤدي إلى CWE-122). إذن `B` يساوي 64. ولا تقارن الشيفرة أيضاً طول السجل بالعدد أبداً، فقد يقرأ النسخ بعد الحمولة (CWE-125). ويحسب الإصلاح الحجم بنوع عريض ويشترط أن يساوي الطول 1 زائد 4 أضعاف العدد.

### التحرير (النوع 5، 0x401438)

</div>

```text
401438:  mov    rdi,r12
40143b:  call   free                         ; free the block; r12 keeps its value
401440:  ...                                 ; the loop continues
...
40147a:  movzx  ecx,BYTE PTR [r12+rax*1]     ; at the end: sum the block bytes (r13 of them)
...
401494:  mov    rdi,r12
401497:  call   free                         ; and free it again
```

<div dir="rtl">

يحرر السجل من النوع 5 الكتلة ويترك `r12` والطول `r13` دون تغيير. وتجمع الشيفرة التي بعد الحلقة بايتات الكتلة وتحررها مرة ثانية: قراءة بعد التحرير (CWE-416) وتحرير مزدوج (CWE-415). إذن `U` يساوي 5. (ويحرر السجل من النوع 4 الكتلة القديمة قبل أن يحجز جديدة، وهذا صحيح فقط لأنه يكتب فوق `r12`.) ويضع الإصلاح المؤشر فارغاً والطول 0 بعد التحرير.

### تقرير نموذجي

</div>

```text
Title:      The record walk never compares its position with the end of the data
Class:      CWE-125 out-of-bounds read
Location:   main (the parser is inlined), loop at 0x401440 to 0x401465
Severity:   Medium. The length bytes come from the file; only reads are affected.
Cause:      The next record is computed as payload + length and the position is
            never compared with the size of the data returned by fread.
Evidence:   A file with a last record of length 255 and 10 bytes of data makes
            the loop read the stale content of the static buffer.
Impact:     Bytes outside the file are interpreted as records; the output depends
            on memory that is not part of the input and the program can fault.
Fix:        Track the bytes that are left, and refuse a record when fewer than 2
            bytes remain or the payload does not fit.
Test:       Truncations at every position of a valid file; a last record of length
            255 with no data.

Title:      Names: off-by-one in the terminator
Class:      CWE-193 off-by-one (out-of-bounds write of one byte)
Location:   0x401322 (limit) and 0x40133a (terminator)
Severity:   Medium. One byte with a fixed value, written next to a stack buffer.
Cause:      The limit is 16 for a buffer of 16 bytes, and the terminator is written
            at index length.
Evidence:   A name of 16 bytes passes the check; the store at [rsp+r8+0x80] with
            r8 = 16 is outside the 16 bytes at [rsp+0x80].
Impact:     A zero byte is written to whatever follows the buffer.
Fix:        Accept lengths up to 15.
Test:       Names of 0, 15, 16 and 255 bytes.

Title:      Numbers: signed length check
Class:      CWE-195 signed to unsigned conversion (leading to an out-of-bounds write)
Location:   0x401362 to 0x401373
Severity:   High. A length byte of 128 to 255 gives a copy size of about 2^64.
Cause:      The length is tested as a signed 8-bit value and then sign-extended to
            the copy size.
Evidence:   A record with length 128 crashes the program; a debugger stopped at
            0x401370 shows rcx = 0xffffffffffffff80.
Impact:     The copy runs far beyond its 32-byte destination and ends with a fault.
Fix:        Treat the length as unsigned; require a multiple of 4 and at most 32.
Test:       Lengths 0, 4, 32, 33, 36, 127, 128, 255.

Title:      Tags: the table has 8 entries and the check accepts 8
Class:      CWE-193 off-by-one (out-of-bounds write of a counter)
Location:   0x4012f9 and 0x401304
Severity:   Medium. A counter increment on the four bytes after the table.
Cause:      The limit is "above 8" for a table whose last index is 7.
Evidence:   A tag value of 8 passes the check and the increment is at [rsp+0x20].
Impact:     The data that follows the table is changed.
Fix:        Reject tag values above 7.
Test:       Tag values 0, 7, 8, 9, 255.

Title:      Data blocks: the allocation size wraps
Class:      CWE-190 integer overflow (leading to CWE-122 heap overflow)
Location:   0x4013c9 to 0x4013e1
Severity:   High. The count and the data come from the file.
Cause:      malloc receives count * 4 truncated to 8 bits; the copy uses count * 4
            in 32 bits; the record length is not compared with the count.
Evidence:   A count of 64 gives malloc(0) and a copy of 256 bytes.
Impact:     A heap overflow of up to 768 bytes beyond a small allocation.
Fix:        Compute the size in 64 bits, require length = 1 + 4 * count, and
            allocate that size.
Test:       Counts 0, 1, 63, 64, 65, 255, each with a matching and a mismatching
            length.

Title:      Release: the freed block is used and freed again
Class:      CWE-416 use after free (and CWE-415 double free)
Location:   0x401438 and 0x40147a to 0x401497
Severity:   High. The block is read after it was freed, then freed again.
Cause:      Type 5 frees the block and keeps the pointer and the length.
Evidence:   A file with a data block followed by a record of type 5 is enough;
            in the debugger, the second call to free at 0x401497 receives the
            pointer that the first call at 0x40143b has already released.
Impact:     Reads of freed memory, and corruption of the allocator state.
Fix:        Set the pointer to null and the length to 0 after the free.
Test:       A block then a reset; a reset with no block; a reset twice; a block
            after a reset.
```

<div dir="rtl">

## الجزء 2: Assemble

الحل المرجعي هو `solution.asm`. والنقاط المهمة:

- **الحلقة.** تُحسب البايتات المتبقية بالصيغة `n - pos` قبل كل مقارنة. ويمكن أيضاً مقارنة الموضع زائد الطول بـ `n`، لكن الطرح لا يفيض لأن `pos` لا يتجاوز `n` أبداً. يُفحص بايتا الترويسة أولاً ثم الحمولة، وعندئذ فقط يُفحص النوع، فيتبع السجل من النوع 0 القاعدة نفسها.
- **الأسماء.** يمسح تخزين من 16 بايتاً الحقل ثم تُنسخ بايتات الحمولة. والحد 15.
- **الأعداد.** يرفض `test r11b, 3` الأطوال التي ليست من مضاعفات 4، وترفض مقارنة بلا إشارة ما فوق 32. وتُحمَّل القيم بـ `movsxd` ليُضاف العدد السالب إضافة صحيحة إلى المجموع من 64 بتاً.
- **الوسوم.** يُقارن البايت بـ 7 قبل أن يفهرس الجدول.
- **كتل البيانات.** يحسب `lea r10d, [r10*4 + 1]` الطول المتوقع في سجل من 32 بتاً؛ وعدد يتسع في بايت لا يمكن أن يفيضه. وتحسم المقارنة بالطول كل شيء، فلا قراءة تتجاوز الحمولة. ويبدأ المجموع من 0 لكل كتلة.
- **السجلات.** لا تُستخدم إلا السجلات التي يجوز للدالة تغييرها، فلا شيء يُحفظ، ولا استدعاءات.

## أخطاء تضيّع الوقت

- **مقارنة الموضع زائد الطول بـ `n` بـ 32 بتاً.** مع مخزن قرب أعلى فضاء العناوين أو بطول كبير يلتف المجموع. احسب ما تبقى بدلاً من ذلك.
- **فحص نوع السجل قبل القطع.** يجب أن يُرفض السجل من النوع 0 الذي قُطعت حمولته كذلك.
- **استخدام تحميل بلا إشارة للأعداد.** تضيف القيم السالبة مقداراً خاطئاً؛ ويختلف المجموع.
- **نسيان مسح الاسم.** الاسم الأقصر الذي يحل محل أطول يترك بايتات قديمة.
- **عدم تصفير مجموع الكتلة.** تُضاف الكتلة الثانية إلى الأولى.
- **قراءة العدد قبل فحص أن الطول لا يقل عن 1.** سجل من النوع 4 بلا حمولة يقرأ بايتاً بعدها.
- **فحص الوسم مقابل 8.** يُنسخ خطأ الأصل بمقدار واحد إلى الإصلاح.

</div>

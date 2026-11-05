<!-- source: WRITEUP.md @ sha256:daf1bb341543 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

<div dir="rtl">

# شرح الحل: التمرين 19. مكتب الفرز

العلم هو `ARL{triage_6_34_1543003_1574002_1071004}`: للإعدادات 6 مداخل مرمَّزة، وأطول مدخل مفكوك طوله 34 بايتاً، والسلوكيات الثلاثة تقابل T1543.003 (خدمة Windows) وT1574.002 (التحميل الجانبي لـ DLL) وT1071.004 (DNS).

## الجزء 1: Reverse

### التعريف والسلاسل النصية

الملفان صغيران ومجردان من الرموز. يبلّغ `file` عن ELF من x86-64 لـ `dispatch` وعن ملف PE32+ لوحدة التحكم لـ `dispatch.exe`. تشغيل نسخة Linux آمن، لأن البرنامج لا يستدعي شيئاً له أثر:

</div>

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would query the list of running processes
[sim] would create the service <cfg 0> that runs <cfg 1>
[sim] would place the library <cfg 2> next to the program <cfg 3>
[sim] would resolve names under <cfg 4> and carry data in the labels
[sim] would clear the event log <cfg 5>
```

<div dir="rtl">

يجيب الخرج عن سؤال السلوك على مستوى الأفعال، وتؤكد سلاسل الملف أن هذه القوالب هي النص المقروء الوحيد: يعرض `strings -n 6` الأسطر الستة نفسها مع `%d` حيث تكون الأرقام في الخرج. البيانات المهمة غائبة: أسماء الخدمة والملفات، والنطاق، والسجل. لا توجد سلاسل مقروءة أخرى ولا استيراد يتعلق بالخدمات أو الملفات أو الشبكات: نسخة Linux لا تستورد إلا `puts` و`__printf_chk`، ونسخة Windows لا تستورد إلا دوال البدء والذاكرة التي تحتاجها بيئة تشغيل المترجم من `KERNEL32.dll` و`msvcrt.dll` إضافة إلى `puts`. لذلك لا بد أن تكون البيانات داخل الملف بصيغة لا تستطيع `strings` قراءتها.

### الجدول ومفكّك الترميز

يُملأ العنصر النائب `<cfg %d>` من مصفوفة من ست خانات، حجم كل منها 64 بايتاً، في ذاكرة قابلة للكتابة. الشيفرة التي تملأ المصفوفة هي أول ما تفعله الدالة الرئيسية بعد السطر الأول:

</div>

```text
401178:  mov    ebx,0x404040            ; destination: six slots of 0x40 bytes
40117d:  mov    r12d,0x4041c0           ; end of the destination
401183:  mov    ebp,0x4021a0            ; source: the table in .rodata
401188:  mov    rsi,rbx
40118b:  mov    rdi,rbp
40118e:  call   401136                  ; decode one entry, returns its length
401193:  lea    rbp,[rbp+rax*1+0x2]     ; next entry = this one + length + 2
401198:  add    rbx,0x40
40119c:  cmp    rbx,r12
40119f:  jne    401188
```

<div dir="rtl">

الحلقة هي مفتاح البنية. تمتد الوجهة من 0x404040 إلى 0x4041c0، أي 0x180 بايتاً أو ست خانات من 0x40. يتقدم مؤشر المصدر بالقيمة التي تعيدها الدالة زائد 2، فكل مدخل في الجدول طول يتبعه بايت إضافي ثم هذا العدد من البايتات. العدد `N` هو 6.

الدالة عند 0x401136 هي مفكّك الترميز:

</div>

```text
401136:  movzx  r8d,BYTE PTR [rdi]        ; length
40113b:  movzx  edx,BYTE PTR [rdi+0x1]    ; seed: the initial state
40113f:  test   r8,r8
401141:  je     401161                    ; length 0: nothing to decode
401143:  mov    eax,0x0                   ; position i = 0
401148:  rol    dl,0x3                    ; state = rotate left by 3
40114b:  lea    edx,[rdx+rax*1+0x5d]      ; state = state + i + 0x5d
40114f:  mov    ecx,edx
401151:  xor    cl,BYTE PTR [rdi+rax*1+0x2]  ; plain = encoded xor state
401155:  mov    BYTE PTR [rsi+rax*1],cl
401158:  add    rax,0x1
40115c:  cmp    r8,rax
40115f:  jne    401148
401161:  mov    BYTE PTR [rsi+r8*1],0x0   ; terminating zero byte
401166:  mov    rax,r8
401169:  ret
```

<div dir="rtl">

بلغة C تُقرأ الدالة نفسها هكذا. التعليمة `lea` ذات الحدود الثلاثة هي جمع الموضع والثابت، ولا يهم إلا البايت الأدنى من النتيجة، لأن الحالة تُستخدم عبر `dl` و`cl`:

</div>

```c-fragment
size_t decode_record(const unsigned char *rec, char *out)
{
    size_t len = rec[0];
    unsigned char k = rec[1];
    for (size_t i = 0; i < len; i++) {
        k = (unsigned char)(rol8(k, 3) + i + 0x5d);
        out[i] = rec[2 + i] ^ k;
    }
    out[len] = 0;
    return len;
}
```

<div dir="rtl">

مخطط بمفتاح ثابت، كما في الوحدة 20، لن يفك ترميز هذا الجدول: المفتاح يتغير مع الموضع، وكل مدخل يبدأ من بذرته الخاصة. المحاولة الأولى بتطبيق xor على كل بايت بثابت واحد تعطي ضجيجاً في كل المداخل، وتجريب الثوابت الـ 256 لا يجد شيئاً يُقرأ نصاً. التعرف على الدوران هو ما يحل المسألة.

### فك الترميز

بعد وضع القاعدة في نص برمجي، أو في المحاكي من الوحدة 20، تُفك المداخل الستة إلى:

| الخانة | الطول | البذرة | النص |
|---|---|---|---|
| 0 | 13 | 0x13 | WinUpdSvcHost |
| 1 | 34 | 0xA7 | C:\ProgramData\Intel\svchost32.exe |
| 2 | 11 | 0x5C | version.dll |
| 3 | 19 | 0x2E | OneDriveUpdater.exe |
| 4 | 22 | 0xD1 | ns1.cdn-telemetry.test |
| 5 | 8 | 0x66 | Security |

أطول نص هو المسار في الخانة 1 بطول 34 بايتاً: `L` هو 34. النص المفكوك لا يوجد في الذاكرة إلا بعد تنفيذ الحلقة، ولذلك لا تجده `strings` ولا البحث في الملف. المنقّح الذي يتوقف بعد الحلقة يُظهر القيم نفسها عند 0x404040.

### السلوكيات والتقنيات

كل قالب، بعد ملئه بالقيم المفكوكة، يصف سلوكاً:

| القالب | بالقيم المفكوكة | ATT&CK |
|---|---|---|
| query the list of running processes | تعداد النظام | T1057 Process Discovery |
| create the service `<cfg 0>` that runs `<cfg 1>` | خدمة اسمها WinUpdSvcHost تشغّل برنامجاً في مجلد يخص المورّد | T1543.003 Create or Modify System Process: Windows Service |
| place the library `<cfg 2>` next to the program `<cfg 3>` | version.dll بجوار OneDriveUpdater.exe، والبرنامج يحمّلها أولاً من مجلده | T1574.002 Hijack Execution Flow: DLL Side-Loading |
| resolve names under `<cfg 4>` and carry data in the labels | أسماء تحت نطاق المشغِّل مع بيانات في تسميات النطاق الفرعي | T1071.004 Application Layer Protocol: DNS |
| clear the event log `<cfg 5>` | سجل Security | T1070.001 Indicator Removal: Clear Windows Event Logs |

يطلب العلم التقنيات الثلاث في الوسط. وهناك تحفظان على التقرير. البرنامج محاكٍ: يطبع ما كان سيفعله، ويصف الربط السلوك الذي يحاكيه لا سلوكاً لوحظ على نظام. ومعرّف التقنية يُستنتج من السلوك لا من اسم في سلسلة نصية: اسم الخدمة WinUpdSvcHost مختار ليبدو مكوّناً من النظام، وهذه نقطة تُذكر في التقرير، لكن T1543.003 تسنده عملية إنشاء الخدمة نفسها.

ومنه يأتي `ARL{triage_6_34_1543003_1574002_1071004}`: `N` هو 6، و`L` هو 34، والمعرّفات بعد حذف `T` والنقطة هي 1543003 و1574002 و1071004.

### قاعدة الكشف

لا يحتوي الملف نصاً مفكوكاً، لذلك لا تستطيع القاعدة استخدامه. أما ما يحتويه الملف فهو قوالب السجل الستة بنص صريح وأول بايتات المدخل الأول المرمَّز. قاعدة المرجع في `solutions/19/rule.yar`:

</div>

```text
rule triage_desk_dispatch
{
    strings:
        $service  = "would create the service <cfg" ascii
        $sideload = "would place the library <cfg" ascii
        $dns      = "would resolve names under <cfg" ascii
        $clear    = "would clear the event log <cfg" ascii
        $discover = "would query the list of running processes" ascii
        $record0  = { 0D 13 A2 64 A9 CB 25 68 90 F4 1A 79 9F 9C 9C }
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($service, $sideload, $dns, $clear, $discover) and $record0
}
```

<div dir="rtl">

يتبع التعليل البرامج المموِّهة، وهي الجزء من التمرين الذي يختبر القاعدة:

- يطبع `notes` نص مساعدة يذكر مفتاح Run وخدمة و`version.dll`. القاعدة المبنية على هذه الكلمات تُطلَق عليه. العينة لا تحتويها بنص صريح.
- يطبع `dryrun` أسطراً بصيغة `[dry-run] would ...`. القاعدة المبنية على الكلمة `would`، أو على جزء قصير مثل `would create`، تُطلَق عليه. قوالب العينة تحمل العنصر النائب `<cfg` الذي لا يوجد في التشغيل التجريبي.
- يحتوي `updater` روتين فك الترميز نفسه وجدولاً مرمَّزاً بنصوص غير مؤذية. القاعدة المبنية على بايتات مفكّك الترميز تُطلَق عليه. المدخل الأول المرمَّز في العينة لا يظهر في المحدِّث.
- يمثل `/bin/ls` البرامج التي يجب ألا يُطلَق عليها شيء أبداً.

يجعل فحص نوع الملف القاعدة تعمل على الملفات التنفيذية فقط. وهو ليس لازماً لفصل البرامج المموِّهة، ويؤكد المدقق أن قاعدة بدونه تُقبل أيضاً، لكنه العادة التي تبقي القاعدة سريعة على المجموعات الكبيرة. أول بايتات المدخل الأول هي الجزء الوحيد من القاعدة الذي يعتمد على البناء بعينه؛ ونسخة من العينة بجدول مختلف تحتاج قاعدة جديدة، وهذه هي الحالة العادية للكشف وليست عيباً في هذه القاعدة. والقاعدة التي تحتاج القوالب فقط، مثل `all of them` على أطول أربعة منها، صحيحة أيضاً.

## الجزء 2: Assemble

فاك ترميز العينة، مضافاً إليه فحصا المواصفات، هو الدالة كلها. حل المرجع في `solutions/19/solution.asm`:

</div>

```nasm-fragment
decode_record:
    cmp     rsi, 2
    jb      .truncated                  ; no room for the length and the seed
    movzx   r8d, byte [rdi]             ; length
    lea     r9, [r8 + 2]
    cmp     r9, rsi
    ja      .truncated                  ; the record is cut off
    lea     r9, [r8 + 1]
    cmp     r9, rcx
    ja      .small                      ; text and zero byte do not fit
    movzx   r10d, byte [rdi + 1]        ; k = seed
    xor     r11d, r11d                  ; i = 0
.next:
    cmp     r11, r8
    jae     .done
    rol     r10b, 3
    add     r10b, r11b                  ; plus i (low byte)
    add     r10b, 0x5d
    mov     al, [rdi + r11 + 2]
    xor     al, r10b
    mov     [rdx + r11], al
    inc     r11
    jmp     .next
```

<div dir="rtl">

ترتيب الفحوص مهم. المدخل المقطوع والأطول من أن يتسع في `cap` يعيد -1، لأن الفحص الأول يُطبَّق أولاً، وتتضمن الاختبارات مداخل كهذه. واختبار `avail` الأصغر من 2 يسبق قراءة بايت الطول، لأن الطول نفسه جزء من المدخل. وإجراء المقارنة مع `avail` على صورة `length + 2 > avail` في سجل من 64 بتاً يتجنب أي تجاوز، لأن الطول 255 على الأكثر.

## أخطاء تضيّع الوقت

- فك الترميز بمفتاح ثابت. من السهل تجاوز الدوران على أنه "خطوة خلط"؛ والحالة المتغيرة عند كل موضع هي ما يُفشل المفتاح الثابت.
- نسيان أن الموضع يُضاف إلى الحالة في كل خطوة، بما فيها الأولى. بدء الموضع من 1، أو إضافته بعد xor، يعطي نصاً خاطئاً من البايت الأول.
- استخدام السجل ذي 64 بتاً للحالة. الحالة بايت واحد: يجب أن يلتف الدوران والجمع عند 8 بتات. تستخدم الاختبارات بذوراً وأطوالاً كثيرة مختلفة تكشف حالة لا تلتف.
- قاعدة مبنية على كلمة شائعة أو على مفكّك الترميز وحده. البرامج المموِّهة موجودة لهذا السبب. يسمّي المدقق البرامج التي تعطي عليها القاعدة الجواب الخاطئ.
- فحص `cap` قبل `avail`. المواصفات تحدد الترتيب، والمداخل المقطوعة مع `cap` صغير تُظهره.
- الثقة باسم الخدمة. الاسم WinUpdSvcHost مقصود به أن يبدو جزءاً من النظام؛ والتقنية تُستنتج مما يفعله البرنامج.

</div>

<!-- source: WRITEUP.md @ sha256:8af0bc97c35b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

<div dir="rtl">

# شرح الحل: التمرين 04. ترتيب البايتات

العلم هو `ARL{v3_7_p8443_t45000}`. للسجل حقل big-endian بحجم 32 بت عند الإزاحة 0، وحقل little-endian بحجم 16 بت عند الإزاحة 4، وحقل big-endian بحجم 16 بت عند الإزاحة 6، وحقل little-endian بحجم 32 بت عند الإزاحة 8. يفك الحل المرجعي ترميزه بتحميل البايتات والإزاحات و`or`.

## الجزء 1: Reverse

### قراءة الشيفرة

</div>

```text
$ objdump -d -M intel --no-show-raw-insn bin/config_probe
0000000000401000 <_start>:
  mov    eax,DWORD PTR [rip+0xffa]        # 402000 <record>
  bswap  eax
  movzx  ecx,WORD PTR [rip+0xff5]         # 402004 <record+0x4>
  movzx  edx,WORD PTR [rip+0xff0]         # 402006 <record+0x6>
  rol    dx,0x8
  mov    ebx,DWORD PTR [rip+0xfe8]        # 402008 <record+0x8>
  add    eax,ecx
  add    eax,edx
  add    eax,ebx
  cmp    eax,0x450cb
```

<div dir="rtl">

أربعة تحميلات من السجل، اثنان منها يليهما تبديل:

| الحقل | التعليمة | الإزاحة | العرض | ترتيب البايتات |
|---|---|---|---|---|
| A | `mov eax, [record]` ثم `bswap eax` | 0 | 4 بايتات | big-endian |
| B | `movzx ecx, word [record+4]` | 4 | بايتان | little-endian |
| C | `movzx edx, word [record+6]` ثم `rol dx, 8` | 6 | بايتان | big-endian |
| D | `mov ebx, [record+8]` | 8 | 4 بايتات | little-endian |

تعكس كلٌّ من `bswap` و`rol dx, 8` بايتات ما تعمل عليه، وهكذا يحوّل البرنامج بيانات big-endian إلى القيمة الأصلية little-endian. أما التحميل بلا تبديل فيحتفظ بترتيب الذاكرة، وهو little-endian على x86.

### فك ترميز البايتات

</div>

```text
$ objdump -s -j .rodata bin/config_probe
 402000 00030007 fb208001 c8af0000 ...
```

<div dir="rtl">

السجل هو `00 03 00 07 | fb 20 | 80 01 | c8 af 00 00`.

| الحقل | البايتات | بعد العكس عند الحاجة | القيمة |
|---|---|---|---|
| A | `00 03 00 07` | بترتيب القراءة أصلاً (big-endian) | `0x00030007`: النصف الأعلى 3، والنصف الأدنى 7 |
| B | `fb 20` | `20 fb` | `0x20fb` = 8443 |
| C | `80 01` | بترتيب القراءة أصلاً (big-endian) | `0x8001` |
| D | `c8 af 00 00` | `00 00 af c8` | `0xafc8` = 45000 |

### التحقق بالمجموع الاختباري

يجمع البرنامج القيم الأربع المفكوكة ويقارن المجموع بالقيمة `0x450cb`:

</div>

```text
0x00030007 + 0x20fb + 0x8001 + 0xafc8 = 0x450cb
196615 + 8443 + 32769 + 45000 = 282827
```

<div dir="rtl">

لو قرأت B بترتيب big-endian (`0xfb20` = 64288) لما طابق المجموع. الثابت في البرنامج مفتاح إجابة مدمج لفك ترميزك.

العلم هو `ARL{v3_7_p8443_t45000}`.

## الجزء 2: Assemble

الحل المرجعي هو `solution.asm`.

`load_le16` تعليمة واحدة، لأن التحميل الأصلي للمعالج هو little-endian:

</div>

```nasm-fragment
load_le16:
    movzx   eax, word [rdi]
    ret
```

<div dir="rtl">

تبني `load_be32` القيمة بايتاً بايتاً، بدءاً بالبايت الأعلى أهمية. تحرّك كل خطوة ما هو موجود أصلاً بمقدار 8 بتات وتضيف البايت التالي:

</div>

```nasm-fragment
    movzx   eax, byte [rdi]             ; first byte
    shl     eax, 8
    movzx   edx, byte [rdi + 1]
    or      eax, edx                    ; now two bytes
    ; ... repeat for bytes 2 and 3
```

<div dir="rtl">

تفعل `decode_record` الشيء نفسه لـ `version` و`flags`، وتنسخ `port` و`timeout_ms` بتحميل وتخزين عاديين بالعرض نفسه.

## أخطاء تضيّع الوقت

- **تحميل ثمانية بايتات في `load_be32`.** `mov rax, [rdi]` بداية صالحة، لكنها تقرأ أربعة بايتات بعد نهاية المدخل. تضع الاختبارات المدخل قبل صفحة غير متاحة مباشرة، فينهار البرنامج بالإشارة 11.
- **الكتابة بعرض أكبر في `out`.** `mov [rsi + 4], eax` تخزّن أربعة بايتات في حقل من بايتين وتكتب فوق `flags`. تملأ الاختبارات `out` بنمط معروف أولاً، فيظهر الضرر.
- **عدم تصفير السجل.** تحميل بايت بـ `mov al, [rdi]` يترك البتات العليا القديمة من `eax` في مكانها. استخدم `movzx` لتكون البتات العليا صفراً.
- **استخدام `bswap` أو `movbe`.** تنتجان القيم الصحيحة، ويرفضهما المدقق بالقاعدة.
- **قراءة B أو D بترتيب big-endian.** التحميل بلا تبديل في البرنامج الأصلي هو little-endian. يكشف المجموع الاختباري ذلك فوراً.

</div>

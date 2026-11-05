<!-- source: WRITEUP.md @ sha256:33109aef56b5 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

<div dir="rtl">

# شرح الحل: التمرين 02. عالمان

العلم هو `ARL{worlds_36_162_1_60}`. ينفذ الملفان التنفيذيان الاستدعاءات الثلاثة نفسها (`write` و`sync` و`exit`)، لكن بتعليمات وأرقام وسجلات مختلفة. يغيّر النقل في الجزء 2 الثلاثة جميعاً.

## الجزء 1: Reverse

### تحديد الملفين

</div>

```text
$ file bin/two_worlds32 bin/two_worlds64
bin/two_worlds32: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), statically linked, not stripped
bin/two_worlds64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

<div dir="rtl">

### السلوك متطابق

</div>

```text
$ strace ./bin/two_worlds32
[ Process PID=... runs in 32 bit mode. ]
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?

$ strace ./bin/two_worlds64
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?
```

<div dir="rtl">

يلاحظ `strace` أن العملية الأولى تعمل بوضع 32 بت ويعرض الاستدعاءات الثلاثة نفسها للملفين. الأرقام غير معروضة، فاقرأها في التفكيك.

### الشيفرة مختلفة

</div>

```text
$ objdump -d -M intel bin/two_worlds32
08049000 <_start>:
 8049000:  mov  eax,0x4
 8049005:  mov  ebx,0x1
 804900a:  mov  ecx,0x804a000
 804900f:  mov  edx,0xb
 8049014:  int  0x80
 8049016:  mov  eax,0x24
 804901b:  int  0x80
 804901d:  mov  eax,0x1
 8049022:  xor  ebx,ebx
 8049024:  int  0x80

$ objdump -d -M intel bin/two_worlds64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <text>
  401011:  mov  edx,0xb
  401016:  syscall
  401018:  mov  eax,0xa2
  40101d:  syscall
  40101f:  mov  eax,0x3c
  401024:  xor  edi,edi
  401026:  syscall
```

<div dir="rtl">

| | 32 بت | 64 بت |
|---|---|---|
| التعليمة | `int 0x80` | `syscall` |
| رقم `write` | 0x4 = 4 | 0x1 = 1 |
| رقم `sync` | 0x24 = 36 | 0xa2 = 162 |
| رقم `exit` | 0x1 = 1 | 0x3c = 60 |
| سجل الواصف | `ebx` | `rdi` |
| سجل العنوان | `ecx` | `rsi` |
| سجل الطول | `edx` | `rdx` |
| عنوان النص | مطلق: `mov ecx, 0x804a000` | نسبي: `lea rsi, [rip+0xfef]` |

الصف الأخير اختلاف يسهل تفويته. تحمّل الشيفرة ذات 32 بت عند عنوان ثابت العنوانَ كثابت؛ أما شيفرة 64 بت فتحسبه نسبةً إلى `rip`.

العلم هو `ARL{worlds_36_162_1_60}`.

## الجزء 2: Assemble

الحل المرجعي هو `solution.asm`. مقارنةً بالبرنامج الابتدائي، يغيّر ما يلي:

- `bits 32` إلى `bits 64`، ويضيف `default rel` لعنوان النص.
- `int 0x80` إلى `syscall` في المواضع الثلاثة.
- الأرقام: من 4 إلى 1 لـ `write`، ومن 36 إلى 162 لـ `sync`، ومن 1 إلى 60 لـ `exit`.
- سجلات الوسائط: من `ebx` و`ecx` و`edx` إلى `rdi` و`rsi` و`rdx`.
- `mov ecx, message` إلى `lea rsi, [message]`.

## أخطاء تضيّع الوقت

- **تغيير الترويسة فقط.** البرنامج ذو 32 بت المجمَّع كبرنامج 64 بت ما زال يعمل، لأن Linux يقبل `int 0x80` في العمليات ذات 64 بت ويقرأ جدول الاستدعاءات ذا 32 بت. لهذا يبحث المدقق عن التعليمة نفسها وليس عن الخرج فقط. وفي شيفرة 64 بت لا يمكن تمرير العناوين فوق 4 غيغابايت بهذه الطريقة، فقد يفشل لاحقاً برنامج يعمل اليوم بالصدفة.
- **الإبقاء على الأرقام القديمة.** مع `syscall` يكون رقم الاستدعاء 4 هو `stat` وليس `write`. يضيع الخرج أو يفشل الاستدعاء.
- **قراءة الأرقام على أنها عشرية.** `0xa2` هو 162 وليس 2.
- **حذف `sync`.** ليس له أثر مرئي، ولهذا يسهل نسيانه. يقارن المدقق تسلسل الاستدعاءات.

</div>

<!-- source: README.md @ sha256:7927481bd356 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

<div dir="rtl">

# التمرين 07. كنّاسة السلاسل

Level 2. الوقت: نحو ساعتين. المعمارية: x86-64 على Linux. الوحدات: [04. نقل البيانات وأنماط العنونة](../../modules/04-moving-data/README.ar.md)، [06. الشروط والتفرعات والحلقات](../../modules/06-conditions-and-loops/README.ar.md)، [07. السلاسل النصية وتعليمات السلاسل](../../modules/07-strings/README.ar.md)

## السيناريو

تحمل أداة تشخيص اسمها `sweeper` مكتبة صغيرة خاصة بها: ستة روتينات للذاكرة والسلاسل، بلا مكتبة C خلفها. أسماء الروتينات بلا معنى، ويحتاج القائمون عليها إلى معرفة أيٌّ منها هو أيٌّ قبل أن يوثّقوها أو يستبدلوها. ثم يحتاج مكوّن جديد إلى أربعة من الروتينات نفسها مكتوبة من الصفر.

## الأهداف

- التعرف على روتينات الذاكرة والسلاسل من تعليمات السلاسل والسجلات التي تستخدمها.
- التمييز بين النسخ الأمامي والنسخ الآمن عند التداخل، وبين الطول والبحث.
- كتابة `strlen` و`memchr` و`memcpy` و`memmove` بتعليمات السلاسل، مع إبقاء راية الاتجاه صحيحة.

## الجزء 1: Reverse

الملف التنفيذي هو `bin/sweeper`. يشغّل اختباراً ذاتياً لستة روتينات ويطبع `sweeper: all routines behave`. والروتينات هي الرموز العامة من `fn1` إلى `fn6`.

</div>

```bash
bin/sweeper
objdump -d -M intel bin/sweeper
```

<div dir="rtl">

ينفذ كل روتين إحدى دوال مكتبة C التالية: `strlen` و`memchr` و`memcpy` و`memmove` و`memset` و`memcmp`. حدّد أي رقم `fn` يقابل أي دالة.

ابنِ العلم من الأرقام الستة، بهذا الترتيب للدوال:

</div>

```text
ARL{sweeper_<strlen>_<memchr>_<memcpy>_<memmove>_<memset>_<memcmp>}
```

<div dir="rtl">

مثلاً، لو كانت `strlen` هي `fn3`، و`memchr` هي `fn1`، و`memcpy` هي `fn2`، و`memmove` هي `fn4`، و`memset` هي `fn5`، و`memcmp` هي `fn6`، لكان العلم `ARL{sweeper_3_1_2_4_5_6}`.

## الجزء 2: Assemble

عدّل `starter/solution.asm` لتنفيذ أربعة من الروتينات وفق اصطلاح استدعاء System V: الوسائط في `rdi` و`rsi` و`rdx`، والنتيجة في `rax`.

</div>

```c
#include <stddef.h>

size_t my_strlen(const char *s);                      // bytes before the first zero byte
void  *my_memchr(const void *s, int c, size_t n);     // first byte equal to (unsigned char) c
                                                      // among the n bytes, or NULL
void  *my_memcpy(void *d, const void *s, size_t n);   // copy n bytes, no overlap; returns d
void  *my_memmove(void *d, const void *s, size_t n);  // copy n bytes, overlap allowed; returns d
```

<div dir="rtl">

## القواعد

- استخدم تعليمات السلاسل: يشترط المدقق وجود `rep movs` واحدة على الأقل و`repnz scas` واحدة على الأقل في شيفرتك.
- لا تستخدم `call`.
- تكون راية الاتجاه ممسوحة حين تُستدعى دالتك. ويجب أن تكون ممسوحة حين تعود.
- اقرأ واكتب فقط البايتات التي تخص العملية. تضع الاختبارات كل كتلة قبل صفحة غير متاحة مباشرة، فلمس بايت بعدها يُسقط الاختبار.
- عالج الحالة `n` تساوي 0 معالجة صحيحة.

## تحقق من عملك

</div>

```bash
make check EX=07 ARGS='--flag ARL{your_answer_here}'
```

<div dir="rtl">

يعرض المدقق الجزء 1 والجزء 2 على حدة. وبدون `--flag` يفحص الجزء 2 فقط.

## التلميحات

افتح [HINTS.md](HINTS.ar.md) مستوى واحداً في كل مرة.

</div>

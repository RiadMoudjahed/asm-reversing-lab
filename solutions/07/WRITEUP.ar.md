<!-- source: WRITEUP.md @ sha256:019af3edf455 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

<div dir="rtl">

# شرح الحل: التمرين 07. كنّاسة السلاسل

العلم هو `ARL{sweeper_2_5_1_3_6_4}`: `fn2` هي `strlen`، و`fn5` هي `memchr`، و`fn1` هي `memcpy`، و`fn3` هي `memmove`، و`fn6` هي `memset`، و`fn4` هي `memcmp`. يُعرَّف كل روتين بتعليمة السلاسل التي يستخدمها وبادئتها والسجلات التي يجهّزها قبلها.

## الجزء 1: Reverse

### الروتينات الستة

</div>

```text
fn1:  mov rax,rdi            fn2:  xor eax,eax
      mov rcx,rdx                  mov rcx,-1
      shr rcx,3                    repnz scas al,[rdi]
      rep movs QWORD               not rcx
      mov ecx,edx                  lea rax,[rcx-1]
      and ecx,7                    ret
      rep movs BYTE
      ret

fn3:  mov rax,rdi            fn4:  mov rcx,rdx
      mov rcx,rdx                  xor eax,eax
      cmp rdi,rsi                  repz cmps [rsi],[rdi]
      jbe .forward                 je .same
      lea r8,[rsi+rdx]             movzx eax,[rdi-1]
      cmp rdi,r8                   movzx ecx,[rsi-1]
      jae .forward                 sub eax,ecx
      std                          ret
      lea rsi,[rsi+rdx-1]
      lea rdi,[rdi+rdx-1]    fn5:  mov eax,esi
      rep movs BYTE                mov rcx,rdx
      cld                          test rcx,rcx
      ret                          je .none
.forward: rep movs BYTE            repnz scas al,[rdi]
      ret                          jne .none
                                   lea rax,[rdi-1]
fn6:  mov r8,rdi                   ret
      movzx eax,sil
      mov rcx,rdx
      rep stos BYTE
      mov rax,r8
      ret
```

<div dir="rtl">

### التعرف على كلٍّ منها

| الروتين | الدليل | الدالة |
|---|---|---|
| `fn1` | تنسخ `rep movs` عدد `rcx` من الكلمات الرباعية ثم البايتات المتبقية؛ بلا معالجة للاتجاه؛ وتعيد `rdi` | `memcpy` |
| `fn2` | `al` صفر و`rcx` يساوي -1؛ وتمسح `repnz scas` حتى البايت الصفري؛ و`not rcx` مع طرح واحد يحوّلان العدّ إلى طول | `strlen` |
| `fn3` | تقارن العنوانين، وحين تكون الوجهة فوق المصدر وتتداخل معه تنفذ `std` وتنسخ من آخر بايت وتعيد `cld` | `memmove` |
| `fn4` | تتوقف `repz cmps` عند أول اختلاف؛ ويُطرح البايتان المختلفان لإعطاء النتيجة | `memcmp` |
| `fn5` | يأتي `al` من الوسيط الثاني و`rcx` من الثالث؛ وتتوقف `repnz scas` عند تطابق؛ وتعيد `rdi - 1` أو صفراً | `memchr` |
| `fn6` | يأخذ `al` البايت من الوسيط الثاني وتخزّنه `rep stos` عدد `rcx` من المرات | `memset` |

يختلف الروتينان اللذان يستخدمان `repnz scas` فيما يقارنان به وفيما يعيدانه: تبحث `fn2` عن صفر وتعيد عدداً، وتبحث `fn5` عن بايت يحدده المستدعي وتعيد مؤشراً. وتختلف النسختان في معالجة الاتجاه: `fn3` وحدها فيها `std` و`cld`. بترتيب strlen وmemchr وmemcpy وmemmove وmemset وmemcmp تكون الأرقام 2 و5 و1 و3 و6 و4.

العلم هو `ARL{sweeper_2_5_1_3_6_4}`.

## الجزء 2: Assemble

الحل المرجعي هو `solution.asm`، بالأشكال نفسها لـ `fn2` و`fn5` و`fn1` (مبسّطة إلى نسخ بايت بايت) و`fn3`. والنقاط المهمة:

- **`my_strlen`.** `al` تساوي 0، و`rcx` تساوي -1، ثم `repnz scasb`، ثم `not rcx` وطرح واحد.
- **`my_memchr`.** افحص حالة `n` تساوي 0 قبل المسح، لأن المسح لا يفعل شيئاً حين تكون `rcx` صفراً وتحتفظ راية الصفر بقيمتها السابقة أياً كانت. وبعد المسح تبيّن راية الصفر هل وُجد تطابق.
- **`my_memcpy`.** احفظ `rdi` لقيمة الإرجاع، وضع `n` في `rcx`، ونفّذ `rep movsb`.
- **`my_memmove`.** النسخ الأمامي آمن حين لا تكون الوجهة فوق المصدر، أو حين تبدأ عند نهاية المصدر أو بعدها. وإلا انسخ إلى الخلف بـ `std` وامسح الراية قبل العودة.

## أخطاء تضيّع الوقت

- **النسخ إلى الأمام دائماً في `my_memmove`.** تخرج حالات التداخل خاطئة؛ وتفشل 8 من حالات الاختبار الـ 74.
- **ترك راية الاتجاه مضبوطة.** يبلّغ المدقق `BAD direction flag left set`. وكانت تعليمة السلاسل التالية عند المستدعي ستعمل إلى الخلف.
- **نسخ بايتات أكثر من اللازم.** الكتابة بعد نهاية الكتلة تصطدم بالصفحة غير المتاحة وينهار الاختبار بالإشارة 11.
- **تجاوز فحص `n` تساوي 0 في `my_memchr`.** يستدعي الاختبار الروتين وراية الصفر مضبوطة و`n` تساوي 0؛ والروتين الذي يثق بالراية القديمة يعيد مؤشراً سيئاً ويفشل في تلك الحالة.
- **إعادة المؤشر بعد موضعه بمقدار بايت.** بعد التطابق يكون `rdi` أصلاً بعد البايت المطابق بواحد؛ وتفشل 11 من الحالات الـ 74.
- **عدّ المحرف الأخير في `my_strlen`.** تكون النتيجة أكبر بواحد؛ وتفشل 16 حالة، بدءاً بالسلسلة الفارغة.
- **استدعاء روتين آخر.** الاستدعاء `call` ممنوع؛ اجعل كل روتين مكتفياً بذاته.

</div>

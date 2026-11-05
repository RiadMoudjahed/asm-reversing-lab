<!-- source: quiz.md @ sha256:016737cf52bb -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

<div dir="rtl">

# اختبار قصير: 05. الحساب والعمليات المنطقية

**1. بعد `add rax, 1` حين تكون `rax = 0x7FFFFFFFFFFFFFFF`، أيٌّ من CF وOF تُضبط؟**

<details>
<summary>الإجابة</summary>

OF فقط. كجمع بلا إشارة لا يخرج أي حمل من السجل، لكن كجمع بإشارة تلتف النتيجة من أكبر قيمة موجبة إلى أصغر قيمة سالبة.

</details>

**2. لماذا يجب أن تأتي `cqo` قبل `idiv`؟**

<details>
<summary>الإجابة</summary>

تقسم `idiv` القيمة ذات 128 بت `rdx:rax`. تملأ `cqo` السجل `rdx` بتوسيع الإشارة لـ `rax`؛ وبدونها يحمل `rdx` قيمة لا علاقة لها وتكون النتيجة خاطئة.

</details>

**3. ماذا تكتب `mul rsi` عدا `rax`؟**

<details>
<summary>الإجابة</summary>

`rdx`. يُخزَّن حاصل الضرب ذو 128 بت في `rdx:rax`، فأي قيمة كنت تحتفظ بها في `rdx` تُمحى.

</details>

**4. حُسبت `-17 / 5` بـ `idiv`. ما الناتج وما الباقي؟**

<details>
<summary>الإجابة</summary>

الناتج -3 والباقي -2. تقرّب `idiv` نحو الصفر ويأخذ الباقي إشارة المقسوم.

</details>

**5. ما الفرق بين `sar` و`shr`؟**

<details>
<summary>الإجابة</summary>

تُدخل `shr` أصفاراً (إزاحة منطقية، للقيم بلا إشارة). أما `sar` فتنسخ بت الإشارة (إزاحة حسابية، للقيم بإشارة)، فيبقى العدد السالب سالباً.

</details>

**6. تحتوي دالة مترجمة على `shr rax, 3`. ما الذي فعلته شيفرة C المصدرية على الأرجح؟**

<details>
<summary>الإجابة</summary>

قسمت قيمة بلا إشارة على 8.

</details>

**7. لماذا يمكن أن تقع `mov [rdx], rdi` بين `add rdi, rsi` و`seto al`؟**

<details>
<summary>الإجابة</summary>

لا تغيّر `mov` أي راية، فتبقى OF تصف `add` حين تقرؤها `seto`.

</details>

**8. لماذا تفشل `daa` في التجميع في شيفرة 64 بت؟**

<details>
<summary>الإجابة</summary>

تعليمات الضبط العشري غير متاحة في وضع 64 بت. يبلّغ NASM بالرسالة `error: instruction not supported in 64-bit mode`.

</details>

</div>

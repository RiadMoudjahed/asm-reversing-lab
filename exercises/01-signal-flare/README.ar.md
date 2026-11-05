<!-- source: README.md @ sha256:09da60cefe79 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

<div dir="rtl">

# التمرين 01. إشارة الإنذار

Level 1. الوقت: نحو 45 دقيقة. المعمارية: x86-64 على Linux. الوحدات: [00. إعداد المختبر](../../modules/00-setup/README.ar.md)، [03. برنامجك الأول](../../modules/03-first-program/README.ar.md)

## السيناريو

برنامج إشارة صغير اسمه `signal_flare` يبلّغ عن إطلاقه ثم ينتهي. ضاعت شيفرته المصدرية، ويحتاج القائمون عليه إلى معرفة ما يفعله بالضبط ليعيدوا بناءه. لديك الملف التنفيذي ولا شيء غيره.

## الأهداف

- قراءة استدعاءات النظام لبرنامج باستخدام `strace` و`objdump`.
- استنتاج رقم كل استدعاء ووسائطه ونتيجته.
- إعادة بناء البرنامج بلغة التجميع باستخدام استدعاءات النظام الخام دون مكتبة C.

## الجزء 1: Reverse

الملف التنفيذي هو `bin/signal_flare`. شغّل هذه الأوامر من مجلد التمرين داخل المختبر:

</div>

```bash
file bin/signal_flare
strace ./bin/signal_flare
objdump -d -M intel bin/signal_flare
```

<div dir="rtl">

يسرد `strace` كل استدعاء نظام ينفذه البرنامج. ويعرض `objdump -d -M intel` التعليمات بصيغة Intel؛ والشيفرة التي تحتاجها تبدأ عند الوسم `_start`. تبيّن [ورقة مرجعية لاستدعاءات نظام Linux](../../cheatsheets/syscalls-linux.ar.md) معنى كل رقم استدعاء وأي سجل يحمل كل وسيط.

أجب عن الأسئلة التالية:

1. ما أرقام استدعاءات النظام التي يستخدمها البرنامج، بالترتيب؟
2. لكل `write`، ما واصف الملف المستخدم وكم بايتاً يرسل؟
3. ما حالة الخروج التي يعيدها البرنامج؟

ابنِ العلم من إجاباتك:

</div>

```text
ARL{flare_<bytes written to descriptor 1>_<bytes written to descriptor 2>_<exit status>}
```

<div dir="rtl">

مثلاً، لو كانت الأرقام 10 و20 و3 لكان العلم `ARL{flare_10_20_3}`.

## الجزء 2: Assemble

عدّل `starter/solution.asm` ليتصرف برنامجك تماماً مثل `signal_flare`:

- البايتات نفسها على الخرج القياسي.
- البايتات نفسها على خرج الأخطاء القياسي.
- حالة الخروج نفسها.

ابنِه وشغّله من مجلد `starter`:

</div>

```bash
make run
```

<div dir="rtl">

## القواعد

- استخدم استدعاءي النظام `write` و`exit` فقط. لا تربط مكتبة C.
- اكتب النص والاستدعاءات بنفسك. يجب ألا يشغّل برنامجك `signal_flare`.
- عرّف أطوال النصوص بواسطة NASM (`$ - label`) وليس بالعدّ اليدوي.

## تحقق من عملك

</div>

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

<div dir="rtl">

يعرض المدقق الجزء 1 والجزء 2 على حدة. وبدون `--flag` يفحص الجزء 2 فقط.

## التلميحات

افتح [HINTS.ar.md](HINTS.ar.md) مستوى واحداً في كل مرة.

</div>

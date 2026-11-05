<!-- source: README.md @ sha256:cbf3cf5f6a4f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

<div dir="rtl">

# 19. تحليل البرامج المشبوهة

الوقت: نحو 3 ساعات. المتطلبات السابقة: [18. تدقيق الملفات التنفيذية بحثاً عن الأخطاء](../18-auditing-binaries/README.ar.md)

## لماذا يهم هذا

يتلقى المدافع بانتظام ملفاً يشتبه فيه أحد: مرفقاً أو تنزيلاً أو برنامجاً وُجد على خادم. والمهمة أن يجيب عن أربعة أسئلة بسرعة وأمان. ما هو؟ ماذا كان سيفعل؟ كيف نتعرف عليه في مكان آخر؟ ماذا نقول للذين عليهم الاستجابة؟ تعلّم هذه الوحدة هذا العمل من جانب المدافع. وهي تتعلق بالتعرف على السلوك ووصفه كي يمكن كشفه. ولا تعلّم كتابة برمجيات كهذه. كل العينات في هذا المقرر محاكيات غير ضارة تطبع ما كانت ستفعله ولا شيء غيره؛ ولا تظهر برمجيات خبيثة حقيقية في هذا المستودع أبداً، ولا يجوز لك إدخالها إلى المختبر. اقرأ [بيان الأخلاقيات](../../ETHICS.md) قبل أن تبدأ.

## المفاهيم

### ممارسة التحليل الآمن

عامل كل برنامج مجهول على أنه خطر إلى أن تثبت العكس، ورتّب العمل بحيث لا يكلّف الخطأ شيئاً.

| القاعدة | السبب |
|---|---|
| لا تشغّل برنامجاً مجهولاً على حاسوبك | تشغيل واحد قد يغيّر ملفات أو يثبّت استمرارية أو يرسل بياناتك |
| اعمل في بيئة معزولة: آلة افتراضية أو حاوية بلا شبكة | بلا شبكة لا يستطيع البرنامج تلقي تعليمات ولا إخراج بيانات |
| خذ لقطة أولاً واستعدها بعد ذلك | تعود دائماً إلى حالة معروفة |
| لا تشارك المضيف في شيء: لا مجلدات مشتركة ولا حافظة ولا بيانات اعتماد | يجب ألا تستطيع العينة بلوغ أي شيء مهم |
| أبقِ العينات خاملة وهي ساكنة: غيّر الامتداد واحفظها في أرشيف ودوّن التجزئة | النقر المزدوج بالخطأ لا يفعل شيئاً |
| دوّن ما تفعله: الأوامر والأوقات والتجزئات والنتائج | لا يزيد التقرير جودةً على الملاحظات التي يقوم عليها |
| استخدم المحاكيات وحدها في هذا المقرر | كُتبت المحاكيات لتكون غير ضارة؛ أما العينات الحقيقية فتحتاج إلى بيئة احترافية وتفويض |

تعمل صورة المختبر في حاوية لا ترى إلا المجلد الذي تركّبه. وللتشغيل الأشد صرامة ركّب المجلد للقراءة فقط وعطّل الشبكة. وهذا أمر للمضيف:

</div>

```bash
docker run --rm --network none --read-only -v "$PWD":/lab:ro -w /lab asm-reversing-lab \
    modules/19-analyzing-suspicious-programs/examples/build/19_sim
```

<div dir="rtl">

حتى البرنامج غير الضار يُفحص قبل أن يوثق به. يعد المحاكي `examples/19_sim.c` بألا يفعل شيئاً سوى الطباعة. ويمكنك التحقق من الوعد من الخارج، دون النظر في الشيفرة، بالأداة `strace` التي تسرد استدعاءات النظام التي يجريها برنامج:

</div>

```bash
make
strace -f -e trace=execve,connect,socket,clone,openat,unlink,write build/19_sim 2>&1 >/dev/null
```

```text
execve("build/19_sim", ["build/19_sim"], 0x7fff3dd23cc8 /* 15 vars */) = 0
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
write(1, "[sim] simulator: no action is pe"..., 1018) = 1018
+++ exited with 0 +++
```

<div dir="rtl">

يحمّل البرنامج مكتبة C ويكتب 1018 بايتاً إلى خرجه القياسي ثم ينتهي. لا يبدأ عملية ولا يجري اتصالاً ولا يحذف شيئاً ولا يفتح ملفاً غير ملفات المحمّل. وكانت العينة الحقيقية ستُظهر العكس، وهذا هو نوع الأدلة الذي يقوم عليه الفرز وتحليل السلوك.

### الفرز: النظرة الأولى دون تشغيل شيء

يقرر الفرز إلى أي عمق تذهب. اجمع الحقائق الرخيصة أولاً. يجمعها النص `examples/19_triage.py` لملفات ELF وPE ولا يشغّل الملف أبداً:

</div>

```bash
python3 19_triage.py build/19_sim.exe
```

```text
file:     build/19_sim.exe
size:     251253 bytes
sha256:   656c3556a79829ebae38d56c31bc16b7f3b10174e89323862279217c015affe8
type:     PE (MZ header)
entropy:  5.89 bits per byte (above about 7.2 suggests packed or encrypted data)
imports:  48
    KERNEL32.dll!DeleteCriticalSection
    KERNEL32.dll!EnterCriticalSection
    KERNEL32.dll!GetLastError
    KERNEL32.dll!InitializeCriticalSection
    KERNEL32.dll!IsDBCSLeadByteEx
    KERNEL32.dll!LeaveCriticalSection
    KERNEL32.dll!MultiByteToWideChar
    KERNEL32.dll!SetUnhandledExceptionFilter
    KERNEL32.dll!Sleep
    KERNEL32.dll!TlsGetValue
    KERNEL32.dll!VirtualProtect
    KERNEL32.dll!VirtualQuery
    ... and 36 more (the C runtime and basic KERNEL32 functions)
strings worth a second look:
    [registry path] HKCU\Software\Microsoft\Windows\CurrentVersion\Run
    [command line] schtasks /create /tn "%s" /tr "%s" /sc daily /st 09:00
    [url or host] updates.contoso-cdn.test
    [windows path] C:\Users\Public\upd\updsvc.exe
    [process name] updsvc.exe
    [process name] explorer.exe
```

<div dir="rtl">

| الحقيقة | ما تخبرك به |
|---|---|
| التجزئة | اسم للملف. ابحث عنها في سجلاتك وفي قواعد البيانات العامة؛ وأبلغ عنها. للبناءات المختلفة تجزئات مختلفة، فالتطابق يثبت الهوية وعدم التطابق لا يثبت شيئاً |
| النوع والحجم | أي الأدوات تنطبق، وهل يلائم الحجم الغرض المعلن |
| الإنتروبيا | القرب من 8 بتات لكل بايت يعني محتوى مضغوطاً أو مشفراً، وغالباً ما يعني تعبئة؛ أما الشيفرة العادية والنص فيبقيان دون ذلك بكثير. انظر أيضاً إلى إنتروبيا كل قسم |
| الاستيرادات | الدوال التي يطلبها البرنامج من النظام. قد تعني قائمة استيرادات ضئيلة أن البرنامج يحل الدوال بنفسه؛ وقائمة فيها دوال العمليات والذاكرة والسجل والشبكة تقول ما يستطيع فعله |
| السلاسل النصية | مسارات ومفاتيح وأوامر ومضيفات. وهي أرخص دليل على الغرض وغالباً أفضله |

لاحظ ما يُظهره هذا الملف PE. استيراداته ليست إلا بيئة تشغيل C ودوال نظام أساسية، لأن المحاكي لا يستدعي شيئاً خطراً. ويظهر غرضه في السلاسل النصية وحدها: موضع استمرارية وأمر مهمة مجدولة ومسار إسقاط واسم مضيف واسم عملية.

### المؤشرات الساكنة

**المؤشر** حقيقة قابلة للملاحظة تدل على برنامج أو عائلة أو سلوك. وتأتي المؤشرات الساكنة من الملف دون تشغيله. أنواع مفيدة:

| النوع | أمثلة | احتراز |
|---|---|---|
| التجزئات | SHA-256 للملف | تنكسر مع أي تغيير في الملف |
| السلاسل النصية | مسارات السجل وأسطر الأوامر وأسماء الملفات وأسماء المضيفات وأسماء كائنات المزامنة وأسماء الأنابيب وسلاسل وكيل المستخدم | السلاسل الشائعة (تظهر مفاتيح `Run` في أدوات التوثيق) تحتاج إلى دعم من سلاسل أخرى |
| الاستيرادات والتصديرات | مجموعات من الدوال توحي معاً بقدرة | تستخدم البرمجيات المشروعة الدوال نفسها |
| البنية | أسماء الأقسام وأحجامها ونقطة دخول في قسم غير معتاد وإنتروبيا عالية ومورد هو ملف تنفيذي | المترجمات والمثبّتات تنتج أيضاً تخطيطات غريبة |
| أنماط الشيفرة | حلقة فك ترميز أو تسلسل استدعاءات أو جدول ثوابت | تحتاج إلى هندسة عكسية لتأكيدها |

نادراً ما يدين مؤشر واحد. أما عدة مؤشرات تتسق معاً فتدين. دوّن دائماً مدى دعم كل مؤشر ومدى احتمال ظهوره في ملفات بريئة.

### مؤشرات السلوك

**مؤشر السلوك** يأتي من مراقبة ما يفعله البرنامج في المختبر المعزول. انظر إلى نظام الملفات (ما يُنشأ أو يُغيَّر أو يُحذف وأين) والعمليات (ما يُبدأ وبأي سطر أوامر) والسجل في Windows (المفاتيح والقيم المكتوبة) والشبكة (محاولات الاتصال ولو فشلت والأسماء المستعلَم عنها) والذاكرة (العمليات الأخرى المفتوحة والذاكرة المحجوزة فيها). في Linux يعرض `strace -f` استدعاءات النظام؛ وفي Windows تؤدي مراقبات العمليات والملفات والسجل والشبكة العمل نفسه. ويطبع المحاكي سلوكه مباشرة:

</div>

```bash
make run
```

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would read the computer name, the user name and the OS version
[sim] would list the running processes
[sim] would open the key HKCU\Software\Microsoft\Windows\CurrentVersion\Run
[sim] would set the value "Updater" to "C:\Users\Public\upd\updsvc.exe"
[sim] would run: schtasks /create /tn "GoogleUpdateTaskMachineQC" /tr "C:\Users\Public\upd\updsvc.exe" /sc daily /st 09:00
[sim] would open the process explorer.exe with write access
[sim] would allocate 4096 bytes of executable memory in it
[sim] would write the payload into that memory
[sim] would start a remote thread at the payload
[sim] would open the access token of a system process
[sim] would duplicate the token as a primary token
[sim] would start C:\Users\Public\upd\updsvc.exe with the duplicated token
[sim] would send an HTTP GET to http://updates.contoso-cdn.test:8443/v2/check?id=<host id> every 60 seconds
[sim] would write the reply to C:\Users\Public\upd\updsvc.exe
```

<div dir="rtl">

الترتيب مفيد. وترتيب أفعال عينة حقيقية ظاهر في الشيفرة أيضاً: في هذا البناء تستدعي `main` ست دوال الواحدة بعد الأخرى، وتفضحها الأسماء (والعينة الحقيقية المجردة من الرموز لن تكون لها أسماء، لكن لها الترتيب نفسه ووسائط السلاسل النصية نفسها):

</div>

```text
00000000004013da <main>:
  4013e2:  mov    edi,0x402300
  4013e7:  call   puts                      ; the banner
  4013ec:  call   sim_discover
  4013f1:  call   sim_persist_runkey
  4013f6:  call   sim_persist_task
  4013fb:  call   sim_inject
  401400:  call   sim_token
  401405:  call   sim_beacon
```

<div dir="rtl">

### الربط بـ ATT&CK

MITRE ATT&CK فهرس عام للتقنيات المعروف أن المهاجمين يستخدمونها. وهو ينظمها بحسب **التكتيك**، أي الهدف (الاستمرارية والتهرب من الدفاعات والقيادة والتحكم)، ويعطي كل **تقنية** رقماً مثل T1547 وتحتها **تقنيات فرعية** مثل T1547.001. وربط ملاحظاتك بهذه الأرقام يعطي الجميع المفردات نفسها للتقارير وللكشوف ولأسئلة التغطية. اربط السلوك لا الأدوات: اسأل عما يفعله البرنامج وابحث عن التقنية التي تصفه، ثم اقرأ الوصف الرسمي لتتأكد من ملاءمتها ولتعرف كيف تُكشف التقنية.

| الملاحظة | التقنية | التكتيك |
|---|---|---|
| قيمة مكتوبة تحت مفتاح `Run` في السجل تبدأ برنامجاً عند تسجيل الدخول | T1547.001 Registry Run Keys / Startup Folder | الاستمرارية |
| مهمة مجدولة أُنشئت بـ `schtasks` أو بواجهات مجدول المهام | T1053.005 Scheduled Task | الاستمرارية، التنفيذ |
| برنامج يكتب في ذاكرة عملية أخرى ويبدأ فيها مساراً | T1055 Process Injection (T1055.001 DLL injection، T1055.002 portable executable injection، T1055.003 thread execution hijacking، T1055.004 asynchronous procedure call، T1055.012 process hollowing) | التهرب من الدفاعات، رفع الامتيازات |
| رمز وصول (token) نُسخ أو استُعير من عملية أخرى واستُخدم لبدء عملية | T1134 Access Token Manipulation (T1134.001 token impersonation or theft، T1134.002 create process with token) | التهرب من الدفاعات، رفع الامتيازات |
| برنامج يجعل برنامجاً موثوقاً يحمّل مكتبته الخاصة | T1574 Hijack Execution Flow (T1574.001 DLL search order hijacking، T1574.002 DLL side-loading) | الاستمرارية، رفع الامتيازات، التهرب من الدفاعات |
| قراءة اسم الحاسوب والمستخدم وإصدار نظام التشغيل | T1082 System Information Discovery | الاستكشاف |
| سرد العمليات الجارية | T1057 Process Discovery | الاستكشاف |
| طلبات HTTP إلى خادم على فترات | T1071.001 Application Layer Protocol: Web Protocols | القيادة والتحكم |
| تنزيل ملف لتشغيله لاحقاً | T1105 Ingress Tool Transfer | القيادة والتحكم |
| أوامر تُنفَّذ عبر مفسر الأوامر | T1059.003 Windows Command Shell | التنفيذ |
| سلاسل نصية أو شيفرة تُرمَّز وتُفك في وقت التشغيل | T1027 Obfuscated Files or Information، T1140 Deobfuscate/Decode Files or Information | التهرب من الدفاعات |

للمحاكي، يُربط الخرج أعلاه بـ T1082 وT1057 (سطرا الاستكشاف) وT1547.001 (المفتاح والقيمة) وT1053.005 (الأمر `schtasks`) وT1055 (فتح عملية وحجز ذاكرة فيها والكتابة فيها وبدء مسار بعيد: عائلة الحقن الموصوفة في الجدول أدناه) وT1134 (أسطر الرمز) وT1071.001 (طلب HTTP الدوري). اكتب الربط في جدول بجانب الدليل لكل سطر. وإذا لاءم سلوك عدة تقنيات فاذكر الأقرب وعلّل ذلك.

### أنماط استدعاءات واجهة البرمجة وراء التقنيات

في Windows يصل البرنامج إلى النظام عبر دوال واجهة البرمجة (API)، وتحتاج كل تقنية إلى مجموعة مميزة منها. ومعرفة المجموعات تتيح التعرف على التقنية في قائمة استيرادات أو في ترتيب الاستدعاءات في التفكيك أو في سجل مراقب. هذه أنماط للتعرف والكشف. والبرمجيات المشروعة، مثل المثبّتات ومصححات الأخطاء وأدوات الأمان وأدوات النظام، تستخدم كثيراً من الدوال نفسها، فالنمط سبب لتدقيق النظر وليس حكماً أبداً.

| التقنية | الاستدعاءات المميزة بالترتيب المعتاد | ما يُبحث عنه أيضاً |
|---|---|---|
| مفتاح Run في السجل (T1547.001) | `RegCreateKeyEx` أو `RegOpenKeyEx` ثم `RegSetValueEx` ومسار بوصفه البيانات | مسار المفتاح (`...\CurrentVersion\Run` و`RunOnce`) ومسار إلى ملف في مجلد يستطيع المستخدم الكتابة فيه |
| مهمة مجدولة (T1053.005) | `CreateProcess` أو `ShellExecute` لـ `schtasks /create` أو `CoCreateInstance` لواجهات مجدول المهام | سطر الأوامر الذي فيه `/tr` و`/sc` و`/tn` واسم مهمة يقلّد منتجاً معروفاً |
| حقن DLL (T1055.001) | `OpenProcess` و`VirtualAllocEx` و`WriteProcessMemory` و`CreateRemoteThread` (وغالباً يكون عنوان البدء عنوان `LoadLibrary`) | اسم العملية الهدف ومسار مكتبة في الذاكرة المكتوبة |
| حقن الشيفرة (T1055.002) | الاستدعاءات الثلاثة الأولى نفسها يليها مسار عنوان بدئه داخل الذاكرة المحجوزة | أذونات التنفيذ (`PAGE_EXECUTE_READWRITE`) على الحجز |
| اختطاف المسار (T1055.003) | `OpenThread` و`SuspendThread` و`GetThreadContext` و`SetThreadContext` و`ResumeThread` | مؤشر تعليمات متغير في السياق |
| حقن APC (T1055.004) | `OpenProcess` و`VirtualAllocEx` و`WriteProcessMemory` و`QueueUserAPC` | مسار في حالة قابلة للتنبيه |
| تفريغ العملية (T1055.012) | `CreateProcess` مع راية الإيقاف المؤقت وفك ربط الصورة الأصلية (`NtUnmapViewOfSection`) و`WriteProcessMemory` و`SetThreadContext` و`ResumeThread` | عملية تختلف صورتها على القرص عما يعمل |
| التلاعب برموز الوصول (T1134) | `OpenProcessToken` و`DuplicateTokenEx` ثم `ImpersonateLoggedOnUser` أو `SetThreadToken` أو `CreateProcessWithTokenW`؛ و`AdjustTokenPrivileges` لتفعيل امتياز | طلب امتياز التنقيح أو انتحال الصفة |
| اختطاف ترتيب بحث DLL والتحميل الجانبي (T1574.001 وT1574.002) | `LoadLibrary` باسم مكتبة مجرد، وملف تنفيذي يبدأ من مجلد يحوي أيضاً مكتبة باسم متوقع | مكتبة جديدة كُتبت بجانب ملف تنفيذي موثوق في مجلد يستطيع المستخدم الكتابة فيه |

وفي Linux تستخدم العائلات المماثلة استدعاءات أخرى (`ptrace` و`process_vm_writev` للكتابة في عملية أخرى، و`LD_PRELOAD` ومسارات بحث المكتبات للاختطاف، ومدخلات cron ووحدات systemd للاستمرارية)، والطريقة هي نفسها: جمّع الاستدعاءات ورتّبها وابحث عن الدليل في السلاسل النصية والوسائط.

### ملاحظات الكشف

**ملاحظة الكشف** تخبر من يراقبون الأنظمة بما يبحثون عنه. لكل سلوك دوّن ما يُلاحظ وأين، ولماذا هو مشبوه، وأي نشاط مشروع يشبهه، وإلى أي تقنية يُربط. ومثال قصير للمحاكي:

</div>

```text
Behavior:     A value is added under HKCU\Software\Microsoft\Windows\CurrentVersion\Run
              that points to a program in C:\Users\Public.
Technique:    T1547.001 Registry Run Keys / Startup Folder
Where to see: Registry auditing and endpoint monitoring events for registry value
              writes, the process that made the write, the file at the path.
Why suspicious: Run values normally point to program folders; a path in a public,
              user-writable folder is unusual.
False positives: Installers of user-level applications also add Run values. Check
              the signer and the folder.
Response:     Collect the file, hash it, check where it came from, remove the value
              after the evidence is saved.
```

<div dir="rtl">

المصادر الشائعة للأحداث في هذه الملاحظات: أحداث إنشاء العمليات (مع سطر الأوامر الكامل) وأحداث الكتابة في السجل وأحداث إنشاء المهام من مجدول المهام وإنشاء المسارات البعيدة وأحداث الوصول إلى العمليات من مراقبات النقاط الطرفية وأحداث إنشاء الملفات. سمِّ المصدر في الملاحظة، فالكشف الذي لا يراه أحد لا قيمة له.

### كتابة قاعدة YARA

تطابق YARA الملفات بالأنماط. للقاعدة اسم وبيانات وصفية وسلاسل وشرط:

</div>

```text
rule name
{
    meta:      description, author, references
    strings:   $a = "text"  $b = { 6A 40 68 ?? ?? }  $c = /regex/
    condition: boolean expression over the strings and the file
}
```

<div dir="rtl">

في الملف `examples/19_sim.yar` نسختان من قاعدة للمحاكي. الأولى سهلة الكتابة:

</div>

```text
rule sim_updater_v1_weak
{
    strings:
        $run = "CurrentVersion\\Run"
    condition:
        $run
}
```

<div dir="rtl">

تطابق المحاكي، وتطابق كذلك البرنامج المساعد غير الضار (`19_helper.c`) الذي ذنبه الوحيد أن نص مساعدته يذكر مسار السجل نفسه. واختبار القاعدة على ملفات نظيفة بأهمية اختبارها على العينة:

</div>

```bash
for f in build/19_sim build/19_sim.exe build/19_helper build/19_helper.exe /bin/ls; do yara 19_sim.yar $f; done
```

```text
sim_updater_v1_weak build/19_sim
sim_updater_v2 build/19_sim
sim_updater_v1_weak build/19_sim.exe
sim_updater_v2 build/19_sim.exe
sim_updater_v1_weak build/19_helper
sim_updater_v1_weak build/19_helper.exe
```

<div dir="rtl">

تنطلق النسخة 1 على البرنامجين المساعدين، وهذا إيجابي كاذب. أما النسخة 2 فتشترط عدة سلاسل مميزة معاً وفحصاً لنوع الملف:

</div>

```text
rule sim_updater_v2
{
    meta:
        description = "Harmless updater simulator from module 19"
        technique = "T1547.001, T1053.005, T1055, T1134, T1071.001"
    strings:
        $runkey = "CurrentVersion\\Run"
        $task   = "schtasks /create /tn" ascii
        $drop   = "\\Users\\Public\\upd\\updsvc.exe" ascii
        $host   = "updates.contoso-cdn.test" ascii
        $tag    = "[sim] would" ascii
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($runkey, $task, $drop, $host) and $tag
}
```

<div dir="rtl">

لا تطابق إلا بناءي المحاكي. ويبيّن الخيار `-s` أي السلاسل تطابقت وأين، وبهذا تُنقَّح القاعدة:

</div>

```bash
yara -s 19_sim.yar build/19_sim
```

```text
sim_updater_v1_weak build/19_sim
0x2150:$run: CurrentVersion\Run
sim_updater_v2 build/19_sim
0x2150:$runkey: CurrentVersion\Run
0x21ad:$task: schtasks /create /tn
0x216a:$drop: \Users\Public\upd\updsvc.exe
0x2071:$host: updates.contoso-cdn.test
0x2004:$tag: [sim] would
```

<div dir="rtl">

إرشادات للقواعد:

- طابق ما هو مميز: مسارات وأسماء غير معتادة وأسطر أوامر وثوابت وتسلسلات شيفرة قصيرة بأحرف بديلة. وتجنب السلاسل التي يملكها كل برنامج من هذا النوع.
- اجمع السلاسل بـ `N of` أو `all of` أو `and`، وأضف فحص نوع الملف بـ `uint16(0) == 0x5a4d` (الحرفان MZ) لملفات PE أو `uint32(0) == 0x464c457f` لملفات ELF.
- أضف `ascii` أو `wide` أو كليهما حين يمكن للملف تخزين النص بأي من الشكلين.
- اختبر على مجموعة من الملفات النظيفة وعلى عدة عينات. ودوّن الغرض من كل قاعدة.
- تتوقف القاعدة المبنية على السلاسل عن العمل حين يغيّر المؤلف السلاسل. فضّل عدة خصائص مستقلة على واحدة.
- ضع أرقام التقنيات في البيانات الوصفية ليمكن العثور على القاعدة بحسب التغطية.

## جرّب

شغّل أوامر المختبر من `modules/19-analyzing-suspicious-programs/examples`.

1. ابنِ بالأمر `make`. شغّل نص الفرز على `build/19_sim` وعلى `build/19_sim.exe` وعلى `/bin/ls`. قارن الاستيرادات والإنتروبيا والسلاسل النصية.
2. شغّل `strace -f -c build/19_sim` واقرأ ملخص استدعاءات النظام. أي الاستدعاءات تتوقعها من برنامج ثبّت استمراريته واتصل فعلاً؟
3. اصنع جدولاً من `make run`: صفاً لكل سطر `[sim]` فيه معرّف التقنية والتكتيك والدليل. وتحقق من كل معرّف بوصف التقنية في موقع ATT&CK.
4. اكتب قاعدة YARA تطابق `19_sim` و`19_sim.exe` ولا تطابق `19_helper`، بسلاسل مختلفة عن التي في `19_sim.yar`. واختبرها بـ `yara -s`.
5. اكتب ملاحظة كشف لسلوك المهمة المجدولة بالصيغة أعلاه.
6. اشرح لماذا ستطابق قاعدة تطابق السلاسل `OpenProcess` و`VirtualAllocEx` و`WriteProcessMemory` و`CreateRemoteThread` بعض البرامج المشروعة أيضاً، وما الذي يمكنك إضافته إلى الشرط لجعلها أشد تحديداً.

## أخطاء شائعة

- **التشغيل أولاً.** لا تشغّل شيئاً قبل الفحوص الرخيصة، ولا تشغّله إلا في المختبر المعزول.
- **الوثوق بالتسمية.** اسم الملف أو أيقونته أو توقيعه ليست دليلاً على ما يفعله البرنامج.
- **قراءة مؤشر واحد على أنه إثبات.** مسار السجل في سلسلة نصية توثيق في ملف واستمرارية في آخر. ابحث عن مجموعة.
- **ربط الأدوات بدل السلوك.** السؤال هو ما يفعله البرنامج لا ما يُسمّى به.
- **كتابة قاعدة لا تلائم إلا العينة.** القاعدة المبنية على تجزئة أو على سلسلة واحدة تنكسر مع البناء التالي. اختبرها للإيجابيات الكاذبة وللمتانة.
- **إهمال الملاحظات.** بلا أوامر وأوقات وتجزئات لا يستطيع أحد تكرار النتائج أو التحقق منها.

## الخلاصة

- حلّل البرامج المجهولة في بيئة معزولة بلا شبكة وبلقطة فقط، وتحقق حتى من المحاكي غير الضار من الخارج بتتبع استدعاءات النظام.
- يجمع الفرز التجزئات والنوع والإنتروبيا والاستيرادات والسلاسل النصية دون تشغيل الملف ويقرر إلى أي عمق تذهب.
- تأتي المؤشرات الساكنة من الملف ومؤشرات السلوك من مراقبته أثناء التشغيل؛ وعدة مؤشرات تتسق معاً تساوي أكثر من واحد.
- يعطي ATT&CK مفردات مشتركة: اربط كل سلوك ملاحَظ بتكتيك وتقنية مع الدليل.
- لكل تقنية مجموعة مميزة من استدعاءات واجهة البرمجة، وهي سبب لتدقيق النظر وليست حكماً أبداً.
- تحوّل ملاحظات الكشف وقواعد YARA النتائج إلى شيء يستطيع الآخرون استخدامه، ويجب اختبار كليهما على ملفات نظيفة.

## الخطوات التالية

- أجرِ [الاختبار القصير](quiz.ar.md).
- تابع مع الوحدة 20، قراءة الشيفرة المموَّهة.
- التمارين التي تستخدم هذه الوحدة: [التمرين 19](../../exercises/19-triage-desk/README.ar.md).

</div>

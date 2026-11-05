[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 19. Analyzing suspicious programs

Answer first, then open the answer.

**1. Name four precautions for analyzing an unknown program.**

<details>
<summary>Answer</summary>

Work in an isolated environment (a virtual machine or container) with no network, take a snapshot first and restore it afterwards, share nothing with the host (folders, clipboard, credentials), and keep samples inert at rest (changed extension or an archive) with their hashes recorded. Recording commands and findings is part of the work too.

</details>

**2. A simulator claims to print and nothing else. How can you check this without reading its code?**

<details>
<summary>Answer</summary>

Run it in the lab under a system-call tracer such as `strace -f` and look at the calls: a program that only prints loads its libraries, writes to its output and exits. Any process creation, network connection, file deletion or unexpected file access would appear in the trace.

</details>

**3. What does a high entropy of an executable suggest, and why is it not proof?**

<details>
<summary>Answer</summary>

Content close to 8 bits per byte means the data is compressed or encrypted, which often indicates a packed program. It is not proof because compressed resources, installers and legitimate protected software also have high entropy, and a sample can be packed in ways that keep the entropy lower.

</details>

**4. Why is a registry path such as `CurrentVersion\Run` found in a file not enough to call it malicious, and what do you add?**

<details>
<summary>Answer</summary>

Documentation tools, installers and legitimate utilities mention or use the same path. Add other evidence that fits together: a scheduled-task command, a drop path in a public folder, a network host, the order of behavior. A group of indicators is worth more than one.

</details>

**5. Which technique ID covers a program that adds a value under a Run key, and which covers a program that creates a scheduled task?**

<details>
<summary>Answer</summary>

T1547.001 (Registry Run Keys / Startup Folder) for the Run key value, and T1053.005 (Scheduled Task) for the scheduled task. Both are persistence techniques.

</details>

**6. Which sequence of API calls is characteristic of classic DLL injection, and which technique ID does it map to?**

<details>
<summary>Answer</summary>

`OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory` and `CreateRemoteThread`, where the thread often starts at the address of `LoadLibrary` and the written memory holds the path of a library. It maps to T1055.001 (DLL injection) under T1055 (Process Injection). The calls alone are not proof, since debuggers and other legitimate tools use them too.

</details>

**7. A YARA rule consists of a single common string and it matches your sample. What should you do before you use it?**

<details>
<summary>Answer</summary>

Test it against clean files, because a common string will match legitimate programs (a false positive). Then make it specific: require several distinctive strings together, add a file-type test such as the MZ or ELF magic, and test again on the sample and on the clean collection.

</details>

**8. What does a good detection note contain?**

<details>
<summary>Answer</summary>

The behavior, the technique ID, where it can be observed (the event source), why it is suspicious, which legitimate activity looks similar (false positives) and what to do in response. Without a named event source, nobody can act on it.

</details>

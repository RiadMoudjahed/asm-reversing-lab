// Exercise 19, Part 1: write a YARA rule that matches the simulator in bin/ (the Linux and the
// Windows build) and none of the other programs the checker tries.
//
// Run:  make check EX=19 ARGS='--flag ARL{your_answer_here}'
// The rule below matches nothing. Replace it.

rule my_rule
{
    strings:
        $a = "replace this text"
    condition:
        $a
}

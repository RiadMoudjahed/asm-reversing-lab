#!/usr/bin/env bash
# Purpose : verify that every tool the course needs is installed.
# Target  : the lab image or a native install (Ubuntu/Debian).
# Build   : none. Run: bash check_tools.sh
# Output  : one line per tool ("ok" or "MISSING"); exit status 1 if any is missing.
set -u

tools=(nasm ld gcc g++ make gdb objdump readelf strace ltrace rizin yara python3 xxd file)
missing=0

for tool in "${tools[@]}"; do
  if command -v "$tool" > /dev/null 2>&1; then
    printf '%-10s ok\n' "$tool"
  else
    printf '%-10s MISSING\n' "$tool"
    missing=1
  fi
done

# The 32-bit toolchain needs the multilib packages; test it by compiling.
if echo 'int main(void){return 0;}' | gcc -m32 -x c - -o /dev/null 2> /dev/null; then
  printf '%-10s ok\n' "gcc -m32"
else
  printf '%-10s MISSING\n' "gcc -m32"
  missing=1
fi

exit "$missing"

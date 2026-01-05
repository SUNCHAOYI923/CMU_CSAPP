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

## Part B

To fix the `undefined reference to matherr` error, **comment out** the unused `matherr` function declaration in `ssim.c`.


The goal is to extend the SEQ processor to support the iaddq instruction, which adds an immediate value to a register `iaddq V, rB` → `rB = rB + V`.

### Fetch Stage

```hcl
bool instr_valid = icode in 
	{ INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
	       IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ, IIADDQ };
bool need_regids =
	icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, 
		     IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ };
bool need_valC =
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL,IIADDQ };
```

- Declare `IIADDQ` as a valid instruction so the processor recognizes it.

- `IIADDQ` needs a register byte to specify `rB` (the destination register).

- `IIADDQ` requires an immediate value `V`, which is stored in the constant word `valC`.

### Decode Stage

```hcl
word srcB = [
	icode in { IOPQ, IRMMOVQ, IMRMOVQ, IIADDQ } : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't need register
];
word dstE = [
	icode in { IRRMOVQ } && Cnd : rB;
	icode in { IIRMOVQ, IOPQ, IIADDQ } : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't write any register
];
```

- `IIADDQ` needs to read register `rB` to get its current value (`valB = Reg[rB]`).

- `IIADDQ` writes the result back to register `rB` (through `dstE`).

### Execute Stage

```hcl
word aluA = [
	icode in { IRRMOVQ, IOPQ } : valA;
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ } : valC;
	icode in { ICALL, IPUSHQ } : -8;
	icode in { IRET, IPOPQ } : 8;
	# Other instructions don't need ALU
];

## Select input B to ALU
word aluB = [
	icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, 
		      IPUSHQ, IRET, IPOPQ, IIADDQ } : valB;
	icode in { IRRMOVQ, IIRMOVQ } : 0;
	# Other instructions don't need ALU
];
bool set_cc = icode in { IOPQ, IIADDQ };
```

- For `IIADDQ`, `aluA` uses the immediate value `valC`.

- `aluB` uses `valB` (the current value of register `rB`).

- Like arithmetic operations (`IOPQ`), iaddq should update the condition codes (`ZF`, `SF`, `OF`).

### Memory Stage & Program Counter Update 

No need to update.

### Run Verification Tests

```bash
(cd ../y86-code; make testssim)
(cd ../ptest; make SIM=../seq/ssim)
(cd ../ptest; make SIM=../seq/ssim TFLAGS=-i)
```

### Expected Output

All tests should pass with messages like:

```bash
asum.seq:ISA Check Succeeds
asumr.seq:ISA Check Succeeds
cjr.seq:ISA Check Succeeds
j-cc.seq:ISA Check Succeeds
poptest.seq:ISA Check Succeeds
prog1.seq:ISA Check Succeeds
prog2.seq:ISA Check Succeeds
prog3.seq:ISA Check Succeeds
prog4.seq:ISA Check Succeeds
prog5.seq:ISA Check Succeeds
prog6.seq:ISA Check Succeeds
prog7.seq:ISA Check Succeeds
prog8.seq:ISA Check Succeeds
pushquestion.seq:ISA Check Succeeds
pushtest.seq:ISA Check Succeeds
ret-hazard.seq:ISA Check Succeeds

All 49 ISA Checks Succeed
All 64 ISA Checks Succeed
All 22 ISA Checks Succeed
All 600 ISA Checks Succeed

All 58 ISA Checks Succeed
All 96 ISA Checks Succeed
All 22 ISA Checks Succeed
All 756 ISA Checks Succeed
```

## Part C

### Command

#### Compilation

Note that each time you modify your `pipe-full.hcl` file, you can rebuild the simulator by typing `make psim VERSION=full`. Each time you modify your `ncopy.ys` program, you can rebuild the driver programs by typing `make drivers`. You can type `make VERSION=full` to rebuild the simulator and the driver programs.

#### Test `pipe-full.hcl`

```bash
cd ../ptest; make SIM=../pipe/psim
cd ../ptest; make SIM=../pipe/psim TFLAGS=-i
```

#### Test your code on a range of block lengths with the ISA simulator

```bash
./correctness.pl
```
#### Partial Score

```bash
./benchmark.pl
```

### Refference Solution

- Some suggestions in the pdf

    > Reordering instructions, replacing groups of instructions with single instructions, deleting some instructions, and adding other instructions. You may ﬁnd it useful to read about loop unrolling.

First, add `IIADDQ` instruction support to `pipe-full.hcl` as required in Part B. Before proceeding to the next step, ensure that your implementation passes all tests similiar to Part B.

Original CPE `Average CPE     15.18`. Then try to use `IIADDQ` in the `ncopy.ys`:

```assembly
Loop:	
	mrmovq (%rdi), %r10	# read val from src...
	rmmovq %r10, (%rsi)	# ...and store it to dst
	andq %r10, %r10		# val <= 0?
	jle Npos		# if so, goto Npos:
	iaddq $1, %rax		# count++
Npos:	
	iaddq $-1, %rdx		# len--
	iaddq $8, %rdi		# src++
	iaddq $8, %rsi		# dst++
	andq %rdx,%rdx		# len > 0?
	jg Loop			# if so, goto Loop:
```

The current implementation achieves an ``Average CPE of 12.70``, but the performance score remains at ``0.0/60.0``.

Loop unrolling was attempted to improve performance. After experimentation, $4 \times$ loop unrolling yields with ``Average CPE of 10.79``.

1. Default Register Initialization

    In Y86-64, registers are initialized to zero by default. Therefore, the explicit `xorq %rax,%rax` instruction can be omitted, reducing the instruction count.
    
2. Loop Unrolling

    Subtract the unroll factor from the length counter. If the result is negative, handle remaining elements separately. Otherwise, restore the counter and execute the unrolled loop.

    ```assembly
        andq %rdx,%rdx
        jle Done

    Loop:	
        iaddq $-4, %rdx
        jl add
    work1:
        iaddq $4, %rdx
        mrmovq (%rdi),%r8
        rmmovq %r8, (%rsi)
        andq %r8, %r8
        jle work2
        iaddq $1, %rax
    work2:
        mrmovq 8(%rdi),%r8
        rmmovq %r8, 8(%rsi)
        andq %r8, %r8
        jle work3
        iaddq $1, %rax
    work3:
        mrmovq 16(%rdi),%r8
        rmmovq %r8, 16(%rsi)
        andq %r8, %r8
        jle work4
        iaddq $1, %rax
    work4:
        mrmovq 24(%rdi),%r8
        rmmovq %r8, 24(%rsi)
        andq %r8, %r8
        jle modify
        iaddq $1, %rax
    modify:
        iaddq $32,%rdi
        iaddq $32,%rsi
        iaddq $-4,%rdx
        jge Loop
    add:
        iaddq $4, %rdx
    remain:
        je Done
        mrmovq (%rdi), %r10
        rmmovq %r10, (%rsi)
        andq %r10, %r10
        jle Npos
        iaddq $1, %rax
    Npos:	
        iaddq $-1, %rdx
        iaddq $8, %rdi
        iaddq $8, %rsi
        andq %rdx,%rdx
        jg remain	
    ```

3. Handle Data hazards

    To reduce data hazards in the pipeline, we employ **6-way loop unrolling** with **early loading** of data values by using additional registers (`%r8`, `%r9`, `%r10`, `%r11`, $\cdots$) to pre‑fetch memory operands. The optimized implementation achieves an `average CPE of 7.96`.

    ```assembly
    ncopy:
        iaddq $-6, %rdx
        jl res1

    work1:	
        mrmovq (%rdi),%r8
        mrmovq 8(%rdi),%r9
        rmmovq %r8, (%rsi)
        andq %r8, %r8
        jle work2
        iaddq $1, %rax
    work2:
        rmmovq %r9,8(%rsi)
        mrmovq 16(%rdi),%r10
        andq %r9, %r9
        jle work3
        iaddq $1, %rax
    work3:
        rmmovq %r10,16(%rsi)
        mrmovq 24(%rdi),%r11
        andq %r10, %r10
        jle work4
        iaddq $1, %rax
    work4:
        rmmovq %r11,24(%rsi)
        mrmovq 32(%rdi),%r8
        andq %r11, %r11
        jle work5
        iaddq $1, %rax
    work5:
        rmmovq %r8,32(%rsi)
        mrmovq 40(%rdi),%r9
        andq %r8, %r8
        jle work6
        iaddq $1, %rax
    work6:
        rmmovq %r9,40(%rsi)
        andq %r9, %r9
        jle modify
        iaddq $1, %rax
    modify:
        iaddq $48,%rdi
        iaddq $48,%rsi
        iaddq $-6,%rdx
        jge work1
    res1:
        iaddq $5, %rdx
        jl Done
        mrmovq (%rdi), %r8
        mrmovq 8(%rdi), %r9
        rmmovq %r8, (%rsi)
        andq %r8, %r8
        jle res2
        iaddq $1, %rax
    res2:	
        iaddq $-1, %rdx
        jl Done
        mrmovq 16(%rdi), %r10
        rmmovq %r9, 8(%rsi)
        andq %r9, %r9
        jle res3
        iaddq $1, %rax
    res3:	
        iaddq $-1, %rdx
        jl Done
        mrmovq 24(%rdi), %r11
        rmmovq %r10, 16(%rsi)
        andq %r10, %r10
        jle res4
        iaddq $1, %rax
    res4:	
        iaddq $-1, %rdx
        jl Done
        mrmovq 32(%rdi), %r8
        rmmovq %r11, 24(%rsi)
        andq %r11, %r11
        jle res5
        iaddq $1, %rax
    res5:	
        iaddq $-1, %rdx
        jl Done
        rmmovq %r8, 32(%rsi)
        andq %r8, %r8
        jle Done
        iaddq $1, %rax
    ```

4. Optimaze the Remaining Part

    A **tree-like comparison structure** reduces branch instructions by hierarchically testing the remaining length, cutting down the average number of comparisons per iteration. Furthermore, by reordering certain comparison operations, the overall code size has been reduced, enabling higher execution efficiency within the strict byte constraint.

    ```assembly
    iaddq $-10, %rdx
        jl f09
    pre:
        mrmovq (%rdi), %r8
        mrmovq 8(%rdi), %r9
        mrmovq 16(%rdi), %r10
        mrmovq 24(%rdi), %r11
        mrmovq 32(%rdi), %r12
        mrmovq 40(%rdi), %r13
        mrmovq 48(%rdi), %r14
        mrmovq 56(%rdi), %rbx
        mrmovq 64(%rdi), %rcx
        mrmovq 72(%rdi), %rbp
    work1:	
        rmmovq %r8, (%rsi)
        andq %r8, %r8
        jle work2
        iaddq $1, %rax
    work2:
        rmmovq %r9, 8(%rsi)
        andq %r9, %r9
        jle work3
        iaddq $1, %rax
    work3:
        rmmovq %r10, 16(%rsi)
        andq %r10, %r10
        jle work4
        iaddq $1, %rax
    work4:
        rmmovq %r11, 24(%rsi)
        andq %r11, %r11
        jle work5
        iaddq $1, %rax
    work5:
        rmmovq %r12, 32(%rsi)
        andq %r12, %r12
        jle work6
        iaddq $1, %rax
    work6:
        rmmovq %r13, 40(%rsi)
        andq %r13, %r13
        jle work7
        iaddq $1, %rax
    work7:
        rmmovq %r14, 48(%rsi)
        andq %r14, %r14
        jle work8
        iaddq $1, %rax
    work8:
        rmmovq %rbx, 56(%rsi)
        andq %rbx, %rbx
        jle work9
        iaddq $1, %rax
    work9:
        rmmovq %rcx, 64(%rsi)
        andq %rcx, %rcx
        jle work10
        iaddq $1, %rax
    work10:
        rmmovq %rbp, 72(%rsi)
        andq %rbp, %rbp
        jle modify
        iaddq $1, %rax
    modify:
        iaddq $0x50,%rdi
        iaddq $0x50,%rsi
        iaddq $-10,%rdx
        jge pre
    f09:
        iaddq $7, %rdx
        jg f49
        jl f02
        je rem3
    f02:
        iaddq $2, %rdx
        je rem1
        jg rem2
        jl Done
    f46:
        iaddq $2, %rdx
        jl rem4
        je rem5
        jg rem6
    f49:
        iaddq $-4, %rdx
        jl f46
        je rem7
    f89:
        iaddq $-2, %rdx
        jl rem8
    rem9:
        mrmovq 0x40(%rdi), %r8
        rmmovq %r8, 0x40(%rsi)
        andq %r8, %r8
        jle rem8
        iaddq $1, %rax
    rem8:
        mrmovq 0x38(%rdi), %r8
        rmmovq %r8, 0x38(%rsi)
        andq %r8, %r8
        jle rem7
        iaddq $1, %rax
    rem7:
        mrmovq 0x30(%rdi), %r8
        rmmovq %r8, 0x30(%rsi)
        andq %r8, %r8
        jle rem6
        iaddq $1, %rax
    rem6:
        mrmovq 0x28(%rdi), %r8
        rmmovq %r8, 0x28(%rsi)
        andq %r8, %r8
        jle rem5
        iaddq $1, %rax
    rem5:
        mrmovq 0x20(%rdi), %r8
        rmmovq %r8, 0x20(%rsi)
        andq %r8, %r8
        jle rem4
        iaddq $1, %rax
    rem4:
        mrmovq 0x18(%rdi), %r8
        rmmovq %r8, 0x18(%rsi)
        andq %r8, %r8
        jle rem3
        iaddq $1, %rax
    rem3:
        mrmovq 0x10(%rdi), %r8
        rmmovq %r8, 0x10(%rsi)
        andq %r8, %r8
        jle rem2
        iaddq $1, %rax
    rem2:
        mrmovq 0x8(%rdi), %r8
        rmmovq %r8, 0x8(%rsi)
        andq %r8, %r8
        jle rem1
        iaddq $1, %rax
    rem1:
        mrmovq (%rdi), %r8
        rmmovq %r8, (%rsi)
        andq %r8, %r8
        jle Done
        iaddq $1, %rax
    ```

    The average CPE has improved to `7.66`.

   The remainder cases are handled using a carefully structured decision tree. Special attention must be paid to the interleaving of `mrmovq` and `andq` instructions, as they directly affect global condition codes. Specifically, the `jle` instruction condition `(SF ^ OF) | ZF = 1` requires precise control over flag states. This ensures that `jl` or `jg` branches correctly direct execution to the appropriate remainder-handling routines; otherwise, incorrect flag propagation could lead to erroneous counting. ([Reference Article](https://blog.csdn.net/Leafing_/article/details/130207471))

    ```hcl
        iaddq $-10, %rdx
        jl f09
    pre:
        mrmovq (%rdi), %r8
        mrmovq 8(%rdi), %r9
        mrmovq 16(%rdi), %r10
        mrmovq 24(%rdi), %r11
        mrmovq 32(%rdi), %r12
        mrmovq 40(%rdi), %r13
        mrmovq 48(%rdi), %r14
        mrmovq 56(%rdi), %rbx
        mrmovq 64(%rdi), %rcx
        mrmovq 72(%rdi), %rbp
    work1:	
        rmmovq %r8, (%rsi)
        andq %r8, %r8
        jle work2
        iaddq $1, %rax
    work2:
        rmmovq %r9, 8(%rsi)
        andq %r9, %r9
        jle work3
        iaddq $1, %rax
    work3:
        rmmovq %r10, 16(%rsi)
        andq %r10, %r10
        jle work4
        iaddq $1, %rax
    work4:
        rmmovq %r11, 24(%rsi)
        andq %r11, %r11
        jle work5
        iaddq $1, %rax
    work5:
        rmmovq %r12, 32(%rsi)
        andq %r12, %r12
        jle work6
        iaddq $1, %rax
    work6:
        rmmovq %r13, 40(%rsi)
        andq %r13, %r13
        jle work7
        iaddq $1, %rax
    work7:
        rmmovq %r14, 48(%rsi)
        andq %r14, %r14
        jle work8
        iaddq $1, %rax
    work8:
        rmmovq %rbx, 56(%rsi)
        andq %rbx, %rbx
        jle work9
        iaddq $1, %rax
    work9:
        rmmovq %rcx, 64(%rsi)
        andq %rcx, %rcx
        jle work10
        iaddq $1, %rax
    work10:
        rmmovq %rbp, 72(%rsi)
        andq %rbp, %rbp
        jle modify
        iaddq $1, %rax
    modify:
        iaddq $0x50,%rdi
        iaddq $0x50,%rsi
        iaddq $-10,%rdx
        jge pre
    f09:
        iaddq $7, %rdx
        jg f49
        jl f02
        je rem3
    f02:
        iaddq $2, %rdx
        je rem1
        iaddq $-1, %rdx
        je rem2
        ret
    f49:
        iaddq $-3, %rdx
        jg f79
        je rem6
        iaddq $1, %rdx
        je rem5
        jmp rem4
    f79:
        iaddq $-2, %rdx
        jl rem7
        je rem8
    rem9:
        mrmovq 64(%rdi), %r8
        andq %r8, %r8
        rmmovq %r8, 64(%rsi)
    rem8:
        mrmovq 56(%rdi), %r8
        jle ex9
        iaddq $1, %rax
    ex9:
        rmmovq %r8, 56(%rsi)
        andq %r8, %r8
    rem7:
        mrmovq 48(%rdi), %r8
        jle ex8
        iaddq $1, %rax
    ex8:
        rmmovq %r8, 48(%rsi)
        andq %r8, %r8
    rem6:
        mrmovq 40(%rdi), %r8
        jle ex7
        iaddq $1, %rax
    ex7:
        rmmovq %r8, 40(%rsi)
        andq %r8, %r8
    rem5:
        mrmovq 32(%rdi), %r8
        jle ex6
        iaddq $1, %rax
    ex6:
        rmmovq %r8, 32(%rsi)
        andq %r8, %r8
    rem4:
        mrmovq 24(%rdi), %r8
        jle ex5
        iaddq $1, %rax
    ex5:
        rmmovq %r8, 24(%rsi)
        andq %r8, %r8
    rem3:
        mrmovq 16(%rdi), %r8
        jle ex4
        iaddq $1, %rax
    ex4:
        rmmovq %r8, 16(%rsi)
        andq %r8, %r8
    rem2:
        mrmovq 8(%rdi), %r8
        jle ex3
        iaddq $1, %rax
    ex3:
        rmmovq %r8, 8(%rsi)
        andq %r8, %r8
    rem1:
        mrmovq (%rdi), %r8
        jle ex2
        iaddq $1, %rax
    ex2:
        rmmovq %r8, (%rsi)
        andq %r8, %r8
        jle Done
        iaddq $1, %rax
    ```

    After implementing comprehensive pipeline optimizations including register preloading and careful remainder handling, the implementation achieved:

    ```
    Average CPE     7.50
    Score   60.0/60.0
    ```

5. Modify HCL (extra)

    The expression `E_icode in { IMRMOVQ, IPOPQ } && E_dstM in { d_srcA, d_srcB }` identifies load/use hazards in the pipeline. 

    |Instruction\Cycle|1|2|3|4|5|6|
    |:--:|:--:|:--:|:--:|:--:|:--:|:--:|
    |`mrmov I1(%rax) %rbx`|F|D|E|**M**|W||
    |`rmmov %rbx I2(%rax)`||F|D|E|M|**W**|

    In the pipeline timeline, the first instruction reads data from memory during its Memory stage (cycle 4). The second instruction needs that same data during its own Memory stage (cycle 5). Because the data is available one cycle before it is needed, forwarding is possible: the value from `m_valM` can be routed directly to the `valA` field in the next instruction's pipeline register, bypassing the register file.

    Load forwarding works when two conditions are met:  

    1. The current instruction loads a value from memory into a register (e.g., `mrmovq` or `popq`) and uses that register as the source operand.  
    2. The next instruction uses that same register as a source operand **not in its Execute stage**, but only later, during its Memory stage — typically in store-type instructions such as `rmmovq` or `pushq`.  

    To support this, a forwarding path is added that connects the memory output signal `m_valM` to the `valA` input of the pipeline register M, allowing it to be passed directly to `e_valA` in the following instruction. Formally, the forwarding condition can be expressed as `E_icode in { IMRMOVQ, IPOPQ } && E_srcA == M_dstM`. Therefore, only the following control logic will trigger load/use hazard handling:

    ```hcl
    E_icode in { IMRMOVQ, IPOPQ } && (E_dstM == d_srcB || (E_dstM == d_srcA && !(D_icode in { IMRMOVQ, IPUSHQ })))
    ```

    The logic for `e_valA` in the Execute stage must be updated to select the forwarded value from memory when applicable:

    ```hcl
    word e_valA = [
    E_icode in { IRMMOVQ, IPUSHQ } && E_srcA == M_dstM: m_valM;
        1 : E_valA;  # Use valA from stage pipe register
    ];
    ```

    The pipeline control logic must also be adjusted to handle load/use hazards appropriately:

    ```hcl
    bool F_bubble = 0;
    bool F_stall =
        # Conditions for a load/use hazard
        (E_icode in { IMRMOVQ, IPOPQ } && (E_dstM == d_srcB || (E_dstM == d_srcA && !(D_icode in { IMRMOVQ, IPUSHQ })))) ||
        # Stalling at fetch while ret passes through pipeline
        IRET in { D_icode, E_icode, M_icode };

    # Should I stall or inject a bubble into Pipeline Register D?
    # At most one of these can be true.
    bool D_stall = 
        # Conditions for a load/use hazard
        E_icode in { IMRMOVQ, IPOPQ } &&
        (E_dstM == d_srcB || (E_dstM == d_srcA && !(D_icode in { IMRMOVQ, IPUSHQ })));

    bool D_bubble =
        # Mispredicted branch
        (E_icode == IJXX && !e_Cnd) ||
        # Stalling at fetch while ret passes through pipeline
        # but not condition for a load/use hazard
        !(E_icode in { IMRMOVQ, IPOPQ } && (E_dstM == d_srcB || (E_dstM == d_srcA && !(D_icode in { IMRMOVQ, IPUSHQ })))) &&
        IRET in { D_icode, E_icode, M_icode };

    # Should I stall or inject a bubble into Pipeline Register E?
    # At most one of these can be true.
    bool E_stall = 0;
    bool E_bubble =
        # Mispredicted branch
        (E_icode == IJXX && !e_Cnd) ||
        # Conditions for a load/use hazard
        (E_icode in { IMRMOVQ, IPOPQ } && (E_dstM == d_srcB || (E_dstM == d_srcA && !(D_icode in { IMRMOVQ, IPUSHQ }))));
    ```
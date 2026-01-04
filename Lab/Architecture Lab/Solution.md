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

First subtract the unroll factor from the length counter. If the result is negative, handle remaining elements separately. Otherwise, restore the counter and execute the unrolled loop.

```assembly
	xorq %rax,%rax		# count = 0;
	andq %rdx,%rdx		# len <= 0?
	jle Done		# if so, goto Done:

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

To reduce data hazards in the pipeline, we employ **4-way loop unrolling** with **early loading** of data values by using additional registers (`%r8`, `%r9`, `%r10`, `%r11`) to pre‑fetch memory operands. The optimized implementation achieves an `average CPE of 8.07`.

```assembly
	xorq %rax,%rax
	iaddq $-4, %rdx
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
	andq %r11, %r11
	jle modify
	iaddq $1, %rax
modify:
	iaddq $32,%rdi
	iaddq $32,%rsi
	iaddq $-4,%rdx
	jge work1
res1:
	iaddq $3, %rdx
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
	rmmovq %r10, 16(%rsi)
	andq %r10, %r10
	jle Done
	iaddq $1, %rax
```
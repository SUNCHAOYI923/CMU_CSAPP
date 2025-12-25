## Chapter 1 A Tour of Computer System

#### Compilation System
    
Take C language as an example `linux > gcc hello.c -o hello`

- **Pre-processor** (cpp)   
    $\texttt{.c} \to \texttt{.i}$ Removal of comments & Header file inclusion & Macro expansion

- **Compiler** (cc1)   
    $\texttt{.i} \to \texttt{.s}$ Translates into assembly language

- **Assembler** (as)   
    $\texttt{.s} \to \texttt{.o}$ Generate relocatable object file

- **Linker** (ld) $\texttt{.o} \to \texttt{Executable}$ Combines codes with standard libraries

#### Hardware Organization of a System

<img src="pic/1.png" width="70%" height="70%">

- **System Bus** 

    Transfer data in fixed-size blocks called words (4/8 bytes on a 32/64 bit system)

#### Memory Hierarchy

<img src="pic/2.png" width="70%" height="70%">

#### Abstractions in Computer Systems

<img src="pic/3.png" width="70%" height="70%">

- **Process & Thread**   
    Context switching with OS Kernel
- **Virtual Memory**  
    Program Code & Data, Shared Libraries, Heap, Stack, Kernel Virtual Memory from bottom to top
- **File**

#### Amdahl's Law (Quantifying the performance improvement ceiling)

$T_{new} = (1 - \alpha) T_{old} + \frac{\alpha T_{old}}{k} = T_{old} (1 - \alpha + \frac{\alpha}{k})$

$S = \frac{T_{old}}{T_{new}} = \frac{1}{1 - \alpha + \frac{\alpha}{k}}$


## Chapter 2 Representing and Manipulating Information

### Information Storage

- **Words**

    $w$ word size $\Longleftrightarrow$ $[0,2^w)$ virtual address space

- **Addressing and Byte Ordering** Big endian & Little endian

### Integer Representaions

#### Unsigned Encodings

Suppose a vector $\mathrm{x} = [x_{w - 1},x_{w - 2},\cdots,x_0]$, then $\operatorname{B2U_w}(x) = \sum \limits_{i = 0}^{w - 1}x_i \cdot 2^i$

#### Two's Complement Encodings

Suppose a vector $\mathrm{x} = [x_{w - 1},x_{w - 2},\cdots,x_0]$, then $\operatorname{B2T_w}(x) = -x_{w - 1} \cdot 2^{w - 1} + \sum \limits_{i = 0}^{w - 2}x_i \cdot 2^i$

#### Conversions between Signed and Unsigned

$$
\operatorname{T2U_w}(x) = 
\begin{cases}
x + 2^w,\quad x < 0 \\
x,\quad x \ge 0
\end{cases}
\quad
\operatorname{U2T_w}(u) = 
\begin{cases}
u,\quad x \le Tmax_w\\
u - w^w,\quad x > Tmax_w
\end{cases}
$$

#### Sign Extension

- **Small to Big** 

    - Zero extension of unsigned numbers
    
    - Sign extension of two's complement numbers

        $\operatorname{B2T_w}([x_{w - 1},x_{w - 2},\cdots,w_0]) = \operatorname{B2T_{w + k}}([x_{w - 1},x_{w - 1},\cdots,x_{w - 1},x_{w - 1},x_{w - 2},\cdots,x_0])$

        Since $\operatorname{B2T_{w + 1}} -\operatorname{B2T_w} = (-x_{w - 1} \cdot 2^w + x_{w - 1} \cdot 2^{w - 1}) - x_{w - 1} \cdot 2^{w - 1} = 0$, by induction, we can proof it.

- **Big to Small**

    - $\operatorname{B2U_{k}}(x) = \operatorname{B2U_{w}}(x) \bmod 2^k$

    - $\operatorname{B2T_{k}}(x) = \operatorname{U2T_{w}}(\operatorname{B2U_{w}}(x) \bmod 2^k)$

### Integer Arithmetic

#### Addition

$$
x + y^u_w = 
\begin{cases}
x + y,\quad x + y < 2^w \\
x + y - 2^w,\quad 2^w \le x + y < 2^{w + 1}
\end{cases}
$$

$$
x + y^t_w = 
\begin{cases}
x + y - 2^w,\quad 2^{w - 1} \le x + y\\
x + y,\quad -2^{w - 1} \le x + y < 2^{w - 1} \\
x + y + 2^w,\quad x + y < -2^{w - 1}
\end{cases}
$$

#### Additive Inverse

$x + x' = 0\ \text{or}\ 2^w$

$$
-x^u_w = 
\begin{cases}
x,\quad x = 0 \\
2^w - x,\quad x > 0
\end{cases}
\quad
-x^t_w = 
\begin{cases}
x,\quad x > Tmin_w \\
Tmin_w,\quad x = Tmin_w
\end{cases}
$$

#### Multipilication

$$
x \times y^u_w = (x \cdot y) \bmod 2^k \\
x \times y^t_w = \operatorname{U2T_w}((x \cdot y) \bmod 2^k)
$$

#### Division (by a power of 2)

Unsigned numbers use **logical shift**, while two's complement numbers use **arithmetic shift** to achieve sign-preserving extension.

Right shift performs integer division by powers of two : 

- $x \le 0 \quad x >> k = \lfloor \frac{x}{2^k} \rfloor$

- $x < 0 \quad (x + (1 << k) - 1) >> k = \lceil \frac{x}{2^k} \rceil$

### Floating Point

#### Floating-Point Representation

$V = (-1)^s \times M \times 2^E$

s (sign) : The number is positive ($s=0$) or negative ($s=1$).

M (fraction) : A binary fraction.

E (exponent) : $2^E$ weight.

<img src="pic/4.png" width="70%" height="70%">

$\texttt{bias (float)} = 127 \quad \texttt{bias(double)} = 1023$ ($\texttt{bias} = 2^k - 1$)

- **Normalized** exp is neither all 0s nor all 1s, i.e. $e \in [1,255]$

    $E = e - \texttt{bias}$, 

    $M = 1 + f$

- **Denormalized**

    $s = 0, f = 0 \rightarrow V = +0.0 \quad s = 1, f = 0 \rightarrow V = -0.0$

    $E = 1 - \texttt{bias}$

    $M = f$

- **Infinity**

    $s = 0, f = 0 \rightarrow V = +\infin \quad s = 1, f = 0 \rightarrow V = -\infin$

- **NaN** (Not a Number)

    $f \neq 0$

|Format|Minimum|Maximum|
|:--:|:--:|:--:|
|Single Precision Normalized <br> $V = (-1)^s \times \overline{1.f} \times 2^{e - 127}$|$e = \texttt{00000001}$ <br> $E_{\min} = -126$ <br> $f = 0$ <br> $V = 1.0 \times 2^{-126}$|$e = \texttt{11111110}$ <br> $E_{\max} = 127$ <br> $f = 0.\underbrace{11\ldots 1}_{23\ \text{ones}}$ <br> $M = 1 + f = 1 + (1 - 2^{-23})$ <br>$V = 1.0 \times 2^{127} \times (2 - 2^{-23}) \approx 3.4 \times 10^{38}$|
|Double Precision Normalized <br> $V = (-1)^s \times \overline{0.f} \times 2^{-126}$|$e = \texttt{00000000}$ <br> $f = 2^{-23}$ <br> $V = 1.0 \times 2^{-149}$|$e = \texttt{00000000}$ <br> $f = 0.\underbrace{11\ldots 1}_{23\ \text{ones}}$ <br> $V = 1.0 \times 2^{-126} \times (1 - 2^{-23})$|

#### Rounding

- **Round-down** e.g. $1.40 \to 1 \quad -1.5 \to -2$

- **Round-up** e.g. $1.40 \to 2 \quad -1.5 \to -1$

- **Round-toward-zero** e.g. $1.40 \to 1 \quad -1.5 \to -1$

- **Round-to-even** e.g. $1.40 \to 1 \quad 1.6 \to 2 \quad 1.5 \to 2 \quad 2.5 \to 2$
    - **Non-midpoint** round to the nearest representable value
    - **Midpoint** choose the even one

#### Floating Point Operations

Lack of Associativity & Lack of Distributivity

## Chapter 3 Machine-Level Representation of Programs

### Machine-Level Representation of Programs

#### Size of Data Type in IA32

|C declaration|Intel data type|Assembly-code suffix|Size (bytes)|
|:--:|:--:|:--:|:--:|
|$\texttt{char}$|Byte|$\texttt{b}$|$1$|
|$\texttt{short}$|Word|$\texttt{w}$|$2$|
|$\texttt{int}$|Double word|$\texttt{l}$|$4$|
|$\texttt{long}$|Quad word|$\texttt{q}$|$8$|
|$\texttt{char *}$|Quad word|$\texttt{q}$|$8$|
|$\texttt{float}$|Single precision|$\texttt{s}$|$4$|
|$\texttt{double}$|Double precision|$\texttt{l}$|$8$|

#### Register

<img src="pic/5.png" width="50%" height="60%"><img src="pic/6.png" width="40%" height="50%">

#### Information Access

- **Operands**

    - **Immediate**

    - **Register**

    - **Memory Reference** Immediate & Base Register & Index Register & Scale Factor (1,2,4,8)

    |Type|Form|Operator Value|Name|
    |:--:|:--:|:--:|:--:|
    |Immediate|$\$Imm$|$Imm$|Immediate|
    |Register|$r_a$|$R[r_a]$|Register|
    |Memory|$Imm (r_b,r_i,s)$|$M[Imm + R[r_b] + R[r_i] \cdot s]$|Scaled Indexed|
    |Memory|$Imm$|$M[Imm]$|Absolute|
    |Memory|$(r_a)$|$M[R[r_a]]$|Indirect|
    |Memory|$Imm (r_b)$|$M[Imm + R[r_b]]$|Base + Displacement|
    |Memory|$(r_b,r_i)$|$M[R[r_b] + R[r_i]]$|Indexed|
    |Memory|$Imm (r_b,r_i)$|$M[Imm + R[r_b] + R[r_i]]$|Indexed|
    |Memory|$(,r_i,s)$|$M[R[r_i] \cdot s]$|Scale Indexed|
    |Memory|$Imm(,r_i,s)$|$M[Imm + R[r_i] \cdot s]$|Scale Indexed|
    |Memory|$(r_b,r_i,s)$|$M[R[r_b] + R[r_i] \cdot s]$|Scale Indexed|

- **Operation code**

    - **Movement** 

        - $\text{mov} \quad \text{S,D}$ [`movb`, `movw`, `movl`, `movq`, `movabsq`]

            Source operand (Immedita, Register, Memory); Destination operand (Register, Memory)

            Two operands of a move instruction **cannot** both be memory references. (It is necessary to first move the source memory reference into a register, and then move it to the destination memory reference.)

            `movl` writes 32 bits of data to the lower half and then automatically zero-extends the value into the upper 32 bits. (In x86-64, any instruction that generates a 32-bit result for a register will set the upper 32 bits of that register to zero.)

            `movq` can only be 32 bits (sign-extended to 64 bits), whereas `movabsq` can be 64-bit but the destination must be a **register**.

        - $\text{movz} \quad \text{S, R}$ [`movzbw`, `movzbl`, `movzwl`, `movzbq`, `movzwq`]

            `movzlq` does not exist because of `movl` instruction.

        - $\text{movs} \quad \text{S, R}$ [`movsbw`, `movsbl`, `movswl`, `movsbq`, `movswq`, `movslq`, `cltq`]
            
            `cltq` same as `movslq %eax, %rax`

    - **Stack**

        let a quad number be the example

        - $\texttt{pushq} \quad S$ 

            ```assemblely
            subq $8 %rsp
            movq %rbq,(%rsp)
            ```
        
         - $\texttt{popq} \quad S$ 

            ```assemblely
            movq (%rsq),%rax
            addq $8, %rsp
            ```
#### Arithmetic and Logical Operations

- **Load Effective Address**

    $\texttt{leaq} \quad S, D$ 
    
    Computes effective address and stores it in destination register

- **Unary Operations**

|Instructions|Effect|
|:--:|:--:|
|`inc D`|$D \leftarrow D + 1$|
|`dec D`|$D \leftarrow D - 1$|
|`neg D`|$D \leftarrow -D$|
|`not D`|$D \leftarrow \sim D$|

- **Binary Operations**

|Instructions|Effect|
|:--:|:--:|
|`add S D`|$D \leftarrow D + S$|
|`sub S D`|$D \leftarrow D - S$|
|`imul S D`|$D \leftarrow D \times S$|
|`xor S D`|$D \leftarrow D \oplus S$|
|`or S D`|$D \leftarrow D \mid S$|
|`and S D`|$D \leftarrow D \mathbin{\&} S$|

- **Shift Operations**

|Instructions|Effect|
|:--:|:--:|
|`sal D`|$D \leftarrow D << k$|
|`shl D`|$D \leftarrow D << k$|
|`sar D`|$D \leftarrow D >>_A k$ Arithmetic Right Shift|
|`shr D`|$D \leftarrow D >>_L k$ Logical Right Shift|

Shift instructions can shift by an immediate value, or by a value placed in the single-byte register `%cl`.
# CS:APP Data Lab Report
**Name** SUNCHAOYI  

## Introduction
This report documents the solutions to the CS:APP Data Lab problems. The goal of the lab is to solve a series of programming puzzles under strict constraints, using only a limited set of C operators to manipulate integer and floating-point bit patterns.

## Report

1. **bitXor** 

    **Description:** $x \oplus y$.

    **Legal ops:** `~ &`

    **Solution:**

    - Ops = 8

        $$
        \begin{align*}
        A \oplus B &= \overline{A} B \cup A \overline{B} \\
        &= \overline{\overline{\overline{A} B} \cap \overline{A \overline{B}}} \\
        \end{align*}
        $$

        ```cpp
        int bitXor (int x,int y)
        {
            int a = ~((~x) & y);
            int b = ~(x & (~y));
            return ~(a & b);
        }
        ```

    - Ops = 7 (Opt.)

        $$
        \begin{align*}
        A \oplus B &= (A \cup B) \cap \overline{A \cap B} \\
        &= \overline{\overline{A} \cap \overline{B}} \cap \overline{A \cap B}\\
        \end{align*}
        $$

        ```cpp
        int bitXor (int x,int y) {return ~(~x & ~y) & ~(x & y);}
        ```

2. **tmin** 

    **Description:** Return minimum two's complement integer.

    **Legal ops:** `! ~ & ^ | + << >>`

    **Solution:**

    ```cpp
    int tmin (void) {return 1 << 31;}
    ```

3. **isTmax** 

    **Description:** Returns 1 if $x$ is the maximum, two's complement number, and 0 otherwise.

    **Legal ops:** `! ~ & ^ | +`

    **Solution:**

    - The maximum two's complement integer $T_{\text{max}}$ has the property $x + 1 = \sim x$, i.e. $(x + 1) \oplus (\sim x) = 0$.

    - $x = -1$ is a special case ($\texttt{0xffffffff}$) that need to be excluded.

    ```cpp
    int isTmax (int x) {return !((~(x + 1)) ^ x) & !!(x + 1);}
    ```
    

4. **allOddBits** 

    **Description:** Return 1 if all odd-numbered bits in word set to 1.

    **Legal ops:** `! ~ & ^ | + << >>`

    **Solution:**

    Construct a number whose odd-numbered bits are all 1, i.e. the 32-bit pattern $\texttt{0xAAAAAAAA}$.

    - Ops = 9
        
        ```cpp
        int allOddBits (int x)
        {
            int mask = (0xAA << 24) | (0xAA << 16) | (0xAA << 8) | 0xAA;
            return !((x & mask) ^ mask);
        }
        ```

    - Ops = 7 (Opt.)

        ```cpp
        int allOddBits (int x)
        {
            int a = 0xAA << 8;
            int b = a | 0xAA;
            int c = b << 16 | b;
            return !((x & c) ^ c);
        }
        ```

5. **negate** 

    **Description:** Return $-x$.

    **Legal ops:** `! ~ & ^ | + << >>`

    **Solution:**

    $$
    \begin{cases}
    x + (-x) = 0\\
    x + (\sim x) = -1
    \end{cases}
    \Longrightarrow
    -x = (\sim x) + 1
    $$

    ```cpp 
    int negate (int x) {return (~x) + 1;}
    ```

6. **isAsciiDigit** 

    **Description:** Return 1 if $0x30 \le x \le 0x39$ (ASCII codes for characters `0` to `9`).

    **Legal ops:** `! ~ & ^ | + << >>`

    **Solution:**

    - Upper 24 bits must be zero.

    - Bits 4-7 must equal `0011`.

    - Lower 4 bits must be in range `0000` to `1001`. (Check whether bit 3 is 0 to have more detailed classification.)

    - Ops = 14

        ```cpp
        int isAsciiDigit (int x)
        {
            int a = !(x >> 8); 
            int b = !(x >> 4 ^ 0x3);
            int low = x & 0xF;
            int c = !(low >> 3) | !(low >> 1 ^ 0x4);
            return a & b & c;
        }
        ```
    
    - Ops = 7 (Opt.)
        
        - No need to use variable $a$ to checker upper 24 bits.

        - use +6 to bound lower 4 bits within `1111`.

        ```cpp
        int isAsciiDigit (int x)
        {
            int a = x >> 4 ^ 3;
            int b = ((x & 0xF) + 6) >> 4;
            return !(a | b);
        }
        ```

7. **conditional** 

    **Description:** Same as `x ? y : z`.

    **Legal ops:** `! ~ & ^ | + << >>`

    **Solution:**

    - Convert any non-zero $x$ to boolean.

    - Create a mask that is either all zeros ($\texttt{0x00000000}$) or all ones ($\texttt{0xFFFFFFFF}$).

    - Use the mask to select between $-y$ and $-z$.

    - Ops = 14

        ```cpp
        int conditional (int x,int y,int z)
        {
            int op = !!x;
            int a = (~op) + 1;
            int b = (~a) & ((~y) + 1); // when mask = 0x00000000, b = -y; otherwise b = 0
            int c = a & ((~z) + 1); // when mask = 0xFFFFFFFF, c = -z; otherwise c = 0
            return y + z + b + c;
        }
        ```
    
    - Ops = 8 (Opt.)
        
        ```cpp
        int conditional (int x,int y,int z)
        {
            int mask = (~(!!x)) + 1;
            return (y & mask) | (z & (~mask)); // one side must equal to 0
        }
        ```

8. **isLessOrEqual** 

    **Description:** If $x \le y$ then return 1, else return 0.

    **Legal ops:** `! ~ & ^ | + << >>`

    **Solution:**

    - Different signs: If $x$ is negative and $y$ is non-negative, then $x \leq y$ is always true.
    - Same signs: We can safely compute $y - x$ without overflow and check if it's non-negative.

    ```cpp
    int isLessOrEqual (int x,int y)
    {
        int signx = x >> 31 & 1;
        int signy = y >> 31 & 1;
        int diff = signx ^ signy;
        int a = diff & signx;
        int b = !diff & !((y + ((~x) + 1)) >> 31); // y - x >= 0
        return a | b;
    }
    ```

9. **logicalNeg** 

    **Description:** Implement the `!` operator using all of the legal operators except `!`.

    **Legal ops:** `~ & ^ | + << >>`

    **Solution:**

    - If $x = 0$, both $x$ and $-x$ have sign bit 0. Otherwise either $x$ or $-x$ have sign bit 1.

    - $x | (-x)$ is $\texttt{0x00000000}$ or $\texttt{0xFFFFFFFF}$, just add 1 to solve it.

    ```cpp
    int logicalNeg (int x) {return ((x | ((~x) + 1)) >> 31) + 1;}
    ```

10. **howManyBits** 

    **Description:** Return the minimum number of bits required to represent $x$ in two's complement.

    **Legal ops:** `! ~ & ^ | + << >>`

    **Solution:**

    - For $x < 0$ find the highest 0-bit, $x \ge 0$ find the highest 1-bit. Use `x ^ (x >> 31)` to transform $x$ into the latter case.

    - Use divide-and-conquer approach for the most significant 1-bit, and finally add the sign bit.

    ```cpp
    int howManyBits (int x)
    {
        int b16,b8,b4,b2,b1,b0;
        x = x ^ (x >> 31);
        b16 = (!!(x >> 16)) << 4;
        x >>= b16;
        b8 = (!!(x >> 8)) << 3;
        x >>= b8;
        b4 = (!!(x >> 4)) << 2;
        x >>= b4;
        b2 = (!!(x >> 2)) << 1;
        x >>= b2;
        b1 = !!(x >> 1);
        x >>= b1;
        b0 = x;
        return b16 + b8 + b4 + b2 + b1 + b0 + 1; // sign
    }
    ```

11. **floatScale2** 

    **Description:** Return bit-level equivalent of expression $2 \times f$ for floating point argument $f$.

    **Legal ops:** Any integer/unsigned operations incl. `||`, `&&`. Also `if`, `while`.

    **Solution:**

    - Extraction the sign bit, exponent field and fraction field.

    - When $exp = \texttt{0xFF}$, just return it; When $exp = \texttt{0x00}$,  $frac \times 2$; otherwise $exp + 1$.

    ```cpp
    unsigned floatScale2 (unsigned uf)
    {
        unsigned sign = uf >> 31 & 0x1;
        unsigned exp = (uf >> 23) & 0xFF;
        unsigned frac = uf & 0x7FFFFF;
        if (exp == 0xFF) return uf;
        else if (exp == 0x0) frac <<= 1;
        else ++exp;
        return (sign << 31) | (exp << 23) | frac;
    }
    ```

12. **floatFloat2Int** 

    **Description:** Return bit-level equivalent of expression (int) $f$ for floating point argument $f$.

    **Legal ops:** Any integer/unsigned operations incl. `||`, `&&`. Also `if`, `while`.

    **Solution:**

    Converting to integer with truncation:
    $$
    \text{int}(f) = 
    \begin{cases}
    0 & \text{if } |f| < 1 \\
    \text{trunc}\left(1.f \times 2^{e-127}\right) & \text{if } 1 \leq |f| < 2^{31} \\
    \text{INT\_MIN} & \text{otherwise (overflow)}
    \end{cases}
    $$

    Where truncation is implemented by:
    - When $E \geq 23$: $1.f \times 2^{E} = (1.f \times 2^{23}) \times 2^{E - 23}$
    - When $E < 23$: Right shift discards fractional bits, i.e. $>> (23 - E)$

    ```cpp
    int floatFloat2Int (unsigned uf)
    {
        unsigned sign = uf >> 31 & 0x1;
        unsigned exp = (uf >> 23) & 0xFF;
        unsigned frac = uf & 0x7FFFFF;
        int E,M;
        E = exp - 127;
        if (E < 0) return 0;
        if (E >= 31) return 0x80000000u;
        M = (1 << 23) | frac;
        if (E >= 23) M <<= E - 23;
        else M >>= 23 - E;
        return sign ? -M : M;
    }
    ```

13. **floatPower2** 

    **Description:** Return bit-level equivalent of the expression $2.0^x$ (2.0 raised to the power $x$).

    **Legal ops:** AAny integer/unsigned operations incl. `||`, `&&`. Also `if`, `while`.

    **Solution:**

    - Case 1 : Overflow

        $x > E_{\max} = e - 127 = 254 - 127 = 127$

        return $\texttt{0b01111111100...0}$, i.e. $\texttt{0x7F800000}$

    - Case 2 : Normalized

        $e = E + 127 \in [1,254] \Longrightarrow E \in [-126,127]$ 

        $V = 1.0 \times 2^{e - 127}$

        $f = 0,\quad V \leftarrow (e << 23) | f$

    - Case 3 : Denormalized

        $e = 0$ 
        
        $V = \overline{0.f} \times 2^{-126} = 2^x \Longrightarrow \overline{0.f} = 2^{x + 126}$

        minimum number is $2^{-126} \times 2^{-23} = 2^{-149}$, i.e. put 1 on index 0.

        $\Longrightarrow V \leftarrow 1 << (x + 149)$

    - Case 4 : Underflow

        $x < -149$, return 0
    
    ```cpp
    unsigned floatPower2 (int x)
    {
        if (x < -149) return 0;
        else if (x > 127) return 0x7F800000;
        else if (x >= -126) return (x + 127) << 23;
        else return 1 << (x + 149);
    }
    ```

----

```
Correctness Results     Perf Results
Points  Rating  Errors  Points  Ops     Puzzle
1       1       0       2       7       bitXor
1       1       0       2       1       tmin
1       1       0       2       8       isTmax
2       2       0       2       7       allOddBits
2       2       0       2       2       negate
3       3       0       2       7       isAsciiDigit
3       3       0       2       8       conditional
3       3       0       2       14      isLessOrEqual
4       4       0       2       5       logicalNeg
4       4       0       2       32      howManyBits
4       4       0       2       12      floatScale2
4       4       0       2       16      floatFloat2Int
4       4       0       2       9       floatPower2

Score = 62/62 [36/36 Corr + 26/26 Perf] (128 total operators)
```
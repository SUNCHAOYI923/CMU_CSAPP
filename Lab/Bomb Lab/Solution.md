## Phase_1

```assembly
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi
  400ee9:	e8 4a 04 00 00       	call   401338 <strings_not_equal>
  400eee:	85 c0                	test   %eax,%eax
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>
  400ef2:	e8 43 05 00 00       	call   40143a <explode_bomb>
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	ret
```

- `mov    $0x402400,%esi`

    `0x402400` contains the password string. Use GDB's `x/s` command to display the string at that memory address:

    ```gdb
    gdb bomb
    (gdb) x/s 0x402400
    ```

The Phase_1 answer is:

```
Border relations with Canada have never been better.
```


## Phase_2

```assembly
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp
  400f02:	48 89 e6             	mov    %rsp,%rsi
  400f05:	e8 52 05 00 00       	call   40145c <read_six_numbers>
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	call   40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax
  400f1a:	01 c0                	add    %eax,%eax
  400f1c:	39 03                	cmp    %eax,(%rbx)
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	call   40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	ret
```

- `cmpl $0x1,(%rsp)` & `je     400f30 <phase_2+0x34>`

    If the value at the top of the stack is 1, jump to address `0x400f30`.

- `lea    0x4(%rsp),%rbx` & `lea    0x18(%rsp),%rbp` & `jmp    400f17 <phase_2+0x1b>`

    `%rbx` is set to point to the second element of the array (`&numbers[1]`).

    `%rbp` is set to point just past the last element (`&numbers[6]`), since `0x18` $= 6 \times 4 = 24$ bytes.  

    Execution then jumps to the loop body at `0x400f17`.

- `0x400f17` $\sim$ `0x400f2c` (loop body)

    Load the previous element (`numbers[i-1]`) into `%eax`, double it, and compare the result with the current element (`numbers[i]`).  

    Only if `numbers[i] == 2 * numbers[i-1]` does the loop continue; otherwise the bomb explodes.  

    After processing all six numbers, control jumps to `0x400f3c` (loop exit).

The required sequence for Phase 2 is therefore:

```
1 2 4 8 16 32
```

## Phase_3

```assembly
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	call   400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>
  400f65:	e8 d0 04 00 00       	call   40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
  400f75:	ff 24 c5 70 24 40 00 	jmp    *0x402470(,%rax,8)
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	call   40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	call   40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	ret
```

- `mov    $0x4025cf,%esi` & `cmp    $0x1,%eax` & `jg     400f6a <phase_3+0x27>`

    Using `(gdb) x/s 0x4025cf` shows `"%d %d"`, meaning the input must be two integers separated by a space.  
    
    If `sscanf` returns a value $\le 1$, the bomb explodes.

- `lea    0xc(%rsp),%rcx` & `lea    0x8(%rsp),%rdx` & `cmpl   $0x7,0x8(%rsp)` & `ja     400fad <phase_3+0x6a>`

   `%rdx` points to where the first integer is stored (`0x8(%rsp)`), and `%rcx` points to the second integer (`0xc(%rsp)`).  

    The first integer must be $\le 7$, otherwise the bomb explodes.

- `0x400f7c` $\sim$ `0x400fbe` (switch body)

    Each case loads a specific immediate value into `%eax`, then jumps to a common check at `0x400fbe`.  
    
    There, the loaded value is compared with the second integer. Only if they are equal does the phase pass.

From the jump‑table cases we obtain the valid pairs:

```
0 207 / 1 311 / 2 707 / 3 256 / 4 389 / 5 206 / 6 682 / 7 327
```

Any one of these pairs is a correct solution.
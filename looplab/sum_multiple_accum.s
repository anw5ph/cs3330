// This is assembly is roughly equivalent to the following C code:
// unsigned short sum_unrolled2(long size, unsigned short * a) {
//    unsigned short sum = 0;
//    unsigned short sum1 = 0;
//    unsigned short sum2 = 0;
/     unsigned short sum3 = 0;
//    for (int i = 0; i < size; i + 4) {
//        sum1 += a[i];
//        sum2 += a[i + 1];
//        sum3 += a[i + 2];
//        sum3 += a[i + 3];
//    }
//    sum += (sum1 + sum2 + sum3 + sum4);

// This implementation follows the Linux x86-64 calling convention:
//    %rdi contains the size
//    %rsi contains the pointer a
// and
//    %ax needs to contain the result when the function returns
// in addition, this code uses
//    %rcx to store i

// the '.global' directive indicates to the assembler to make this symbol 
// available to other files.
.global sum_multiple_accum
sum_multiple_accum:
    // set sum (%ax) to 0
    xor %rax, %rax
    xor %r8, %r8
    xor %r9, %r9
    xor %r10, %r10
    xor %r11, %r11
    // return immediately; special case if size (%rdi) == 0
    test %rdi, %rdi
    je .L_done
    // store i = 0 in rcx
    movq $0, %rcx
// labels starting with '.L' are local to this file
.L_loop:
    // sum += a[i]
    addq (%rsi,%rcx,2), %r8
    // sum += a[i + 1]
    addq $1, %rcx
    addq (%rsi,%rcx,2), %r9
    // sum += a[i + 2]
    addq $1, %rcx
    addq (%rsi,%rcx,2), %r10
    // sum += a[i + 3]
    addq $1, %rcx
    addq (%rsi,%rcx,2), %r11
    // increment i for the loop
    addq $1, %rcx
    // if i is not yet at N then keep going
    cmpq %rdi, %rcx
    jl .L_loop
.L_done:
    addq %r8, %rax
    addq %r9, %rax
    addq %r10, %rax
    addq %r11, %rax
    retq

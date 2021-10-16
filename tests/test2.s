.global myCounter
.section test1
.extern x
.extern y
.equ a, 0x2
.equ b, 0x4
.equ c , 6
myCounter:
.word a
.word b
.word c # LC = 6
.skip 5
halt
ret
iret
int r0
not r1
push r2
pop r3
and r1 , r2
sub r3, r4
shl r5, r6
.section test2
ldr r0, $0x1
ldr r1, $2
str r0, 6
jmp %x
jne 5
jeq 0x4
cmp r2, r4 # test komentar
jmp *r5
jeq *[r3]
jne *[r2 + 6]
jmp *2
halt
.section myData
.word 0
.skip 5
# test komentar .skip 3
.end
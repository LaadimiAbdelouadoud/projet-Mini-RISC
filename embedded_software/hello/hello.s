# hello.s - Affiche "hello world!\n"
.section .text
.globl _start

_start:
    # Init stack pointer
    .word 0x82000117          # lui sp, 0x82000
    
    # Charger l'adresse I/O dans a3
    .word 0x100006b7          # lui a3, 0x10000
    
    # Afficher 'h'
    .word 0x06800513          # li a0, 'h'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'e'
    .word 0x06500513          # li a0, 'e'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'l'
    .word 0x06c00513          # li a0, 'l'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'l'
    .word 0x06c00513          # li a0, 'l'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'o'
    .word 0x06f00513          # li a0, 'o'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher ' '
    .word 0x02000513          # li a0, ' '
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'w'
    .word 0x07700513          # li a0, 'w'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'o'
    .word 0x06f00513          # li a0, 'o'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'r'
    .word 0x07200513          # li a0, 'r'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'l'
    .word 0x06c00513          # li a0, 'l'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher 'd'
    .word 0x06400513          # li a0, 'd'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher '!'
    .word 0x02100513          # li a0, '!'
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher '\n'
    .word 0x00a00513          # li a0, 10
    .word 0x00a6a023          # sw a0, 0(a3)
    
halt:
    .word 0x0000006f          # j halt

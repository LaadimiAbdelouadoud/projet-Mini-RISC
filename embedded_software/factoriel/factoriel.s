# factorial.s - VERSION FINALE CORRECTE
.section .text
.globl _start

_start:
    # Init stack pointer
    .word 0x82000117          # lui sp, 0x82000
    
    # Afficher "n? "
    .word 0x100006b7          # lui a3, 0x10000
    .word 0x06e00513          # li a0, 'n'
    .word 0x00a6a023          # sw a0, 0(a3)
    .word 0x03f00513          # li a0, '?'
    .word 0x00a6a023          # sw a0, 0(a3)
    .word 0x02000513          # li a0, ' '
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Lire depuis GETCHAR (0x1000000C)
    .word 0x100006b7          # lui a3, 0x10000
    .word 0x00c6a503          # lw a0, 12(a3)
    
    # Sauvegarder le caractère dans a2 AVANT de l'écraser
    .word 0x00050613          # mv a2, a0
    
    # Echo du caractère
    .word 0x100006b7          # lui a3, 0x10000
    .word 0x00a6a023          # sw a0, 0(a3)
    .word 0x00a00513          # li a0, 10        # newline (écrase a0)
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Convertir ASCII → entier (utiliser a2 qui contient le caractère)
    .word 0xfd060613          # addi a2, a2, -48
    
    # Calculer factoriel(a2)
    .word 0x00060513          # mv a0, a2        # a0 = n
    .word 0x00100593          # li a1, 1         # a1 = result = 1
    
fact_loop:
    .word 0x00050863          # beqz a0, +16
    .word 0x02a585b3          # mul a1, a1, a0
    .word 0xfff50513          # addi a0, a0, -1
    .word 0xff5ff06f          # j fact_loop
    
fact_end:
    # a1 contient le résultat
    # Afficher "= "
    .word 0x100006b7          # lui a3, 0x10000
    .word 0x03d00513          # li a0, '='
    .word 0x00a6a023          # sw a0, 0(a3)
    .word 0x02000513          # li a0, ' '
    .word 0x00a6a023          # sw a0, 0(a3)
    
    # Afficher le résultat (a1 à 0x10000004)
    .word 0x100006b7          # lui a3, 0x10000
    .word 0x00b6a223          # sw a1, 4(a3)
    
    # Newline
    .word 0x00a00513          # li a0, 10
    .word 0x100006b7          # lui a3, 0x10000
    .word 0x00a6a023          # sw a0, 0(a3)
    
halt:
    .word 0x0000006f          # j halt

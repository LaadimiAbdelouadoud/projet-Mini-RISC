================================================================================
                        ÉMULATEUR MINI-RISC
================================================================================

AUTEUR
------
Projet d'émulation d'un processeur RISC-V 32 bits
Réalisé par :
- Abdelouadoud LAADIMI
- Hussein HOUHOU
- Mohamad EL HAJJ


BUT DU PROJET
-------------
Créer un émulateur fonctionnel d'un processeur RISC-V 32 bits capable d'exécuter
des programmes assembleur. L'objectif était de comprendre le fonctionnement interne
d'un processeur à travers l'implémentation du cycle fetch-decode-execute.


FONCTIONNALITÉS INTÉGRÉES
--------------------------
+ Jeu d'instructions RV32I complet (base RISC-V 32 bits)
  - Arithmétique et logique (ADD, SUB, AND, OR, XOR, etc.)
  - Décalages (SLL, SRL, SRA)
  - Branchements conditionnels (BEQ, BNE, BLT, BGE, BLTU, BGEU)
  - Sauts (JAL, JALR)
  - Chargements et stockages mémoire (LB, LH, LW, SB, SH, SW)
  - Instructions immédiates (ADDI, ANDI, ORI, etc.)
  - Instructions upper immediate (LUI, AUIPC)

+ Extension RV32M (multiplication et division)
  - MUL, MULH, MULHSU, MULHU
  - DIV, DIVU, REM, REMU

+ 32 registres généraux (r0-r31, r0 câblé à 0)

+ Mémoire de 32 MiB (0x80000000 - 0x82000000)

+ Périphérique d'entrée/sortie CharOut (0x10000000)
  - Affichage de caractères
  - Affichage d'entiers signés
  - Affichage d'entiers en hexadécimal
  - Lecture de caractères (GETCHAR)

+ Vérification d'alignement et validation des adresses

+ Extension de signe correcte pour toutes les instructions


CE QUI FONCTIONNE
-----------------
- Tous les opcodes RV32I et RV32M sont implémentés
- Le cycle fetch-decode-execute fonctionne correctement
- Les programmes de test (hello.s et factorial.s) s'exécutent avec succès
- La gestion mémoire avec accès byte/half/word
- Les entrées/sorties via le périphérique CharOut
- La détection d'erreurs (adresses invalides, désalignement, etc.)


STRUCTURE DU PROJET
-------------------
MINIRISC/
├── emulator/                  # Émulateur principal
│   ├── main.c                 # Point d'entrée
│   ├── minirisc.c/.h          # Implémentation du processeur
│   ├── platform.c/.h          # Gestion mémoire et I/O
│   ├── Makefile               # Compilation
│   └── build/                 # Binaires compilés
│
└── embedded_software/         # Programmes de test
    ├── factorial/             # Calcul de factorielle
    │   ├── factorial.s        # Code assembleur
    │   ├── Makefile
    │   └── build/
    └── hello/                 # Hello World
        ├── hello.s            # Code assembleur
        ├── Makefile
        └── build/


COMPILATION
-----------

1. Compiler l'émulateur :
   cd emulator
   make

2. Compiler un programme de test :
   cd embedded_software/hello
   make

   cd ../factorial
   make


EXÉCUTION
---------

1. Exécuter le programme "Hello World" :
   cd emulator
   ./build/emulator ../embedded_software/hello/build/hello.bin

   Sortie attendue :
   Starting Mini-RISC emulator...
   Loaded XX bytes from '../embedded_software/hello/build/hello.bin'
   Hello, World!

2. Exécuter le programme "Factorial" :
   cd emulator
   ./build/emulator ../embedded_software/factorial/build/factorial.bin

   Le programme calcule des factorielles et affiche les résultats.

3. Syntaxe générale :
   ./build/emulator <chemin/vers/programme.bin>


NETTOYAGE
---------
Pour supprimer les fichiers compilés :

cd emulator
make clean

cd embedded_software/hello
make clean

cd ../factorial
make clean


DÉTAILS TECHNIQUES
------------------
- Architecture : RV32IM (RISC-V 32 bits avec extension M)
- Mémoire : 32 MiB little-endian
- Adresse de démarrage : 0x80000000
- Format des instructions : 32 bits
- Encodage : Little-endian



DÉPENDANCES
-----------
- GCC (compilateur C)
- Make (système de build)
- Assembleur RISC-V pour créer de nouveaux programmes de test


NOTES
-----
- Le registre r0 est toujours égal à 0 (câblé matériellement)
- Les accès mémoire doivent être alignés selon leur taille
- L'émulateur s'arrête sur une erreur (instruction invalide, accès mémoire
  illégal, etc.)
- Les programmes doivent être au format binaire brut (pas d'ELF)


================================================================================

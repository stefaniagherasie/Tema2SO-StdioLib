# Tema2SO-StdioLibrary
[Tema2 - Sisteme de Operare] Tema presupune implementarea unei versiuni minimale a bibliotecii sdio, care sa permita lucrul cu fisiere. <br>
Enunt: https://ocw.cs.pub.ro/courses/so/teme/tema-2

#### COMPILARE SI TESTARE

► se foloseste Makefile pentru compilarea bibliotecii dinamice libso_stdio.so (make) <br>
► se foloseste Makefile.checker pentru testare (directorul trebuie sa contina libso_stdio.so)
  
    TESTARE CHECKER:
        make -f Makefile.checker
        DO_CLEANUP=no make -f Makefile.checker 
    TESTAREA UNUI TEST:
        _test/run_test.sh init
        _test/run_test.sh <test>
        _test/run_test.sh clean



#### ORGANIZARE
Tema are urmatoarea organizare:
- so_stdio.h / so_stdio.c - implementarea functiilor din biblioteca
- Makefile - compilarea bibliotecii dinamice libso_stdio.so
- Makefile.checker - testarea bibliotecii dinamice
- run_all.sh - script care ruleaza testele
- _test - care contine un script de rulare individuala a testelor, fisierele sursa *.c care folosesc biblioteca dinamica, directorul work unde se stocheaza outputurile etc.

#### IMPLEMENTARE
Am inceput prin a creea structura "so_file", care contine un buffer si diverse 
campuri care ajuta la implementare (fd - file descriptor, offset - pozitia in buffer etc.).
Functiile so_fopen si so_fclose se ocupa de deschiderea/inchiderea fisierului si alocarea/
dezalocarea de memorie. Functia so_fflush e folosita pentru a goli bufferul in fisier 
atunci cand bufferul se umple sau cand este ultima scriere asociata operatiei de fwrite.


Functiile so_fread si so_fwrite scriu/citesc prin apelari succesive ale functiilor so_fgetc/
so_fputc.


Functia so_fseek scrie continutul bufferului in fisier sau il goleste (depinde de ultima
operatie executata), dupa care pozitioneaza cursorul asa cum specifica parametrii dati. 
so_ftell intoarce pozitia curenta a cursorului, iar so_ferror si so_feof spune daca a aparut
o eroare / daca s-a ajuns la sfarsitul fisierului. 

#### BIBLIOGRAFIE
 - https://ocw.cs.pub.ro/courses/so/curs/fs-ops
 - https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
 - https://www.tutorialspoint.com/c_standard_library/stdio_h.htm
 

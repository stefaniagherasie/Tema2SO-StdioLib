# Tema2SO-StdioLib
[Tema2 - Sisteme de Operare] Tema presupune implementarea unei versiuni minimale a bibliotecii sdio, care sa permita lucrul cu fisiere. <br>
Enuntul se gaseste [aici](https://ocw.cs.pub.ro/courses/so/teme/tema-2). <br>
Scheletul de cod se gaseste [aici](https://github.com/systems-cs-pub-ro/so/tree/master/assignments/2-stdio).

#### COMPILARE SI RULARE

► se foloseste Makefile pentru compilarea bibliotecii dinamice libso_stdio.so (make) <br>
► se foloseste Makefile.checker pentru testare (directorul trebuie sa contina libso_stdio.so)
  
    make & make -f Makefile.checker
    DO_CLEANUP=no make -f Makefile.checker 
    
    RULARE UNUI TEST:     _test/run_test.sh init
                          _test/run_test.sh <test>
                          _test/run_test.sh clean



#### ORGANIZARE
Tema are urmatoarea organizare:
- ```so_stdio.h / so_stdio.c``` - implementarea functiilor din biblioteca
- ```Makefile``` - compilarea bibliotecii dinamice libso_stdio.so
- ```Makefile.checker``` - testarea bibliotecii dinamice
- ```run_all.sh``` - script care ruleaza testele
- ```_test``` - care contine un script de rulare individuala a testelor, fisierele sursa *.c care folosesc biblioteca dinamica, directorul work unde se stocheaza outputurile, etc.

#### IMPLEMENTARE
Am inceput prin a creea structura ```so_file```, care contine urmatoarele campuri:
```c
struct _so_file {
	char *buffer;
	int fd;           // file descriptor
	int offset;       // position in buffer
	int cursor;       // position in file
	int bytes_read;   // number of bytes returned from read
	int last_op;      // last operation: 1 - write, 2 - read
	int eof;          // mark reaching EOF
	int err;          // mark an error
};
```


Functiile ```so_fopen``` si ```so_fclose``` se ocupa de deschiderea/inchiderea fisierului si alocarea/
dezalocarea de memorie. Functia ```so_fflush``` e folosita pentru a goli bufferul in fisier 
atunci cand bufferul se umple sau cand este ultima scriere asociata operatiei de fwrite.

Functiile ```so_fread``` si ```so_fwrite``` scriu/citesc prin apelari succesive ale functiilor ```so_fgetc/so_fputc```.

Functia ```so_fseek``` scrie continutul bufferului in fisier sau il goleste (depinde de ultima
operatie executata), dupa care pozitioneaza cursorul asa cum specifica parametrii dati. 
```so_ftell``` intoarce pozitia curenta a cursorului, iar ```so_ferror/so_feof``` spune daca a aparut
o eroare / daca s-a ajuns la sfarsitul fisierului. 

#### MENTIUNE

Nu am implementat rularea de procese (functiile so_popen si so_pclose).

#### BIBLIOGRAFIE
 - https://ocw.cs.pub.ro/courses/so/curs/fs-ops
 - https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
 - https://www.tutorialspoint.com/c_standard_library/stdio_h.htm
 

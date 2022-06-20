# Executable-Loader
 Shared library that acts as an executable loader.
 
 Buzera Tiberiu 333CA

Tema 3 Loader de Executabile

Enunț
Să se implementeze sub forma unei biblioteci partajate/dinamice un loader de fișiere executabile în format ELF pentru Linux și PE pentru Windows. Loader-ul va încărca fișierul executabil în memorie pagină cu pagină, folosind un mecanism de tipul demand paging - o pagină va fi încărcată doar în momentul în care este nevoie de ea. Pentru simplitate, loader-ul va rula doar executabile statice - care nu sunt link-ate cu biblioteci partajate/dinamice.

Implementare
In scheletul de cod se gasesc deja implementate de catre echipa de so structura unui segment din executabil, structura pentru executabil in care se gaseste un vector de segmente si functiile de parsare si rulare al executabilului.
Astfel, eu am implementat un signal handler pentru semnalul SIGSEGV, in fisierul loader.c in functia so_init_loader am am declarat o strucura de tip sigaction si am dat ca in cazul in care se primeste semnalul sa se ruleze functia signal_action ce are 3 paramentrii, numarul semnalului, o strucura de tip siginfo_t si context cu privire la timpul si locatia semnalului. In strucura siginfo_t se gaseste un camp addr in interiorul caruia se afla adresa unde s-a produs page fault-ul. In functia signal_action verificam daca adresa de memorie se gasete in interiorul paginilor mapate in strucura, daca nu rulam handlerul default. Daca se gaseste in structura mapam pagina, citim din fisier pentru a completa pagina cu informatii si apoi ii dam paginii permisiunile segmentului.

Dificultati intampinate
A fost dificila partea de debugging si partea de combinare a tuturor notiunilor invatate la curs si laborator

Pentru ajutor am folosit laboratoarele de pe ocw si paginile de manual pentru fiecare din comenzi.

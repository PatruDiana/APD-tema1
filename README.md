# APD-tema1

Patru Diana-Georgiana, 332CA

 Tema #1 Generare paralela de fractali folosind multimile Mandelbrot si Julia

    Pentru a rezolva prima tema la Algoritmi Paraleli si Distribuiti, am folosit ca schelet principal rezolvarea secventiala aflata in arhiva careia i-am efectuat urmatoarele modificari pentru a realiza paralelizarea programului:
    - Pentru a accesa mai usor datele citite din fiecare fisier (Julia si Mandelbort) am ales sa declar global urmatoarele variabile: width, height, result, barrrier, o instanta a structurii par si numarul de thread-uri. Astfel, singurul argument ce este dat fiecarui thread este chiar id-ul lui; 
    - Crearea celor P thread-uri se face in main, fiind urmata de join-ul lor pentru ca thread-ul main sa isi termine ultimul executia;
    - Inainte de crearea si lansarea in executie a celor P threaduri, in main are loc citirea parametrilor de intrare de la fisierul Julia, alocarea matricii urmand ca apoi in cele P threaduri sa se verifice daca trebuie calculata multimea Julia/Mandelbort, fiind apelata functia corespunzatoare;
    - In functia in care este calculata multimea Julia, am paralelizat for-ul din exterior, calculand 2 indici start si end in functie de width-ul si numarul de threaduri. Deoarece exista posilitatea ca unele thread-uri sa termine mai  rapid sau mai lent calculul, am pus o bariera inainte de transformarea  rezultatului din coodonate matematice in coodonate ecran, urmand sa recalculez indicii de start si end in functie de height / 2 si sa paralelizez si for-ul pentru transformarea rezultatului, punand o bariera si dupa pentru a sincroniza  thread-urile;
    - Am ales apoi ca un singur thread (cel cu id-ul 0) sa efectueze scrierea  in fisierul de out pentru multimea Julia dar si sa citeasca si sa prelucreze datele de intrare pentru Mandelbort, in timp ce celelalte thread-uri sunt blocate la o noua bariera;
    - Paralelizarea folosita pentru calculul multimii Mandelbort este similara cu cea descrisa mai sus pentru multimea Julia. Barierele sunt situate la fel  ca in functia run_julia() iar indicii start si end sunt calculati dupa aceeasi formula.
    - Tot thread-ul 0 va fi apoi cel care va efectua singur scrierea in  fisier-ul de out. 

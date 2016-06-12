# mpi-rainbow-tables
## Opis projektu
Projekt zakłada łamanie hasła złożonego z pięciu cyfr zahasowanego metodą MD5. MPI w
projekcie zostanie wykorzystane podczas tworzenia tablicy tęczowej. Między innymi odpowiadać
będzie za rozdysponowanie haseł, z których będą powstać łańcuchy wraz z liczbą cykli potrzebnych do
ich wygenerowania. Dodatkowo podczas łamania hasha z wykorzystaniem tablicy tęczowej
wykorzystany zostanie on do równomiernego podziału tablicy tęczowej między wszystkimi procesami
w celu zmniejszenia zakresu danych jakie porównad musiałby jeden proces gdyby tego nie
zastosowad oraz do poinformowania pozostałych procesów o ewentualnym znalezieniu łaocucha w
którym zawiera się hasła z którego został stworzony łamany hash.
## Kompilacja
make build	Kompiluje projekt
make test	Kompiluje testy
## Uruchomienie
mpirun -np <n> mpi-rainbow-tables
(opcje opisane w opcji pomocy: mpi-rainbow-tables --help)

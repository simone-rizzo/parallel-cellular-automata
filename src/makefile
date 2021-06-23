main: main.cpp
	g++-10 main.cpp -pthread -lX11 -lpng  -o main_parallel

seq: sequential.cpp
	g++-10 sequential.cpp -pthread -lX11 -lpng -o sequential
	
ff: cellular_automata_ff.cpp
	g++-10 cellular_automata_ff.cpp -pthread -lpng -lX11 -I/mnt/t/Programmi/fastflow-master -o cellular_ff

ffnopinned: cellular_automata_ff.cpp
	g++-10 cellular_automata_ff.cpp -pthread -lpng -lX11 -I/mnt/t/Programmi/fastflow-master -DNO_DEFAULT_MAPPING -o cellular_ff
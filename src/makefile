main: main.cpp
	g++-10 main.cpp -pthread -lX11 -O3 -D PARALLEL_WRITE -I/mnt/c/Users/Simone/Documents/parallel-cellular-automata/fastflow -o main
	
fastflow: cellular_automata_ff.cpp
	g++-10 cellular_automata_ff.cpp -pthread -lX11 -O3 -I/mnt/c/Users/Simone/Documents/parallel-cellular-automata/fastflow -o cellular_automata_ff
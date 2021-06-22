main: main.cpp
	g++-10 main.cpp -pthread -lX11 -lpng  -o main_parallel

seq: sequential.cpp
	g++-10 sequential.cpp -pthread -lX11 -lpng -o sequential
	
fastflow: cellular_automata_ff.cpp
	g++-10 cellular_automata_ff.cpp -pthread -lpng -lX11 -I/mnt/t/Programmi/fastflow-master -o cellular_ff

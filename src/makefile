
FF_ROOT	= -I/mnt/t/Programmi/fastflow-master # leave empty if FastFlow is installed in the system in the main path for libraries
LDFLAGS	= -pthread -lX11 -lpng
CXX = g++

main: main.cpp
	$(CXX) main.cpp $(LDFLAGS)  -o main_parallel

seq: sequential.cpp
	$(CXX) sequential.cpp $(LDFLAGS) -o sequential
	
ff: cellular_automata_ff.cpp
	$(CXX) cellular_automata_ff.cpp $(LDFLAGS) $(FF_ROOT) -o cellular_ff

ffnopinned: cellular_automata_ff.cpp
	$(CXX) cellular_automata_ff.cpp $(LDFLAGS) $(FF_ROOT) -DNO_DEFAULT_MAPPING -o cellular_ff

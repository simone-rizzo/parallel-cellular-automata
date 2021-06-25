
FF_ROOT	= -I/mnt/t/Programmi/fastflow-master # leave empty if FastFlow is installed in the system in the main path for libraries
LDFLAGS	= -pthread -lX11 -lpng -O3 -finline-functions
CXX = g++
DEF = #-DLAST_WRITE #-DPARALLEL_WRITE 

main: main.cpp
	$(CXX) main.cpp $(LDFLAGS) $(FF_ROOT) $(DEF)  -o main_parallel

seq: sequential.cpp
	$(CXX) sequential.cpp $(LDFLAGS) -o sequential

ff: cellular_automata_ff.cpp
	$(CXX) cellular_automata_ff.cpp $(LDFLAGS) $(FF_ROOT) $(DEF) -o cellular_ff

ffnopinned: cellular_automata_ff.cpp
	$(CXX) cellular_automata_ff.cpp $(LDFLAGS) $(FF_ROOT) $(DEF) -DNO_DEFAULT_MAPPING -o cellular_ff_nopinning

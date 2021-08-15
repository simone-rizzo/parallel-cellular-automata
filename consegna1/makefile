
FF_ROOT	= -I/home/kkk/fastflow # leave empty if FastFlow is installed in the system in the main path for libraries
PARFLAGS = -pthread 
LDFLAGS	=  -lX11 -lpng -O3 -finline-functions
CXX = g++-10 
DEF = -DPARALLEL_WRITE 

mine: cellular_automata.cpp
	$(CXX) cellular_automata.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) -o ca

minepar: cellular_automata.cpp
	$(CXX) cellular_automata.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) $(DEF) -o ca_parallel_write

seq: sequential.cpp
	$(CXX) sequential.cpp $(PARFLAGS) $(LDFLAGS) -o sequential

ff: cellular_automata_ff.cpp
	$(CXX) cellular_automata_ff.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) -o ff

ffpar: cellular_automata_ff.cpp
	$(CXX) cellular_automata_ff.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) $(DEF) -o ff_parallel_write

clean:
	
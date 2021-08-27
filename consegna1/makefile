
FF_ROOT	= -I/home/kkk/fastflow # leave empty if FastFlow is installed in the system in the main path for libraries
PARFLAGS = -pthread 
LDFLAGS	=  -lX11 -lpng -O3 -finline-functions
CXX = g++-10 

mine: mine.cpp
	$(CXX) mine.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) -o mine

seq: sequential.cpp
	$(CXX) sequential.cpp $(PARFLAGS) $(LDFLAGS) -o sequential

ffpar: ff_parfor.cpp
	$(CXX) ff_parfor.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) -o ff_parfor

fffarm:ff_farm.cpp
	$(CXX) ff_farm.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) -o ff_farm

ffparw: ff_parfor.cpp
	$(CXX) ff_parfor.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) -o ff_parfor_write

fffarmw:ff_farm.cpp
	$(CXX) ff_farm.cpp $(PARFLAGS) $(LDFLAGS) $(FF_ROOT) -o ff_farm_write

	
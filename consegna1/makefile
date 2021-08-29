
FF_ROOT	= -I/home/kkk/fastflow # leave empty if FastFlow is installed in the system in the main path for libraries
LDFLAGS	=  -pthread -lX11 -lpng -O3 -finline-functions
CXX = g++-10 
IMG = -DWIMG
TARGETS = mine sequential ff_parfor ff_farm

$(TARGETS): %: %.cpp
	$(CXX) $<  $(LDFLAGS) $(FF_ROOT) -o $@

sequentialw: sequential.cpp
	$(CXX) sequential.cpp $(LDFLAGS) $(IMG) -o sequential_write

minew: mine.cpp
	$(CXX) mine.cpp  $(LDFLAGS) $(IMG) -o mine_write

ffparw: ff_parfor.cpp
	$(CXX) ff_parfor.cpp  $(LDFLAGS) $(FF_ROOT) $(IMG) -o ff_parfor_write

fffarmw:ff_farm.cpp
	$(CXX) ff_farm.cpp  $(LDFLAGS) $(FF_ROOT) $(IMG) -o ff_farm_write

	
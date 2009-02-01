
CXXFLAGS = -g -O2 -Wall -Iext/include
LDFLAGS = -lfcgi -Lext/ -lv8_i586_linux
OUTPUT = fastjs

all::
	$(CXX) $(CXXFLAGS) $(LDFLAGS) *.cc -o $(OUTPUT)

clean:
	rm -f *.o a.out $(OUTPUT)

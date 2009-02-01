
CXXFLAGS = -g -O2 -Wall -Iext/include
LDFLAGS = -lfcgi -Lext/ -lpthread
OUTPUT = fastjs

all::
	$(CXX) $(CXXFLAGS) $(LDFLAGS) *.cc ext/libv8_i586_linux.a -o $(OUTPUT)

clean:
	rm -f *.o a.out $(OUTPUT)

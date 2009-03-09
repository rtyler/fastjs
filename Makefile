
CXXFLAGS = -g -O2 -Wall -Iext/include `pkg-config --cflags glib-2.0`
LDFLAGS = -lfcgi -Lext/ -lpthread `pkg-config --libs glib-2.0`
OUTPUT = fastjs

all::
	$(CXX) $(CXXFLAGS) $(LDFLAGS) *.cc ext/libv8_i586_linux.a -o $(OUTPUT)

clean:
	rm -f *.o a.out $(OUTPUT)


CXXFLAGS = -g -O2 -Wall 
LDFLAGS = -lfcgi
OUTPUT = fastjs

all::
	$(CXX) $(CXXFLAGS) $(LDFLAGS) *.cc -o $(OUTPUT)

clean:
	rm -f *.o a.out $(OUTPUT)

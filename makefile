CC=g++
CFLAGS=-c -O3
LDFLAGS=-lrt
SOURCES=netmon.cpp strutil.cpp list.cpp netif.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=netmon

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	    
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf  *.o netmon


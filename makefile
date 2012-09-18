CC = g++
CFLAGS = -c -O3
LDFLAGS = -lrt
INCLUDE = -I

BIN_DIR = bin/
BUILD_DIR = build/
INC_DIR = inc/
SRC_DIR = src/

BINARY = netmon
SOURCES = $(shell ls $(SRC_DIR))
OBJECTS = $(SOURCES:.cpp=.o)

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(addprefix $(BUILD_DIR),$^) -o $(BIN_DIR)$@ $(LDFLAGS)

%.o: $(SRC_DIR)%.cpp
	$(CC) $(CFLAGS) $(INCLUDE)$(INC_DIR) $< -o $(BUILD_DIR)$@

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)* $(BIN_DIR)*


CC=gcc

INCLUDES= -Iincludes
CCFLAGS= -g -Wall $(INCLUDES)
LDFLAGS=
SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:.c=.o)
TEMP=$(OBJECTS:src/%.o=%.o)
OBJ1=$(filter-out receiver.o,$(TEMP))
OBJ2=$(filter-out sender.o,$(TEMP))  
TARGET1=sender
TARGET2=receiver

all: $(TARGET1) $(TARGET2)
	mkdir -p object_files
	mv *.o object_files

$(TARGET1): $(OBJECTS)
	$(CC) -o $@ $(OBJ1) $(LDFLAGS)

$(TARGET2): $(OBJECTS) 
	$(CC) -o $@ $(OBJ2) $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CCFLAGS) -c $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f object_files/*.o $(TARGET)

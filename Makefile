SRC_DIR=.
SRCS=$(shell find $(SRC_DIR) -name '*.cpp')
OBJS=$(subst .cpp,.o,$(SRCS))
TARGET=rcopy.exe

.PHONY: default all tidy clean

default: all

all: $(TARGET)

%.o: %.cpp
	g++ -c $< -o $@

$(TARGET): $(OBJS)
	g++ $^ -o $@

tidy:
	rm -f $(OBJS)

clean: tidy
	rm -f $(TARGET)
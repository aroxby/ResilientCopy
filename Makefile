SRC_DIR=.
SRCS=$(shell find $(SRC_DIR) -name '*.cpp')
OBJS=$(subst .cpp,.o,$(SRCS))
TARGET=rcopy.exe
# This is random 3.8GB file on my drive.  You Substitute this with any other large file.
TEST_SRC="C:\Program Files (x86)\Steam\steamapps\common\Subnautica\Subnautica_Data\resources.assets.resS"
TEST_DST=/dev/null

.PHONY: default all tidy clean

default: all

all: $(TARGET)

%.o: %.cpp
	g++ -c $< -o $@

$(TARGET): $(OBJS)
	g++ $^ -o $@

test: $(TARGET)
	./$(TARGET) $(TEST_SRC) $(TEST_DST)

tidy:
	rm -f $(OBJS)

clean: tidy
	rm -f $(TARGET)
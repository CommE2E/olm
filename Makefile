#!/usr/bin/make -f

BUILD_DIR := build
OPTIMIZE_FLAGS ?= -g -O3
TEST_OPTIMIZE_FLAGS ?= -g -O0
CC = gcc
TARGET := $(BUILD_DIR)/libolm.so

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*.c)
OBJECTS := $(patsubst src/%,$(BUILD_DIR)/%,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES))))
TEST_SOURCES := $(wildcard tests/test_*.cpp) $(wildcard tests/test_*.c)
TEST_BINARIES := $(patsubst tests/%,$(BUILD_DIR)/%,$(patsubst %.c,%,$(patsubst %.cpp,%,$(TEST_SOURCES))))

CPPFLAGS += -Iinclude -Ilib
CFLAGS += -Wall --std=c89 -fPIC
CXXFLAGS += -Wall --std=c++11 -fPIC
LDFLAGS += -Wall

# generate .d files when compiling
CPPFLAGS += -MMD

### per-target variables

$(OBJECTS): CFLAGS += $(OPTIMIZE_FLAGS)
$(OBJECTS): CXXFLAGS += $(OPTIMIZE_FLAGS)
$(TARGET): LDFLAGS += $(OPTIMIZE_FLAGS)

$(TEST_BINARIES): CPPFLAGS += -Itests/include
$(TEST_BINARIES): LDLIBS += -lolm
$(TEST_BINARIES): LDFLAGS += $(TEST_OPTIMIZE_FLAGS) -L$(BUILD_DIR)

### top-level targets

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) --shared -fPIC $^ $(OUTPUT_OPTION)

clean:;
	rm -rf $(OBJECTS) $(OBJECTS:.o=.d) \
               $(TEST_BINARIES) $(TEST_BINARIES:=.d) \
               $(TARGET)

build_tests: $(TEST_BINARIES)

test: build_tests
	for i in $(TEST_BINARIES); do \
	    echo $$i; \
	    LD_LIBRARY_PATH=$(BUILD_DIR) $$i || exit $$?; \
	done

### rules for building objects
$(BUILD_DIR)/%.o: src/%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/%.o: src/%.cpp
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/%: tests/%.c
	$(LINK.c) $< $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/%: tests/%.cpp
	$(LINK.cc) $< $(LOADLIBES) $(LDLIBS) -o $@


### dependencies

$(TEST_BINARIES): $(TARGET)

-include $(OBJECTS:.o=.d)
-include $(TEST_BINARIES:=.d)

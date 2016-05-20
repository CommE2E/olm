#!/usr/bin/make -f

BUILD_DIR := build
OPTIMIZE_FLAGS ?= -g -O3
TEST_OPTIMIZE_FLAGS ?= -g -O0
JS_OPTIMIZE_FLAGS ?= -O3
CC = gcc
EMCC = emcc
TARGET := $(BUILD_DIR)/libolm.so
JS_TARGET := javascript/olm.js

JS_EXPORTED_FUNCTIONS := javascript/exported_functions.json

PUBLIC_HEADERS := include/olm/olm.hh

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*.c)
OBJECTS := $(patsubst src/%,$(BUILD_DIR)/%,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES))))
TEST_SOURCES := $(wildcard tests/test_*.cpp) $(wildcard tests/test_*.c)
TEST_BINARIES := $(patsubst tests/%,$(BUILD_DIR)/%,$(patsubst %.c,%,$(patsubst %.cpp,%,$(TEST_SOURCES))))
JS_OBJECTS := $(patsubst %.o,%.js.bc,$(OBJECTS))
JS_PRE := $(wildcard javascript/*pre.js)
JS_POST := $(wildcard javascript/*post.js)

CPPFLAGS += -Iinclude -Ilib
CFLAGS += -Wall --std=c89 -fPIC
CXXFLAGS += -Wall --std=c++11 -fPIC
LDFLAGS += -Wall

EMCCFLAGS = --closure 1 --memory-init-file 0 -s NO_FILESYSTEM=1 -s INVOKE_RUN=0
# NO_BROWSER is kept for compatibility with emscripten 1.35.24, but is no
# longer needed.
EMCCFLAGS += -s NO_BROWSER=1

EMCC.c = $(EMCC) $(CFLAGS) $(CPPFLAGS) -c
EMCC.cc = $(EMCC) $(CXXFLAGS) $(CPPFLAGS) -c
EMCC_LINK = $(EMCC) $(LDFLAGS) $(EMCCFLAGS)

# generate .d files when compiling
CPPFLAGS += -MMD

### per-target variables

$(OBJECTS): CFLAGS += $(OPTIMIZE_FLAGS)
$(OBJECTS): CXXFLAGS += $(OPTIMIZE_FLAGS)
$(TARGET): LDFLAGS += $(OPTIMIZE_FLAGS)

$(TEST_BINARIES): CPPFLAGS += -Itests/include
$(TEST_BINARIES): LDFLAGS += $(TEST_OPTIMIZE_FLAGS) -L$(BUILD_DIR)

$(JS_OBJECTS): CFLAGS += $(JS_OPTIMIZE_FLAGS)
$(JS_OBJECTS): CXXFLAGS += $(JS_OPTIMIZE_FLAGS)
$(JS_TARGET): LDFLAGS += $(JS_OPTIMIZE_FLAGS)

### top-level targets

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) --shared -fPIC \
	    -Wl,--version-script,version_script.ver \
            $(OUTPUT_OPTION) $(OBJECTS)

js: $(JS_TARGET)
.PHONY: js

$(JS_TARGET): $(JS_OBJECTS) $(JS_PRE) $(JS_POST) $(JS_EXPORTED_FUNCTIONS)
	$(EMCC_LINK) \
	       --pre-js $(JS_PRE) --post-js $(JS_POST) \
	       -s "EXPORTED_FUNCTIONS=@$(JS_EXPORTED_FUNCTIONS)" \
               $(JS_OBJECTS) -o $@

clean:;
	rm -rf $(OBJECTS) $(OBJECTS:.o=.d) \
               $(TEST_BINARIES) $(TEST_BINARIES:=.d) \
               $(JS_OBJECTS) $(JS_TARGET) \
               $(JS_EXPORTED_FUNCTIONS) \
               $(TARGET)

build_tests: $(TEST_BINARIES)

test: build_tests
	for i in $(TEST_BINARIES); do \
	    echo $$i; \
	    $$i || exit $$?; \
	done

$(JS_EXPORTED_FUNCTIONS): $(PUBLIC_HEADERS)
	perl -MJSON -ne '/(olm_[^( ]*)\(/ && push @f, "_$$1"; END { print encode_json \@f }' $^ > $@.tmp
	mv $@.tmp $@

### rules for building objects
$(BUILD_DIR)/%.o: src/%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/%.o: src/%.cpp
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/%.js.bc: src/%.c
	$(EMCC.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/%.js.bc: src/%.cpp
	$(EMCC.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/%: tests/%.c $(OBJECTS)
	$(LINK.c) $< $(OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/%: tests/%.cpp $(OBJECTS)
	$(LINK.cc) $< $(OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@


### dependencies

-include $(OBJECTS:.o=.d)
-include $(JS_OBJECTS:.bc=.d)
-include $(TEST_BINARIES:=.d)

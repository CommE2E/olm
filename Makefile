#!/usr/bin/make -f

BUILD_DIR := build
RELEASE_OPTIMIZE_FLAGS ?= -g -O3
DEBUG_OPTIMIZE_FLAGS ?= -g -O0
JS_OPTIMIZE_FLAGS ?= -O3
CC = gcc
EMCC = emcc
RELEASE_TARGET := $(BUILD_DIR)/libolm.so
DEBUG_TARGET := $(BUILD_DIR)/libolm_debug.so
JS_TARGET := javascript/olm.js

JS_EXPORTED_FUNCTIONS := javascript/exported_functions.json

PUBLIC_HEADERS := include/olm/olm.h

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*.c)
RELEASE_OBJECTS := $(patsubst src/%,$(BUILD_DIR)/release/%,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES))))
DEBUG_OBJECTS := $(patsubst src/%,$(BUILD_DIR)/debug/%,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES))))
TEST_SOURCES := $(wildcard tests/test_*.cpp) $(wildcard tests/test_*.c)
TEST_BINARIES := $(patsubst tests/%,$(BUILD_DIR)/tests/%,$(patsubst %.c,%,$(patsubst %.cpp,%,$(TEST_SOURCES))))
JS_OBJECTS := $(patsubst src/%,$(BUILD_DIR)/javascript/%,$(patsubst %.c,%.js.bc,$(patsubst %.cpp,%.js.bc,$(SOURCES))))
JS_PRE := $(wildcard javascript/*pre.js)
JS_POST := $(wildcard javascript/*post.js)

CPPFLAGS += -Iinclude -Ilib
CFLAGS += -Wall -Werror -std=c89 -fPIC
CXXFLAGS += -Wall -Werror -std=c++11 -fPIC
LDFLAGS += -Wall -Werror

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

$(RELEASE_OBJECTS): CFLAGS += $(RELEASE_OPTIMIZE_FLAGS)
$(RELEASE_OBJECTS): CXXFLAGS += $(RELEASE_OPTIMIZE_FLAGS)
$(RELEASE_TARGET): LDFLAGS += $(RELEASE_OPTIMIZE_FLAGS)

$(DEBUG_OBJECTS): CFLAGS += $(DEBUG_OPTIMIZE_FLAGS)
$(DEBUG_OBJECTS): CXXFLAGS += $(DEBUG_OPTIMIZE_FLAGS)
$(DEBUG_TARGET): LDFLAGS += $(DEBUG_OPTIMIZE_FLAGS)

$(TEST_BINARIES): CPPFLAGS += -Itests/include
$(TEST_BINARIES): LDFLAGS += $(TEST_OPTIMIZE_FLAGS) -L$(BUILD_DIR)

$(JS_OBJECTS): CFLAGS += $(JS_OPTIMIZE_FLAGS)
$(JS_OBJECTS): CXXFLAGS += $(JS_OPTIMIZE_FLAGS)
$(JS_TARGET): LDFLAGS += $(JS_OPTIMIZE_FLAGS)

### top-level targets

lib: $(RELEASE_TARGET)
.PHONY: lib

# Make sure that the build directory exists.
# We can't check the build directory into git because it is empty.
makedirs:
	mkdir -p $(BUILD_DIR)/release $(BUILD_DIR)/debug $(BUILD_DIR)/javascript $(BUILD_DIR)/tests
.PHONY: makedirs

$(RELEASE_TARGET): $(RELEASE_OBJECTS)
	$(CXX) $(LDFLAGS) --shared -fPIC \
            -Wl,--version-script,version_script.ver \
            $(OUTPUT_OPTION) $(RELEASE_OBJECTS)

debug: $(DEBUG_TARGET)
.PHONY: debug

$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	$(CXX) $(LDFLAGS) --shared -fPIC \
            -Wl,--version-script,version_script.ver \
            $(OUTPUT_OPTION) $(DEBUG_OBJECTS)


js: $(JS_TARGET)
.PHONY: js

$(JS_TARGET): $(JS_OBJECTS) $(JS_PRE) $(JS_POST) $(JS_EXPORTED_FUNCTIONS)
	$(EMCC_LINK) \
               --pre-js $(JS_PRE) --post-js $(JS_POST) \
               -s "EXPORTED_FUNCTIONS=@$(JS_EXPORTED_FUNCTIONS)" \
               $(JS_OBJECTS) -o $@

clean:;
	rm -rf $(RELEASE_OBJECTS) $(RELEASE_OBJECTS:.o=.d) \
               $(DEBUG_OBJECTS) $(DEBUG_OBJECTS:.o=.d) \
               $(TEST_BINARIES) $(TEST_BINARIES:=.d) \
               $(JS_OBJECTS) $(JS_OBJECTS:.bc=.d) $(JS_TARGET) \
               $(JS_EXPORTED_FUNCTIONS) \
               $(RELEASE_TARGET) $(DEBUG_TARGET)\

build_tests: $(TEST_BINARIES)

test: build_tests
	for i in $(TEST_BINARIES); do \
	    echo $$i; \
	    $$i || exit $$?; \
	done

$(JS_EXPORTED_FUNCTIONS): $(PUBLIC_HEADERS)
	perl -MJSON -ne '/(olm_[^( ]*)\(/ && push @f, "_$$1"; END { print encode_json \@f }' $^ > $@.tmp
	mv $@.tmp $@

all: test js lib debug
.PHONY: lib

### rules for building objects
$(BUILD_DIR)/release/%.o: src/%.c | makedirs
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/release/%.o: src/%.cpp | makedirs
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/debug/%.o: src/%.c | makedirs
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/debug/%.o: src/%.cpp | makedirs
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/javascript/%.js.bc: src/%.c | makedirs
	$(EMCC.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/javascript/%.js.bc: src/%.cpp | makedirs
	$(EMCC.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/tests/%: tests/%.c $(DEBUG_OBJECTS)
	$(LINK.c) $< $(DEBUG_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/tests/%: tests/%.cpp $(DEBUG_OBJECTS)
	$(LINK.cc) $< $(DEBUG_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@


### dependencies

-include $(RELEASE_OBJECTS:.o=.d)
-include $(DEBUG_OBJECTS:.o=.d)
-include $(JS_OBJECTS:.bc=.d)
-include $(TEST_BINARIES:=.d)

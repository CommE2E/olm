#!/usr/bin/make -f

MAJOR := 1
MINOR := 3
PATCH := 0
VERSION := $(MAJOR).$(MINOR).$(PATCH)
PREFIX ?= /usr/local
BUILD_DIR := build
RELEASE_OPTIMIZE_FLAGS ?= -g -O3
DEBUG_OPTIMIZE_FLAGS ?= -g -O0
JS_OPTIMIZE_FLAGS ?= -O3
FUZZING_OPTIMIZE_FLAGS ?= -O3
CC = gcc
EMCC = emcc
AFL_CC = afl-gcc
AFL_CXX = afl-g++

RELEASE_TARGET := $(BUILD_DIR)/libolm.so.$(VERSION)
DEBUG_TARGET := $(BUILD_DIR)/libolm_debug.so.$(VERSION)
JS_TARGET := javascript/olm.js

JS_EXPORTED_FUNCTIONS := javascript/exported_functions.json

PUBLIC_HEADERS := include/olm/olm.h include/olm/outbound_group_session.h include/olm/inbound_group_session.h

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*.c) \
    lib/crypto-algorithms/sha256.c \
    lib/crypto-algorithms/aes.c \
    lib/curve25519-donna/curve25519-donna.c

FUZZER_SOURCES := $(wildcard fuzzers/fuzz_*.cpp) $(wildcard fuzzers/fuzz_*.c)
TEST_SOURCES := $(wildcard tests/test_*.cpp) $(wildcard tests/test_*.c)

OBJECTS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
RELEASE_OBJECTS := $(addprefix $(BUILD_DIR)/release/,$(OBJECTS))
DEBUG_OBJECTS := $(addprefix $(BUILD_DIR)/debug/,$(OBJECTS))
FUZZER_OBJECTS := $(addprefix $(BUILD_DIR)/fuzzers/objects/,$(OBJECTS))
FUZZER_BINARIES := $(addprefix $(BUILD_DIR)/,$(basename $(FUZZER_SOURCES)))
FUZZER_DEBUG_BINARIES := $(patsubst $(BUILD_DIR)/fuzzers/fuzz_%,$(BUILD_DIR)/fuzzers/debug_%,$(FUZZER_BINARIES))
TEST_BINARIES := $(patsubst tests/%,$(BUILD_DIR)/tests/%,$(basename $(TEST_SOURCES)))
JS_OBJECTS := $(addprefix $(BUILD_DIR)/javascript/,$(OBJECTS))
JS_PRE := $(wildcard javascript/*pre.js)
JS_POST := javascript/olm_outbound_group_session.js \
    javascript/olm_inbound_group_session.js \
    javascript/olm_post.js
DOCS := tracing/README.html \
    docs/megolm.html \
    docs/olm.html \
    README.html \
    CHANGELOG.html

CPPFLAGS += -Iinclude -Ilib \
    -DOLMLIB_VERSION_MAJOR=$(MAJOR) -DOLMLIB_VERSION_MINOR=$(MINOR) \
    -DOLMLIB_VERSION_PATCH=$(PATCH)

# we rely on <stdint.h>, which was introduced in C99
CFLAGS += -Wall -Werror -std=c99 -fPIC
CXXFLAGS += -Wall -Werror -std=c++11 -fPIC
LDFLAGS += -Wall -Werror

EMCCFLAGS = --closure 1 --memory-init-file 0 -s NO_FILESYSTEM=1 -s INVOKE_RUN=0
# NO_BROWSER is kept for compatibility with emscripten 1.35.24, but is no
# longer needed.
EMCCFLAGS += -s NO_BROWSER=1

EMCC.c = $(EMCC) $(CFLAGS) $(CPPFLAGS) -c
EMCC.cc = $(EMCC) $(CXXFLAGS) $(CPPFLAGS) -c
EMCC_LINK = $(EMCC) $(LDFLAGS) $(EMCCFLAGS)

AFL.c = $(AFL_CC) $(CFLAGS) $(CPPFLAGS) -c
AFL.cc = $(AFL_CXX) $(CXXFLAGS) $(CPPFLAGS) -c
AFL_LINK.c = $(AFL_CC) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS)
AFL_LINK.cc = $(AFL_CXX) $(LDFLAGS) $(CXXFLAGS) $(CPPFLAGS)

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
$(TEST_BINARIES): LDFLAGS += $(DEBUG_OPTIMIZE_FLAGS) -L$(BUILD_DIR)

$(FUZZER_OBJECTS): CFLAGS += $(FUZZER_OPTIMIZE_FLAGS)
$(FUZZER_OBJECTS): CXXFLAGS += $(FUZZER_OPTIMIZE_FLAGS)
$(FUZZER_BINARIES): CPPFLAGS += -Ifuzzers/include
$(FUZZER_BINARIES): LDFLAGS += $(FUZZER_OPTIMIZE_FLAGS) -L$(BUILD_DIR)
$(FUZZER_DEBUG_BINARIES): CPPFLAGS += -Ifuzzers/include
$(FUZZER_DEBUG_BINARIES): LDFLAGS += $(DEBUG_OPTIMIZE_FLAGS)

$(JS_OBJECTS): CFLAGS += $(JS_OPTIMIZE_FLAGS)
$(JS_OBJECTS): CXXFLAGS += $(JS_OPTIMIZE_FLAGS)
$(JS_TARGET): LDFLAGS += $(JS_OPTIMIZE_FLAGS)

### top-level targets

lib: $(RELEASE_TARGET)
.PHONY: lib

$(RELEASE_TARGET): $(RELEASE_OBJECTS)
	$(CXX) $(LDFLAGS) --shared -fPIC \
            -Wl,-soname,libolm.so.$(MAJOR) \
            -Wl,--version-script,version_script.ver \
            $(OUTPUT_OPTION) $(RELEASE_OBJECTS)
	ln -sf libolm.so.$(VERSION) $(BUILD_DIR)/libolm.so.$(MAJOR)

debug: $(DEBUG_TARGET)
.PHONY: debug

$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	$(CXX) $(LDFLAGS) --shared -fPIC \
            -Wl,-soname,libolm_debug.so.$(MAJOR) \
            -Wl,--version-script,version_script.ver \
            $(OUTPUT_OPTION) $(DEBUG_OBJECTS)
	ln -sf libolm_debug.so.$(VERSION) $(BUILD_DIR)/libolm_debug.so.$(MAJOR)

js: $(JS_TARGET)
.PHONY: js

$(JS_TARGET): $(JS_OBJECTS) $(JS_PRE) $(JS_POST) $(JS_EXPORTED_FUNCTIONS)
	$(EMCC_LINK) \
               $(foreach f,$(JS_PRE),--pre-js $(f)) \
               $(foreach f,$(JS_POST),--post-js $(f)) \
               -s "EXPORTED_FUNCTIONS=@$(JS_EXPORTED_FUNCTIONS)" \
               $(JS_OBJECTS) -o $@

build_tests: $(TEST_BINARIES)

test: build_tests
	for i in $(TEST_BINARIES); do \
	    echo $$i; \
	    $$i || exit $$?; \
	done

fuzzers: $(FUZZER_BINARIES) $(FUZZER_DEBUG_BINARIES)
.PHONY: fuzzers

$(JS_EXPORTED_FUNCTIONS): $(PUBLIC_HEADERS)
	perl -MJSON -ne '$$f{"_$$1"}=1 if /(olm_[^( ]*)\(/; END { @f=sort keys %f; print encode_json \@f }' $^ > $@.tmp
	mv $@.tmp $@

all: test js lib debug doc
.PHONY: all

install-headers: $(PUBLIC_HEADERS)
	test -d $(DESTDIR)$(PREFIX)/include/olm || mkdir -p $(DESTDIR)$(PREFIX)/include/olm
	install -Dm644 $(PUBLIC_HEADERS) $(DESTDIR)$(PREFIX)/include/olm/
.PHONY: install-headers

install-debug: debug install-headers
	test -d $(DESTDIR)$(PREFIX)/lib || mkdir -p $(DESTDIR)$(PREFIX)/lib
	install -Dm755 $(DEBUG_TARGET) $(DESTDIR)$(PREFIX)/lib/libolm_debug.so.$(VERSION)
	ln -s libolm_debug.so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/libolm_debug.so.$(MAJOR)
	ln -s libolm_debug.so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/libolm_debug.so
.PHONY: install-debug

install: lib install-headers
	test -d $(DESTDIR)$(PREFIX)/lib || mkdir -p $(DESTDIR)$(PREFIX)/lib
	install -Dm755 $(RELEASE_TARGET) $(DESTDIR)$(PREFIX)/lib/libolm.so.$(VERSION)
	ln -s libolm.so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/libolm.so.$(MAJOR)
	ln -s libolm.so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/libolm.so
.PHONY: install

clean:;
	rm -rf $(BUILD_DIR) $(DOCS)
.PHONY: clean

doc: $(DOCS)
.PHONY: doc

### rules for building objects
$(BUILD_DIR)/release/%.o: %.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/release/%.o: %.cpp
	mkdir -p $(dir $@)
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/debug/%.o: %.c
	mkdir -p $(dir $@)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/debug/%.o: %.cpp
	mkdir -p $(dir $@)
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/javascript/%.o: %.c
	mkdir -p $(dir $@)
	$(EMCC.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/javascript/%.o: %.cpp
	mkdir -p $(dir $@)
	$(EMCC.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/tests/%: tests/%.c $(DEBUG_OBJECTS)
	mkdir -p $(dir $@)
	$(LINK.c) $< $(DEBUG_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/tests/%: tests/%.cpp $(DEBUG_OBJECTS)
	mkdir -p $(dir $@)
	$(LINK.cc) $< $(DEBUG_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/fuzzers/objects/%.o: %.c
	mkdir -p $(dir $@)
	$(AFL.c) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/fuzzers/objects/%.o: %.cpp
	mkdir -p $(dir $@)
	$(AFL.cc) $(OUTPUT_OPTION) $<

$(BUILD_DIR)/fuzzers/fuzz_%: fuzzers/fuzz_%.c $(FUZZER_OBJECTS)
	$(AFL_LINK.c) $< $(FUZZER_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/fuzzers/fuzz_%: fuzzers/fuzz_%.cpp $(FUZZER_OBJECTS)
	$(AFL_LINK.cc) $< $(FUZZER_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/fuzzers/debug_%: fuzzers/fuzz_%.c $(DEBUG_OBJECTS)
	$(LINK.c) $< $(DEBUG_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

$(BUILD_DIR)/fuzzers/debug_%: fuzzers/fuzz_%.cpp $(DEBUG_OBJECTS)
	$(LINK.cc) $< $(DEBUG_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@

%.html: %.rst
	rst2html $< $@

### dependencies

-include $(RELEASE_OBJECTS:.o=.d)
-include $(DEBUG_OBJECTS:.o=.d)
-include $(JS_OBJECTS:.o=.d)
-include $(TEST_BINARIES:=.d)
-include $(FUZZER_OBJECTS:.o=.d)
-include $(FUZZER_BINARIES:=.d)
-include $(FUZZER_DEBUG_BINARIES:=.d)

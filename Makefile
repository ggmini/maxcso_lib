SRCDIR := $(abspath $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))

USE_EXTERNAL_LIBDEFLATE ?= 0

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man

CC ?= gcc
CXX ?= g++

CFLAGS ?= -O2
CXXFLAGS ?= $(CFLAGS)

PKG_CONFIG ?= pkg-config

CFLAGS_UV = $(shell $(PKG_CONFIG) --cflags libuv)
LIBS_UV = $(shell $(PKG_CONFIG) --libs libuv)

CFLAGS_LZ4 = $(shell $(PKG_CONFIG) --cflags liblz4)
LIBS_LZ4 = $(shell $(PKG_CONFIG) --libs liblz4)

CFLAGS_ZLIB = $(shell $(PKG_CONFIG) --cflags zlib)
LIBS_ZLIB = $(shell $(PKG_CONFIG) --libs zlib)

DEP_FLAGS := $(CFLAGS_UV) $(CFLAGS_LZ4) $(CFLAGS_ZLIB)
LIBS := $(LIBS_UV) $(LIBS_LZ4) $(LIBS_ZLIB)

OBJDIR := obj
MKDIRS := $(OBJDIR)/src $(OBJDIR)/maxcsolib $(OBJDIR)/LibTest $(OBJDIR)/deps/zopfli/src/zopfli

SRC_CFLAGS += -W -Wall -Wextra -Wno-implicit-function-declaration -DNDEBUG=1
SRC_CXXFLAGS += -W -Wall -Wextra -std=c++11 -I$(SRCDIR)/deps/zopfli/src -I$(SRCDIR)/deps/7zip \
	-DNDEBUG=1 -I$(SRCDIR)/deps/libdeflate -Wno-unused-parameter -Wno-unused-variable \
	-pthread $(DEP_FLAGS)

SRC_CXX_SRC := $(wildcard $(SRCDIR)/src/*.cpp)
SRC_CXX_TMP := $(SRC_CXX_SRC:.cpp=.o)
SRC_CXX_OBJ := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SRC_CXX_TMP))

LIB_CXX_SRC := $(wildcard $(SRCDIR)/maxcsolib/*.cpp)
LIB_CXX_TMP := $(LIB_CXX_SRC:.cpp=.o)
LIB_CXX_OBJ := $(patsubst &(SRCDIR)/%,$(OBJDIR)/%,$(LIB_CXX_TMP))

TEST_CXX_SRC := $(wildcard $(SRCDIR)/LibTest/*.cpp)
TEST_CXX_TMP := $(TEST_CXX_SRC:.cpp=.o)
TEST_CXX_OBJ := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(TEST_CXX_TMP))

ZOPFLI_C_DIR := $(SRCDIR)/deps/zopfli/src/zopfli
ZOPFLI_C_SRC := $(ZOPFLI_C_DIR)/blocksplitter.c $(ZOPFLI_C_DIR)/cache.c \
               $(ZOPFLI_C_DIR)/deflate.c $(ZOPFLI_C_DIR)/gzip_container.c \
               $(ZOPFLI_C_DIR)/hash.c $(ZOPFLI_C_DIR)/katajainen.c \
               $(ZOPFLI_C_DIR)/lz77.c $(ZOPFLI_C_DIR)/squeeze.c \
               $(ZOPFLI_C_DIR)/tree.c $(ZOPFLI_C_DIR)/util.c \
               $(ZOPFLI_C_DIR)/zlib_container.c $(ZOPFLI_C_DIR)/zopfli_lib.c
ZOPFLI_C_TMP := $(ZOPFLI_C_SRC:.c=.o)
ZOPFLI_C_OBJ := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(ZOPFLI_C_TMP))

ifeq ($(OS),Windows_NT)
	LIBDEFLATE := libdeflatestatic.lib
	LIBS += -luuid
else
	LIBDEFLATE := libdeflate.a
endif

SRC_7ZIP := $(OBJDIR)/7zip/7zip.a
SRC_LIBDEFLATE := $(SRCDIR)/deps/libdeflate/$(LIBDEFLATE)

OBJS := $(SRC_CXX_OBJ) $(LIB_CXX_OBJ) $(TEST_CXX_OBJ) $(ZOPFLI_C_OBJ) $(SRC_7ZIP)

ifeq ($(USE_EXTERNAL_LIBDEFLATE), 0)
	OBJS += $(SRC_LIBDEFLATE)
else
	CFLAGS_LIBDEFLATE = $(shell $(PKG_CONFIG) --cflags libdeflate)
	LIBS_LIBDEFLATE = $(shell $(PKG_CONFIG) --libs libdeflate)
	SRC_CXXFLAGS += $(CFLAGS_LIBDEFLATE)
	LIBS += $(LIBS_LIBDEFLATE)
endif

.PHONY: all clean install uninstall

all: LibTesty

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(OBJDIR)/.done
	$(CXX) -c $(SRC_CXXFLAGS) $(CXXFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)/.done
	$(CC) -c $(SRC_CFLAGS) $(CFLAGS) -o $@ $<

LibTesty: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(SRC_CXXFLAGS) $(CXXFLAGS) $^ $(LIBS)

$(SRC_7ZIP):
	$(MAKE) -f $(SRCDIR)/deps/7zip/Makefile 7zip.a

$(SRC_LIBDEFLATE):
	$(MAKE) -C $(SRCDIR)/deps/libdeflate $(LIBDEFLATE)

$(OBJDIR)/.done:
	@mkdir -p $(MKDIRS)
	@touch $@

clean:
	rm -rf -- $(OBJDIR)
	rm -f LibTesty
	$(MAKE) -C $(SRCDIR)/deps/libdeflate clean

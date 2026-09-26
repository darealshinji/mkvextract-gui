BIN  = simple-mkvextract-gui

OBJS = \
	create_cmd.o \
	main.o \
	misc.o \
	mkvextract.o \
	pipe_command.o \
	quote_filename.o \
	rotate.o \
	thread_mkvinfo.o \
	thread_mkvextract.o \
	xml2ogm.o \
	$(NULL)

PKG_CONFIG ?= pkg-config
CXXFLAGS   ?= -Wall -O3 -std=gnu++20
LDFLAGS    ?= -Wl,--as-needed -Wl,--gc-sections -s
LIBS       ?=

# fltk
FLTK_CONFIG ?= fltk/build/usr/bin/fltk-config
ifeq ($(shell $(FLTK_CONFIG) --version 2>/dev/null), )
FLTK_CONFIG  = fltk-config
endif
CXXFLAGS    += $(shell $(FLTK_CONFIG) --cxxflags --use-images 2>/dev/null)
LDFLAGS     += $(shell $(FLTK_CONFIG) --ldstaticflags --use-images 2>/dev/null)
LIBS        += $(shell $(FLTK_CONFIG) --libs --use-images 2>/dev/null)

# tinyxml2
TINYXML2_LIBS = $(shell $(PKG_CONFIG) --libs tinyxml2 2>/dev/null)
ifeq ($(TINYXML2_LIBS), )
OBJS     += tinyxml2/tinyxml2.o
CXXFLAGS += -Itinyxml2
else
LIBS     += $(TINYXML2_LIBS)
endif

# fontconfig
LIBS += $(shell $(PKG_CONFIG) --libs fontconfig 2>/dev/null)


all: $(BIN)

clean:
	-rm -f $(BIN) $(OBJS)

distclean: clean
	-rm -rf fltk/build/

$(BIN): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)


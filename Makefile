BIN  = simple-mkvextract-gui
OBJS = create_extraction_command.o main.o mkvextract.o parsemkv.o pipe_command.o quote_filename.o rotate.o xml2ogm.o

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


create_extraction_command.cpp: mkvextract.hpp check_browser.hpp
main.cpp: mkvextract.hpp
mkvextract.cpp: mkvextract.hpp dnd.hpp check_browser.hpp pipe_command.hpp rotate.hpp
parsemkv.cpp: mkvextract.hpp pipe_command.hpp codecs.h
pipe_command.cpp: pipe_command.hpp
rotate.cpp: rotate.hpp
tinyxml2/tinyxml2.cpp: tinyxml2/tinyxml2.h


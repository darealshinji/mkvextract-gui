FLTK_CONFIG ?= ./fltk/build/usr/bin/fltk-config
PKG_CONFIG  ?= pkg-config
CXXFLAGS    := -Wall -O3 -std=c++20
CXXFLAGS    += $(shell $(FLTK_CONFIG) --use-images --cxxflags 2>/dev/null)
LDFLAGS     := -Wl,--as-needed -Wl,--gc-sections -s
LDFLAGS     += $(shell $(FLTK_CONFIG) --ldstaticflags --use-images 2>/dev/null)
LIBS        := $(shell $(FLTK_CONFIG) --libs --use-images 2>/dev/null)
LIBS        += $(shell $(PKG_CONFIG) --libs fontconfig 2>/dev/null)

BIN  = simple-mkvextract-gui
OBJS = main.o mkvextract.o parsemkv.o pipe_command.o rotate.o xml2ogm.o

TINYXML2_LIBS = $(shell $(PKG_CONFIG) --libs tinyxml2 2>/dev/null)

ifeq ($(TINYXML2_LIBS), )
OBJS     += tinyxml2/tinyxml2.o
CXXFLAGS += -Itinyxml2
else
LIBS += $(TINYXML2_LIBS)
endif



all: $(BIN)

clean:
	-rm -f $(BIN) $(OBJS)

$(BIN): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

main.cpp: mkvextract.hpp
mkvextract.cpp: mkvextract.hpp dnd.hpp check_browser.hpp pipe_command.hpp rotate.hpp
parsemkv.cpp: mkvextract.hpp pipe_command.hpp codecs.h
pipe_command.cpp: pipe_command.hpp
rotate.cpp: rotate.hpp
tinyxml2/tinyxml2.cpp: tinyxml2/tinyxml2.h
xml2ogm.cpp: mkvextract.hpp


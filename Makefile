FLTK_CONFIG := fltk-config
PKG_CONFIG  := pkg-config
CXXFLAGS    := -Wall -O2 -std=c++20
CXXFLAGS    += $(shell $(FLTK_CONFIG) --use-images --cxxflags)
LDFLAGS     := -Wl,--as-needed -s

LIBS  = $(shell $(FLTK_CONFIG) --ldstaticflags --libs --use-images)
LIBS += $(shell $(PKG_CONFIG) --libs fontconfig)

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


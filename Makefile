FLTK_CONFIG := fltk-config
PKG_CONFIG  := pkg-config
CXXFLAGS    := -Wall -O2 -std=c++20
CXXFLAGS    += $(shell $(FLTK_CONFIG) --use-images --cxxflags)
LDFLAGS     := -Wl,--as-needed -s

LIBS  = $(shell $(FLTK_CONFIG) --ldstaticflags --libs --use-images)
LIBS += $(shell $(PKG_CONFIG) --libs fontconfig)

BIN  = simple-mkvextract-gui
OBJS = main.o mkvextract.o parsemkv.o xml2ogm.o

TINYXML2_LIBS = $(shell $(PKG_CONFIG) --libs tinyxml2 2>/dev/null)

ifeq ($(TINYXML2_LIBS), )
OBJS += tinyxml2.o
else
LIBS += $(TINYXML2_LIBS)
endif


all: $(BIN)

clean:
	-rm -f $(BIN) $(OBJS)

$(BIN): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)


/***  compile this file with "fltk-config --use-images --compile mkvextract-gui.cpp"  ***/

#include "dnd.cpp"
#include "MyCheckBrowser.cpp"
#include "parsemkv.cpp"
#include "xml2ogm.cpp"

#ifdef HAS_TINYXML2
#include <tinyxml2.h>
#else
#include "tinyxml2.cpp"
#endif

#include "main.cpp"


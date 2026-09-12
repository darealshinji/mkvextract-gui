#include <FL/Fl.H>
#include <iostream>
#include <string.h>


int mkvextract(const char *in);


static void print_info()
{
  /* get and print FLTK version */
  const int version = Fl::api_version();
  const int major = version / 10000;
  const int minor = (version % 10000) / 100;
  const int patch = version % 100;

  std::cout << "https://github.com/darealshinji/mkvextract-gui\n"
    << "using FLTK " << major << "." << minor << "." << patch << " - http://fltk.org/"
    << std::endl;
}


int main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "--help") == 0) {
    std::cout << "usage: " << argv[0] << " [FILE]" << std::endl;
    return 0;
  }

  const char *file = argc > 1 ? argv[1] : NULL;

  print_info();

  return mkvextract(file);
}

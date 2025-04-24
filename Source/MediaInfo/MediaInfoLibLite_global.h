#pragma once

#ifndef MEDIAINFOLIBLITE_API
#  ifdef MEDIAINFOLIBLITE_BUILD_STATIC
#    define MEDIAINFOLIBLITE_API
#  else // !MEDIAINFOLIBLITE_BUILD_STATIC
#    ifdef _WIN32
#      ifdef MEDIAINFOLIBLITE_BUILD_LIBRARY
#        define MEDIAINFOLIBLITE_API __declspec(dllexport)
#      else // !MEDIAINFOLIBLITE_BUILD_LIBRARY
#        define MEDIAINFOLIBLITE_API __declspec(dllimport)
#      endif // MEDIAINFOLIBLITE_BUILD_LIBRARY
#    else // !_WIN32
#      define MEDIAINFOLIBLITE_API __attribute__((visibility("default")))
#    endif // _WIN32
#  endif // MEDIAINFOLIBLITE_BUILD_STATIC
#endif // MEDIAINFOLIBLITE_API

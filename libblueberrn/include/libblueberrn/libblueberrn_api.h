#ifndef LIBBLUEBERRN_API_H
#define LIBBLUEBERRN_API_H

#if defined(_MSC_VER) && !defined(LIBBLUEBERRN_STATIC)
    #ifdef LIBBLUEBERRN_EXPORTS
        #define LIBBLUEBERRN_API __declspec(dllexport)
    #else
        #define LIBBLUEBERRN_API __declspec(dllimport)
    #endif // LIBBLUEBERRN_EXPORTS
#else
    #define LIBBLUEBERRN_API
#endif // _MSC_VER

#endif // LIBMBGB_API_H
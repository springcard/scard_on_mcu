#ifndef __PROJECT_H__
#define __PROJECT_H__

/* Uncomment this line to enable debugging */
/* ---------------------------------------- */

//#define D(x) x

#ifndef D
  #define D(x)
#endif

/* Here you must define the types we'll be using in the library */
/* ------------------------------------------------------------ */

#ifdef WIN32
  /* Windows.h provides BYTE, WORD, DWORD etc */
  #include <Windows.h>
  /* SCARD_RC could be a signed 16-bit value */
  typedef signed short SCARD_RC;
#else
  /* On other platforms, we use stdint and define some aliases */
  #include <stdint.h>
  typedef uint32_t DWORD;
  typedef uint16_t WORD;
  typedef uint8_t BYTE;
  /* BOOL could be use another type if you want */
  typedef int8_t BOOL;
  /* Also we must define the BOOL values */
  #ifndef TRUE
    #define TRUE (BOOL)1
  #endif
  #ifndef FALSE
    #define FALSE (BOOL)0
  #endif  
  typedef int16_t SCARD_RC;
#endif

/* Other standard includes */
/* ----------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif

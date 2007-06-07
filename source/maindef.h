/****************************  maindef.h   **********************************
* Author:        Agner Fog
* Date created:  2006-08-26
* Last modified: 2006-08-26
* Project:       objconv
* Module:        maindef.h
* Description:
* Header file for type definitions and other main definitions.
*
* (c) 2006 GNU General Public License www.gnu.org/copyleft/gpl.html
*****************************************************************************/
#ifndef MAINDEF_H
#define MAINDEF_H

// Program version
#define OBJCONV_VERSION         0.95


// Integer type definitions with platform-independent sizes:
typedef char               int8;       // 8 bit  signed integer
typedef unsigned char      uint8;      // 8 bit  unsigned integer
typedef short int          int16;      // 16 bit signed integer
typedef unsigned short int uint16;     // 16 bit unsigned integer

// Definition of 32 bit integers depends on platform
#if defined(_WIN16) || defined(__MSDOS__) || defined(_MSDOS) // 16 bit systems
typedef long int           int32;      // 32 bit signed integer
typedef unsigned long int  uint32;     // 32 bit unsigned integer
#else
typedef int                int32;      // 32 bit signed integer
typedef unsigned int       uint32;     // 32 bit unsigned integer

// Definition of 64 bit integers depends on platform
#if defined(__WINDOWS__) && (defined(_MSC_VER) || defined(__INTEL_COMPILER))
typedef __int64            int64;      // 64 bit signed integer
typedef unsigned __int64   uint64;     // 64 bit unsigned integer
#else
typedef long long          int64;      // 64 bit signed integer
typedef unsigned long long uint64;     // 64 bit unsigned integer
#endif
#endif

// Get high part of a 64-bit integer
static inline uint32 HighDWord (uint64 x) {
   return (uint32)(x >> 32);
}

// Check if compiling for big-endian machine 
// (__BIG_ENDIAN__ may not be defined even on big endian systems, so this check is not 
// sufficient. A further check is done in CheckEndianness() in main.cpp)
#if defined(__BIG_ENDIAN__) && (__BIG_ENDIAN__ != 4321)
   #error This machine has big-endian memory organization. Objconv program will not work
#endif

// Max file name length
#define MAXFILENAMELENGTH        256


// File types 
#define FILETYPE_COFF              1         // Windows COFF/PE file
#define FILETYPE_OMF               2         // Windows OMF file
#define FILETYPE_ELF               3         // Linux or BSD ELF file
#define FILETYPE_MACHO_LE          4         // Mach-O file, little endian
#define FILETYPE_MACHO_BE          5         // Mach-O file, big endian
#define FILETYPE_DOS               6         // DOS file
#define FILETYPE_WIN3X             7         // Windows 3.x file
#define IMPORT_LIBRARY_MEMBER   0x10         // Member of import library, Windows
#define FILETYPE_MAC_UNIVBIN    0x11         // Macintosh universal binary
#define FILETYPE_MS_WPO         0x20         // Object file for whole program optimization, MS
#define FILETYPE_INTEL_WPO      0x21         // Object file for whole program optimization, Intel
#define FILETYPE_WIN_UNKNOWN    0x29         // Unknown subtype, Windows
#define FILETYPE_ASM           0x100         // Disassembly output
#define FILETYPE_LIBRARY      0x1000         // UNIX-style library/archive
#define FILETYPE_OMFLIBRARY   0x2000         // OMF-style library


// Define constants for symbol scope
#define S_LOCAL     0                        // Local symbol. Accessed only internally
#define S_PUBLIC    1                        // Public symbol. Visible from other modules
#define S_EXTERNAL  2                        // External symbol. Defined in another module


// Structures and functions used for lookup tables:

// Structure of integers and char *, used for tables of text strings
struct SIntTxt {
   int a;
   char * b;
};

// Translate integer value to text string by looking up in table of SIntTxt.
// Parameters: p = table, n = length of table, x = value to find in table
static inline char const * LookupText(SIntTxt const * p, int n, int x) {
   for (int i=0; i<n; i++, p++) {
      if (p->a == x) return p->b;
   }
   // Not found
   static char utext[32];
   sprintf(utext, "unknown(0x%X)", x);
   return utext;
}
// Macro to get length of text list and call LookupText
#define Lookup(list,x)  LookupText(list, sizeof(list)/sizeof(list[0]), x)

// Function to convert powers of 2 to index
int FloorLog2(uint32 x);

#endif // #ifndef MAINDEF_H

/*
 * version.h
 *
 *  Created on: 5 мая 2020 г.
 *      Author: sadko
 */

#ifndef LSP_PLUG_IN_TK_VERSION_H_
#define LSP_PLUG_IN_TK_VERSION_H_

#define LSP_TK_MAJOR                0
#define LSP_TK_MINOR                5
#define LSP_TK_MICRO                0

#ifdef LSP_TK_LIB_BUILTIN
    #define LSP_TK_LIB_EXPORT
    #define LSP_TK_LIB_CEXPORT
    #define LSP_TK_LIB_IMPORT           LSP_SYMBOL_IMPORT
    #define LSP_TK_LIB_CIMPORT          LSP_CSYMBOL_IMPORT
#else
    #define LSP_TK_LIB_EXPORT           LSP_SYMBOL_EXPORT
    #define LSP_TK_LIB_CEXPORT          LSP_CSYMBOL_EXPORT
    #define LSP_TK_LIB_IMPORT           LSP_SYMBOL_IMPORT
    #define LSP_TK_LIB_CIMPORT          LSP_CSYMBOL_IMPORT
#endif


#endif /* LSP_PLUG_IN_TK_VERSION_H_ */

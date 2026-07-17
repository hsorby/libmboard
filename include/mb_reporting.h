/* $Id: mb_reporting.h 1987 2009-07-22 08:57:36Z lsc $ */
/*!
 * \file mb_reporting.h
 * \code
 *      Author: Lee-Shawn Chin 
 *      Date  : June 2009
 *      Copyright (c) 2009 STFC Rutherford Appleton Laboratory
 * \endcode
 * 
 * \brief Macros to report erros, warnings, and info messages
 * 
 * This header provides the following routines:
 * - P_FUNCFAIL() : State reason (to stderr) when function returns an error
 * - P_WARNING()  : Print warning message (to stderr)
 * - P_INFO()     : Print warning message (to stdout)
 * 
 * These routines are only active when _EXTRA_INFO is defined
 */

/* splint does not support variadic macros. 
 * see -- http://www.splint.org/faq.html#quest18b 
 * We have no choice but to ask splint to ignore this file */
/*@ignore@*/ 

#ifndef MB_REPORTING_H_
#define MB_REPORTING_H_

#ifdef HAVE_CONFIG_H
#include "mb_config.h"
#endif

/* Bug in splint stops us for redefining special definitions like __func __
 * 
#ifndef HAVE_FUNC
#define __func__ "(unknown. __func__ identifier not provided by compiler)"
#endif
*/
#ifdef HAVE_FUNC
#define funcname __func__
#else
/* __func__ identifier not provided by compiler */
#define funcname "(unknown)"
#endif

#if defined(_PRINT_WARNINGS) || defined(_EXTRA_INFO)
extern int MBI_CommRank;
#endif

/* -------------------------------------------------------------------- */
/* if possible, use ISO C version of variadic macros */
#if defined(HAVE_C99_VARIADIC_MACROS)

#ifdef _EXTRA_INFO
#define P_INFO(...) \
    printf("[libmboard] INFO (P%d): ", MBI_CommRank);                         \
    printf(__VA_ARGS__);                                                      \
    printf("\n");
#else
#define P_INFO(...) 
#endif

#ifdef _PRINT_WARNINGS
#define P_FUNCFAIL(...) \
    fprintf(stderr, "[libmboard] WARNING (P%d): Function %s returned with an error\n", MBI_CommRank, funcname); \
    fprintf(stderr, "            Reason : ");                                 \
    fprintf(stderr, __VA_ARGS__);                                             \
    fprintf(stderr, "\n\n");
#define P_WARNING(...) \
    fprintf(stderr, "[libmboard] WARNING (P%d): ", MBI_CommRank);             \
    fprintf(stderr, __VA_ARGS__);                                             \
    fprintf(stderr, "\n\n");
#else
#define P_FUNCFAIL(...)  
#define P_WARNING(...) 
#endif

/* -------------------------------------------------------------------- */
/* next, try  GNU-style variadic macros */
#elif defined(HAVE_GNU_VARIADIC_MACROS)

#ifdef _EXTRA_INFO
#define P_INFO(format, args...) \
    printf("[libmboard] INFO (P%d): ", MBI_CommRank);                          \
    printf(format , ## args); /* space before comma is NECESSARY */            \
    printf("\n");
#else
#define P_INFO(format, args...)
#endif

#ifdef _PRINT_WARNINGS
#define P_FUNCFAIL(format, args...) \
    fprintf(stderr, "[libmboard] WARNING (P%d): Function %s returned with an error\n", MBI_CommRank, funcname); \
    fprintf(stderr, "            Reason : ");                                 \
    fprintf(stderr, format , ## args); /* space before comma is NECESSARY */  \
    fprintf(stderr, "\n\n");
#define P_WARNING(format, args...) \
    fprintf(stderr, "[libmboard] WARNING (P%d): ", MBI_CommRank);             \
    fprintf(stderr, format , ## args); /* space before comma is NECESSARY */  \
    fprintf(stderr, "\n\n");
#else
#define P_FUNCFAIL(format, args...)
#define P_WARNING(format, args...)
#endif

/* -------------------------------------------------------------------- */
/* if all fails, use a custom function that uses va_list */
#else

#include <stdio.h>
#include <stdarg.h>

/* inline keyword necessary to prevent "defined but not used" warnings */
/*@unused@*/ 
inline static void P_FUNCFAIL(const char *fmt, ...) {
#ifdef _PRINT_WARNINGS
    va_list args;
    fprintf(stderr, "[libmboard] WARNING (P%d): Function %s returned with an error\n", MBI_CommRank, funcname); 
    fprintf(stderr, "            Reason : ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n\n");
#else
    (void)fmt;  /* suppress unused warning */
#endif
}

/* inline keyword necessary to prevent "defined but not used" warnings */
/*@unused@*/ 
inline static void P_WARNING(const char *fmt, ...) {
#ifdef _PRINT_WARNINGS
    va_list args;
    fprintf(stderr, "[libmboard] WARNING (P%d): ", MBI_CommRank);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n\n");
#else
    (void)fmt;  /* suppress unused warning */
#endif
}

/* inline keyword necessary to prevent "defined but not used" warnings */
/*@unused@*/ 
inline static void P_INFO(const char *fmt, ...) {
#ifdef _EXTRA_INFO
    va_list args;
    printf("[libmboard] INFO (P%d): ", MBI_CommRank);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
#else
    (void)fmt;  /* suppress unused warning */
#endif
}

#endif /* HAVE_C99_VARIADIC_MACROS || HAVE_GNU_VARIADIC_MACROS || neither */

#endif /*MB_REPORTING_H_*/

/*@end@*/

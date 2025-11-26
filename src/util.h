/**
 * this implementation is picked from dependencies `libco`
 * is used to mask thread_local for runner variable
 * set of macro if-else statement for `thread_local`
 *
 * take it as magic (work for clang and gcc, on msvc could give throuble)
 * */

#ifndef UTIL_H
#define UTIL_H

/* Running in multi-threaded environment */
#if !defined(thread_local) /* override thread_local for obscure compilers */
#if defined(__STDC__)      /* Compiling as C Language */
#if defined(_MSC_VER)      /* Don't rely on MSVC's C11 support */
#define thread_local __declspec(thread)
#elif __STDC_VERSION__ < 201112L            /* If we are on C90/99 */
#if defined(__clang__) || defined(__GNUC__) /* Clang and GCC */
#define thread_local __thread
#else /* Otherwise, we ignore the directive (unless provided own) */
#define thread_local
#endif
#else /* C11 and newer define thread_local in threads.h */
#include <threads.h>
#endif
#elif defined(__cplusplus) /* Compiling as C++ Language */
#if __cplusplus < 201103L  /* thread_local is a C++11 feature */
#if defined(_MSC_VER)
#define thread_local __declspec(thread)
#elif defined(__clang__) || defined(__GNUC__)
#define thread_local __thread
#else /* Otherwise, we ignore the directive (unless provided own) */
#define thread_local
#endif
#else /* In C++ >= 11, thread_local in a builtin keyword */
/* Don't do anything */
#endif
#endif
#endif

#endif /* end of include guard: UTIL_H */

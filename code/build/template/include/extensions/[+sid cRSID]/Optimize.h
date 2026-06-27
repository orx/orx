//cRSID stuff for fine-tuning compile-time optimizations (branch-prediction aid, etc.)
//(GCC/TCC oriented, might need to edit for MSVC and others)

#ifndef LIBCRSID_HEADER__OPTIMIZE
#define LIBCRSID_HEADER__OPTIMIZE


// #define INLINE  inline  __attribute__((__always_inline__)) //__attribute__((always_inline))  //force the compiler as possible to inline thos fricking inlines
//#define INLINE  inline    //milder inline hints
// #define INLINE            //comment out this line to set empty if not preferred to have any inlining
#define INLINE  orxINLINE  //use orx's platform independant inlining


//inform the compiler about variables to be made as register/static instead of slow stack/mainmemory based
// #define FASTVAR  register   //be careful not to use it too much in a function, not to run out from registers (amount is arhitecture-dependent)
//#define FASTVAR  static   //static might still be better than stack-based/automatic local variables (but be careful about initial values set only once!)
#define FASTVAR          //comment out this line to set empty if not preferred to have any variable-hints


// #define FASTPTR  restrict
#define FASTPTR          //comment out this line to set empty if not preferred to have any pointer-hints


//macros for aiding compiler branch-prediction optimizations  (with if clauses, and probably with ternary '?' too)
//(There's also [[likely]] and [[unlikely]] supported by MSVC, but not by TCC.)
#define PREDICATES 0 //set to 0 if you rely on compiler instead (and modern processor's heuristic runtime branch-prediction)

#if (PREDICATES != 0)

#define UNLIKELY(expression)  __builtin_expect(!!(expression), 0) //condition is unlikely to happen most of the times
#define RARELY(expression)    __builtin_expect(!!(expression), 0) //rarer occurrences of the condition being true
#define CALMLY(expression)    __builtin_expect(!!(expression), 0) //not time-critical (calmly/loosely), prefer the other more critical path

#define LIKELY(expression)    __builtin_expect(!!(expression), 1) //condition is likely to be true most of the times
#define MOSTLY(expression)    __builtin_expect(!!(expression), 1) //more often/frequently/typically/mainly true than false
#define TIGHTLY(expression)   __builtin_expect(!!(expression), 1) //more time-critical tense/tight path, so prefer optimizing it even if it's rarer or more unlikely

//#define VERY_LIKELY(expression)    __builtin_expect_with_probability(!!(expression), 1, 0.999) //some docs claim this is a more agressive hint
//#define VERY_UNLIKELY(expression)  __builtin_expect_with_probability(!!(expression), 0, 0.999) //some docs claim this is a more agressive hint

#else //if (PREDICATES == 0)

#define UNLIKELY(expression)  expression
#define RARELY(expression)  expression
#define CALMLY(expression)  expression

#define LIKELY(expression)  expression
#define MOSTLY(expression)  expression
#define TIGHTLY(expression)  expression

//#define VERY_LIKELY(expression) expression
//#define VERY_UNLIKELY(expression) expression

#endif


#endif //LIBCRSID_HEADER__OPTIMIZE

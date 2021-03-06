/******************************************************************************
**
**               Nilpotent Quotient for Lie Algebras
** coeff_long.h
** defines coefficients as signed 64-bit integers
*/

/* COEFF_UNSAFE turns off coefficient overflow detection. It's generally a bad idea to
   disable it, but it's only present in GCC >= 5 */
#if !defined(__GNUC__) || __GNUC__ <= 4
#define COEFF_UNSAFE
#endif

#define COEFF_ID "Z as int64_t"

struct coeff {
  int64_t data;
};

__attribute__((unused)) static int coeff_overflow(const char *name) {
  volatile int zero = 0;
  fprintf(stderr, "%s: integer overflow\n", name);
  return zero / zero;
}
  
/* addition, it seems that GCC does not optimize as well if(coeff_sgn(x)) as if(coeff_nz_p(x)) */
inline bool coeff_nz_p(const coeff &a) {
  return a.data != 0;
}

inline bool coeff_z_p(const coeff &a) {
  return a.data == 0;
}

inline void coeff_init(coeff &a) {
}

inline void coeff_init_set(coeff &result, const coeff &a) {
  result.data = a.data;
}

inline void coeff_init_set_si(coeff &result, const long a) {
  result.data = a;
}

inline void coeff_set(coeff &result, const coeff &a) {
  result.data = a.data;
}

inline void coeff_set_si(coeff &result, const long a) {
  result.data = a;
}

inline long coeff_get_si(const coeff &a) {
  return a.data;
}

inline void coeff_clear(coeff &a) {
}

inline void coeff_add(coeff &result, const coeff &a, const coeff &b) {
#ifdef COEFF_UNSAFE
  result.data = a.data + b.data;
#else
  if (__builtin_expect(__builtin_saddll_overflow(a.data, b.data, &result.data), false))
    coeff_overflow("coeff_add");
#endif
}

inline void coeff_add_si(coeff &result, const coeff &a, long b) {
#ifdef COEFF_UNSAFE
  result.data = a.data + b;
#else
  if (__builtin_expect(__builtin_saddll_overflow(a.data, b, &result.data), false))
    coeff_overflow("coeff_add_si");
#endif
}

inline void coeff_addmul(coeff &result, const coeff &a, const coeff &b) {
#ifdef COEFF_UNSAFE
  result.data += a.data * b.data;
#else
  int64_t m;
  if (__builtin_expect(__builtin_smulll_overflow(a.data, b.data, &m) || __builtin_saddll_overflow(result.data, m, &result.data), false))
    coeff_overflow("coeff_addmul");
#endif
}

inline int coeff_cmp(const coeff &a, const coeff &b) {
  return (a.data > b.data) - (a.data < b.data);
}

inline int coeff_cmp_si(const coeff &a, long b) {
  return (a.data > b) - (a.data < b);
}

inline void coeff_divexact(coeff &result, const coeff &a, const coeff &b) {
  result.data = a.data / b.data;
}

inline void coeff_fdiv_q(coeff &result, const coeff &a, const coeff &b) {
  result.data = a.data / b.data;
  if (a.data % b.data < 0)
    result.data--; /* C rounds quotient to 0, not down */
}

inline void coeff_fdiv_r(coeff &result, const coeff &a, const coeff &b) {
  result.data = a.data % b.data;
  if (result.data < 0)
    result.data += b.data; /* C rounds quotient to 0, not down */
}

inline void coeff_mul(coeff &result, const coeff &a, const coeff &b) {
#ifdef COEFF_UNSAFE
  result.data = a.data * b.data;
#else
  if (__builtin_expect(__builtin_smulll_overflow(a.data, b.data, &result.data), false))
    coeff_overflow("coeff_mul");
#endif
}

inline void coeff_mul_si(coeff &result, const coeff &a, long b) {
#ifdef COEFF_UNSAFE
  result.data = a.data * b;
#else
  if (__builtin_expect(__builtin_smulll_overflow(a.data, b, &result.data), false))
    coeff_overflow("coeff_mul_si");
#endif 
}

inline void coeff_neg(coeff &result, const coeff &a) {
#ifdef COEFF_UNSAFE
  result.data = -a.data;
#else
  if (__builtin_expect(__builtin_ssubll_overflow(0, a.data, &result.data), false))
    coeff_overflow("coeff_neg");
#endif
}

inline int coeff_sgn(const coeff &a) {
  return (a.data > 0) - (a.data < 0);
}

inline void coeff_sub(coeff &result, const coeff &a, const coeff &b) {
#ifdef COEFF_UNSAFE
  result.data = a.data - b.data;
#else
  if (__builtin_expect(__builtin_ssubll_overflow(a.data, b.data, &result.data), false))
    coeff_overflow("coeff_sub");
#endif
}

inline void coeff_submul(coeff &result, const coeff &a, const coeff &b) {
#ifdef COEFF_UNSAFE
  result.data -= a.data * b.data;
#else
  int64_t m;
  if (__builtin_expect(__builtin_smulll_overflow(a.data, b.data, &m) || __builtin_ssubll_overflow(result.data, m, &result.data), false))
    coeff_overflow("coeff_submul");
#endif
}

/* addition, unused */
inline void coeff_swap(coeff &a, coeff &b, coeff &tmp) {
  coeff_set(tmp, a);
  coeff_set(a, b);
  coeff_set(b, tmp);
}

/* this code works only if b>0 */
inline void coeff_gcdext(coeff &gcd, coeff &s, coeff &t, const coeff &a, const coeff &b) {
  coeff new_s, new_t, new_gcd, q, tmp;
  coeff_init(q);
  coeff_init(tmp);
  coeff_init_set_si(new_s, 0); coeff_set_si(s, 1);
  coeff_init_set_si(new_t, 1); coeff_set_si(t, 0);
  coeff_init_set(new_gcd, b); coeff_set(gcd, a);
  while (coeff_sgn(new_gcd)) {
    coeff_fdiv_q(q, gcd, new_gcd);
    coeff_submul(s, q, new_s); coeff_swap(s, new_s, tmp);
    coeff_submul(t, q, new_t); coeff_swap(t, new_t, tmp);
    coeff_submul(gcd, q, new_gcd); coeff_swap(gcd, new_gcd, tmp);
  }
  coeff_clear(tmp);
  coeff_clear(new_s);
  coeff_clear(new_t);
  coeff_clear(new_gcd);
  coeff_clear(q);
}

/* addition, returns true if a in [0,b) or b=0 */
inline bool coeff_reduced_p(coeff &a, coeff &b) {
  return b.data == 0 || (a.data >= 0 && a.data < b.data);
}

/* addition, returns unit and generator of annihilator ideal:
   a*unit is canonical (2^n) and a*annihilator=0 */
inline void coeff_unit_annihilator(coeff &unit, coeff &annihilator, const coeff &a) {
  unit.data = coeff_sgn(a);
  annihilator.data = 0;
}

inline int coeff_out_str(FILE *f, const coeff &a)
{
  return fprintf(f, "%ld", (long) a.data);
}

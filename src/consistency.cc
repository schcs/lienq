/******************************************************************************
**
**                 Nilpotent Quotient for Lie Rings
** consistency.c                                                Csaba Schneider
**                                                           schcs@math.klte.hu
*/

#include "lienq.h"
#include <algorithm>

/*
**  The goal of this file is to check the consistency relations in the
**  factorring.
*/

/*
**  CheckJacobi( a, b, c ) checks whether the Jacobi holds for
**  the triple ( a, b, c ) and returns its value in cv.
*/

/* v += [[a,b],c] */
void TripleProduct(gpvec &v, gen a, gen b, gen c) {
  gpvec temp[2];
  temp[0] = FreshVec();
  temp[1] = FreshVec();
  bool parity = false;
  
  if (a < b) {
    for (gpvec p = Product[b][a]; p->g != EOW && p->g <= NrPcGens; p++)
      if (p->g > c)
	Diff(temp[!parity], temp[parity], p->c, Product[p->g][c]), parity ^= 1;
      else if (p->g < c)
	Sum(temp[!parity], temp[parity], p->c, Product[c][p->g]), parity ^= 1;
  } else {
    for (gpvec p = Product[a][b]; p->g != EOW && p->g <= NrPcGens; p++)
      if (p->g > c)
	Sum(temp[!parity], temp[parity], p->c, Product[p->g][c]), parity ^= 1;
      else if (p->g < c)
	Diff(temp[!parity], temp[parity], p->c, Product[c][p->g]), parity ^= 1;
  }
#if 0
  if (parity)
    PopVec(v), PopVec();
  else
    PopVec(), PopVec(v);
#else
  Copy(v, temp[parity]);
  PopVec();
  PopVec();
#endif
}

/*
**  The relations to be enforced are of form
**  [ a, b, c ] + [ b, c, a ] + [ c, a, b ]    where <a> is of weight 1
**  and  <a> < <b> < <c>  with respect to the linear ordering of the
**  generators.
*/
static void CheckJacobi(gen a, gen b, gen c) {
  if (Weight[a] + Weight[b] + Weight[c] > Class)
    return;

  gpvec temp1 = FreshVec();
  gpvec temp2 = FreshVec();
  gpvec temp3 = FreshVec();
  TripleProduct(temp1, a, b, c); // temp1 = [a,b,c]
  TripleProduct(temp2, b, c, a); // temp2 = [b,c,a]
  Sum(temp3, temp1, temp2); // temp3 = [a,b,c] + [b,c,a]
  TripleProduct(temp1, c, a, b);
  Sum(temp2, temp1, temp3); // temp2 = Jacobi(a,b,c)
  Collect(temp1, temp2);

  if (Debug >= 2) {
    fprintf(OutputFile, "# consistency: [a%d,a%d,a%d] = ", a, b, c);
    PrintVec(temp1);
    fprintf(OutputFile, "\n");
  }

  AddRow(temp1);
  
  PopVec();
  PopVec();
  PopVec();
}

/*
**  The following function checks the consistency relation for
**  o_i[ a, b ] = [ (( o_ia )), b ] where (()) means the substitution
**  of the argument with its relation.
**
*/
static void CheckPower(gen a, gen b) {
  gpvec temp[2];
  temp[0] = FreshVec();
  temp[1] = FreshVec();
  bool parity = false;
  
  for (gpvec p = Power[a]; p->g <= NrPcGens && p->g != EOW; p++) {
    gen g = p->g;
    if (g > b)
      Diff(temp[!parity], temp[parity], p->c, Product[g][b]), parity ^= 1;
    else if (g < b)
      Sum(temp[!parity], temp[parity], p->c, Product[b][g]), parity ^= 1;
  }

  if (a > b)
    Sum(temp[!parity], temp[parity], Exponent[a], Product[a][b]), parity ^= 1;
  else if (a < b)
    Diff(temp[!parity], temp[parity], Exponent[a], Product[b][a]), parity ^= 1;
  Collect(temp[!parity], temp[parity]), parity ^= 1;

  if (Debug >= 2) {
    fprintf(OutputFile, "# consistency: [%ld*a%d,a%d] = ", coeff_get_si(Exponent[a]), a, b);
    PrintVec(temp[parity]);
    fprintf(OutputFile, "\n");
  }

  AddRow(temp[parity]);
  
  PopVec();
  PopVec();
}

/* if N*v = 0 in our ring, and we have a power relation A*g = w,
 * enforce (N/A)*w = 0
 */
static void CheckTorsion(unsigned i) {
  gpvec temp1 = FreshVec(), temp2 = FreshVec();
  coeff annihilator, unit;
  coeff_init(annihilator);
  coeff_init(unit); // unused
  
  coeff_unit_annihilator(unit, annihilator, Exponent[i]);
  Prod(temp1, annihilator, Power[i]);
  Collect(temp2, temp1);
  
  if (Debug >= 2) {
    fprintf(OutputFile, "# consistency: %ld*%ld*a%d = ", coeff_get_si(annihilator), coeff_get_si(Exponent[i]), i);
    PrintVec(temp2);
    fprintf(OutputFile, "\n");
  }

  AddRow(temp2);

  coeff_clear(unit);
  coeff_clear(annihilator);
  PopVec();
  PopVec();
}

void Consistency(void) {
  for (unsigned i = 1; i <= LastGen[1]; i++)
    for (unsigned j = i + 1; j <= NrPcGens; j++)
      for (unsigned k = j + 1; k <= NrPcGens; k++)
        CheckJacobi(i, j, k);

  for (unsigned i = 1; i <= NrPcGens; i++)
    if (coeff_nz_p(Exponent[i])) {
      CheckTorsion(i);

      for (unsigned j = 1; j <= NrPcGens; j++)
	CheckPower(i, j);
    }
  
  TimeStamp("Consistency()");
}

void GradedConsistency(void) {
  for (unsigned a = 1; a <= LastGen[1]; a++) /* a is of weight 1 */
    for (unsigned bw = 1; bw <= Class / 2; bw++) { /* bw is the weight of b */
      unsigned cw = Class - 1 - bw; /* the weight of c */
      for (unsigned b = std::max(a + 1, LastGen[bw-1] + 1); b <= LastGen[bw]; b++)
        for (unsigned c = std::max(b + 1, LastGen[cw-1]); c <= LastGen[cw]; c++)
	  CheckJacobi(a, b, c);
    }

  for (unsigned aw = 1; aw <= Class - 1; aw++) { /* the weight of a */
    unsigned bw = Class - aw; /* that of b */
    for (unsigned a = LastGen[aw-1] + 1; a <= LastGen[aw]; a++)
      if (coeff_nz_p(Exponent[a])) {
	CheckTorsion(a);
	
        for (unsigned b = LastGen[bw-1] + 1; b <= LastGen[bw]; b++)
	  CheckPower(a, b);
      }
  }

  TimeStamp("GradedConsistency()");
}
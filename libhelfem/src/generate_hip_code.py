#
#                This source code is part of
#
#                          HelFEM
#                             -
# Finite element methods for electronic structure calculations on small systems
#
# Written by Susi Lehtola, 2018-
# Copyright (c) 2018- Susi Lehtola
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# Code to generate sources for arbitrary order derivatives of LIP
# basis functions.

import copy

maxorder=10

print('''/* This file is autogenerated with generate_lip_code.py */
#include "HIPBasis.h"
#include <cfloat>

namespace helfem {
  namespace polynomial_basis {
''')

def get_fname(order):
    fname = 'f'
    if order == 1:
        fname = 'df'
    if order > 1:
        fname = 'd{}f'.format(order)
    return fname

print('void HIPBasis::eval_prim_dnf(const arma::vec & x, arma::mat & dnf, int n, double element_length) const {')
print('switch(n) {')

# Use recursion to find out derivative of Li^2(x)
Lider = []
# Zeroth derivative
Lider.append({ (0,0) : 1 })
for order in range(maxorder):
    # Last step
    prev = copy.deepcopy(Lider[-1])
    cur = {}
    for fun in prev:
        # Apply derivative
        ders = [(fun[0]+1,fun[1]), (fun[0],fun[1]+1)]
        for term in ders:
            # Order the term and add
            oterm = (max(term), min(term))
            if oterm in cur:
                cur[oterm] += prev[fun]
            else:
                cur[oterm] = prev[fun]
    Lider.append(cur)

for order in range(0,maxorder):
    fname='dnf'
    print('case({}):'.format(order))
    print('{')

    print('// Allocate memory\n{}.zeros(x.n_elem, 2*x0.n_elem);\n'.format(fname))

    print('// Evaluate LIP basis data\ndouble dummy_length=1.0;')
    for olip in range(order+1):
        print('arma::mat {0}lip;\nLIPBasis::eval_prim_dnf(x, {0}lip, {1}, dummy_length);'.format(get_fname(olip), olip))

    print('// Loop over points\nfor(size_t ix=0; ix<x.n_elem; ix++) {')
    print('// Loop over polynomials\nfor(size_t fi=0; fi<x0.n_elem; fi++) {')
    print('''/* First function is [1 - 2(x-xi)*lipxi(fi)] [l_i(x)]^2 = f1 * f2.
   Second function is (x-xi) * [l_i(x)]^2 = f3 * f2
*/''')

    print('double f1 = 1.0 - 2.0*(x(ix)-x0(fi))*lipxi(fi);')
    if order>0:
        print('double df1 = -2.0*lipxi(fi);')
    print('')

    print('double f3 = x(ix)-x0(fi);')
    if order>0:
        print('double df3 = 1;')
    print('')

    def print_term(label, Li):
        print('double {} = '.format(label), end='')
        for term in Li:
            if Li[term]!=1:
                print('{:+d} *'.format(Li[term]), end='')
            print('{}lip(ix,fi)*{}lip(ix,fi)'.format(get_fname(term[0]), get_fname(term[1]), end=''))
        print(';')

    if order:
        print_term('{}2'.format(get_fname(order-1)), Lider[order-1])
    print_term('{}2'.format(get_fname(order)), Lider[order])

    # Collect terms
    if order==0:
        print('{0}(ix,2*fi) = f1*f2;'.format(fname))
        print('{0}(ix,2*fi+1) = f3*f2*element_length;'.format(fname))
    else:
        print('{0}(ix,2*fi) = {1}*{2}1*{3}2 + {4}1*{5}2;'.format(fname, order, get_fname(1), get_fname(order-1), get_fname(0), get_fname(order)))
        print('{0}(ix,2*fi+1) = ({1}*{2}3*{3}2 + {4}3*{5}2)*element_length;'.format(fname, order, get_fname(1), get_fname(order-1), get_fname(0), get_fname(order)))

    # End loops
    print('}\n}')
    # End function
    print('}')
    print('break;')
print('default:\nstd::ostringstream oss;\noss << n << "th derivatives not implemented!\\n";\nthrow std::logic_error(oss.str());')
print('}\n}')

# End header
print('''}
}
''')

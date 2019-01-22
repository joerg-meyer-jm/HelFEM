/*
 *                This source code is part of
 *
 *                          HelFEM
 *                             -
 * Finite element methods for electronic structure calculations on small systems
 *
 * Written by Susi Lehtola, 2018-
 * Copyright (c) 2018- Susi Lehtola
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include "quadrature.h"
#include "../general/chebyshev.h"
#include "../general/polynomial.h"
#include "../general/gsz.h"

namespace helfem {
  namespace quadrature {
    arma::mat radial_integral(double rmin, double rmax, int n, const arma::vec & x, const arma::vec & wx, const arma::mat & bf) {
#ifndef ARMA_NO_DEBUG
      if(x.n_elem != wx.n_elem) {
        std::ostringstream oss;
        oss << "x and wx not compatible: " << x.n_elem << " vs " << wx.n_elem << "!\n";
        throw std::logic_error(oss.str());
      }
      if(x.n_elem != bf.n_rows) {
        std::ostringstream oss;
        oss << "x and bf not compatible: " << x.n_elem << " vs " << bf.n_rows << "!\n";
        throw std::logic_error(oss.str());
      }
#endif

      // Midpoint is at
      double rmid(0.5*(rmax+rmin));
      // and half-length of interval is
      double rlen(0.5*(rmax-rmin));
      // r values are then
      arma::vec r(rmid*arma::ones<arma::vec>(x.n_elem)+rlen*x);

      // Calculate total weight per point
      arma::vec wp(wx*rlen);
      if(n!=0)
        wp%=arma::pow(r,n);

      // Put in weight
      arma::mat wbf(bf);
      for(size_t i=0;i<bf.n_cols;i++)
	wbf.col(i)%=wp;

      // Matrix elements are then
      return arma::trans(wbf)*bf;
    }

    arma::mat derivative_integral(double rmin, double rmax, const arma::vec & x, const arma::vec & wx, const arma::mat & dbf) {
#ifndef ARMA_NO_DEBUG
      if(x.n_elem != wx.n_elem) {
        std::ostringstream oss;
        oss << "x and wx not compatible: " << x.n_elem << " vs " << wx.n_elem << "!\n";
        throw std::logic_error(oss.str());
      }
      if(x.n_elem != dbf.n_rows) {
        std::ostringstream oss;
        oss << "x and dbf not compatible: " << x.n_elem << " vs " << dbf.n_rows << "!\n";
        throw std::logic_error(oss.str());
      }
#endif

      // Half-length of interval is
      double rlen(0.5*(rmax-rmin));

      // Put in weight
      arma::mat wdbf(dbf);
      for(size_t i=0;i<dbf.n_cols;i++)
	// We get +1 rlen from the jacobian, but -2 from the derivatives
	wdbf.col(i)%=wx/rlen;

      // Integral is
      return arma::trans(wdbf)*dbf;
    }

    arma::mat gsz_integral(double Z, double dz, double Hz, double rmin, double rmax, const arma::vec & x, const arma::vec & wx, const arma::mat & bf) {
#ifndef ARMA_NO_DEBUG
      if(x.n_elem != wx.n_elem) {
        std::ostringstream oss;
        oss << "x and wx not compatible: " << x.n_elem << " vs " << wx.n_elem << "!\n";
        throw std::logic_error(oss.str());
      }
      if(x.n_elem != bf.n_rows) {
        std::ostringstream oss;
        oss << "x and bf not compatible: " << x.n_elem << " vs " << bf.n_rows << "!\n";
        throw std::logic_error(oss.str());
      }
#endif

      // Midpoint is at
      double rmid(0.5*(rmax+rmin));
      // and half-length of interval is
      double rlen(0.5*(rmax-rmin));
      // r values are then
      arma::vec r(rmid*arma::ones<arma::vec>(x.n_elem)+rlen*x);

      // Calculate total weight per point
      arma::vec wp(wx*rlen);
      // Plug in charge
      wp%=-GSZ::Z_GSZ(r,Z,dz,Hz)/r;

      // Put in weight
      arma::mat wbf(bf);
      for(size_t i=0;i<bf.n_cols;i++)
	wbf.col(i)%=wp;

      // Matrix elements are then
      return arma::trans(wbf)*bf;
    }

    arma::mat sap_integral(const ::SAP & sap, int Z, double rmin, double rmax, const arma::vec & x, const arma::vec & wx, const arma::mat & bf) {
#ifndef ARMA_NO_DEBUG
      if(x.n_elem != wx.n_elem) {
        std::ostringstream oss;
        oss << "x and wx not compatible: " << x.n_elem << " vs " << wx.n_elem << "!\n";
        throw std::logic_error(oss.str());
      }
      if(x.n_elem != bf.n_rows) {
        std::ostringstream oss;
        oss << "x and bf not compatible: " << x.n_elem << " vs " << bf.n_rows << "!\n";
        throw std::logic_error(oss.str());
      }
#endif

      // Midpoint is at
      double rmid(0.5*(rmax+rmin));
      // and half-length of interval is
      double rlen(0.5*(rmax-rmin));
      // r values are then
      arma::vec r(rmid*arma::ones<arma::vec>(x.n_elem)+rlen*x);

      // Calculate total weight per point
      arma::vec wp(wx*rlen);
      // Plug in charge
      wp%=sap.get(Z,r);

      // Put in weight
      arma::mat wbf(bf);
      for(size_t i=0;i<bf.n_cols;i++)
	wbf.col(i)%=wp;

      // Matrix elements are then
      return arma::trans(wbf)*bf;
    }

    static arma::vec twoe_inner_integral_wrk(double rmin, double rmax, double rmin0, double rmax0, const arma::vec & x, const arma::vec & wx, const polynomial_basis::PolynomialBasis * poly, int L) {
      // Midpoint is at
      double rmid(0.5*(rmax+rmin));
      // and half-length of interval is
      double rlen(0.5*(rmax-rmin));
      // r values are then
      arma::vec r(rmid*arma::ones<arma::vec>(x.n_elem)+rlen*x);

      // Midpoint of original interval is at
      double rmid0(0.5*(rmax0+rmin0));
      // and half-length of original interval is
      double rlen0(0.5*(rmax0-rmin0));

      // Calculate total weight per point
      arma::vec wp((wx%arma::pow(r,L))*rlen);

      // Calculate x values the polynomials should be evaluated at
      arma::vec xpoly((r-rmid0*arma::ones<arma::vec>(x.n_elem))/rlen0);
      // Evaluate the polynomials at these points
      arma::mat bf(poly->eval(xpoly));

      // Put in weight
      arma::mat wbf(bf);
      for(size_t i=0;i<wbf.n_cols;i++)
        wbf.col(i)%=wp;

      // The integrals are then
      arma::vec inner(arma::vectorise(arma::trans(wbf)*bf));

      return inner;
    }

    arma::mat twoe_inner_integral(double rmin, double rmax, const arma::vec & x, const arma::vec & wx, const polynomial_basis::PolynomialBasis * poly, int L) {
      // Midpoint is at
      double rmid(0.5*(rmax+rmin));
      // and half-length of interval is
      double rlen(0.5*(rmax-rmin));
      // r values are then
      arma::vec r(rmid*arma::ones<arma::vec>(x.n_elem)+rlen*x);

      // Compute the "inner" integrals as function of r.
      arma::mat inner(x.n_elem,std::pow(poly->get_nbf(),2));
      inner.row(0)=arma::trans(twoe_inner_integral_wrk(rmin, r(0), rmin, rmax, x, wx, poly, L));
      // Every subinterval uses a fresh nquad points!
      for(size_t ip=1;ip<x.n_elem;ip++)
        inner.row(ip)=inner.row(ip-1)+arma::trans(twoe_inner_integral_wrk(r(ip-1), r(ip), rmin, rmax, x, wx, poly, L));

      // Put in the 1/r^(L+1) factors now that the integrals have been computed
      arma::vec rpopl(arma::pow(r,L+1));
      for(size_t ip=0;ip<x.n_elem;ip++)
        inner.row(ip)/=rpopl(ip);

      return inner;
    }

    arma::mat twoe_integral(double rmin, double rmax, const arma::vec & x, const arma::vec & wx, const polynomial_basis::PolynomialBasis * poly, int L) {
#ifndef ARMA_NO_DEBUG
      if(x.n_elem != wx.n_elem) {
        std::ostringstream oss;
        oss << "x and wx not compatible: " << x.n_elem << " vs " << wx.n_elem << "!\n";
        throw std::logic_error(oss.str());
      }
#endif
      // and half-length of interval is
      double rlen(0.5*(rmax-rmin));

      // Compute the inner integrals
      arma::mat inner(twoe_inner_integral(rmin, rmax, x, wx, poly, L));

      // Evaluate basis functions at quadrature points
      arma::mat bf(poly->eval(x));

      // Product functions
      arma::mat bfprod(bf.n_rows,bf.n_cols*bf.n_cols);
      for(size_t fi=0;fi<bf.n_cols;fi++)
        for(size_t fj=0;fj<bf.n_cols;fj++)
          bfprod.col(fi*bf.n_cols+fj)=bf.col(fi)%bf.col(fj);
      // Put in the weights for the outer integral
      arma::vec wp(wx*rlen);
      for(size_t i=0;i<bfprod.n_cols;i++)
        bfprod.col(i)%=wp;

      // Integrals are then
      arma::mat ints(arma::trans(bfprod)*inner);
      // but we are still missing the second term which can be
      // obtained as simply as
      ints+=arma::trans(ints);

      return ints;
    }
  }
}

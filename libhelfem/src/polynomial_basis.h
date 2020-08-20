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
#ifndef POLYNOMIAL_BASIS_H
#define POLYNOMIAL_BASIS_H

#include <armadillo>

namespace helfem {
  namespace polynomial_basis {
    /// Get primitive indices for a basis with n nodes and n overlapping functions.
    arma::uvec primitive_indices(int nnodes, int noverlap, bool drop_first, bool drop_last);

    /// Template for a primitive basis
    class PolynomialBasis {
    protected:
      /// Number of basis functions
      int nbf;
      /// Number of overlapping functions
      int noverlap;
      /// Identifier
      int id;
      /// Order
      int order;
    public:
      /// Constructor
      PolynomialBasis();
      /// Destructor
      virtual ~PolynomialBasis();
      /// Get a copy
      virtual PolynomialBasis * copy() const=0;

      /// Get number of basis functions
      int get_nbf() const;
      /// Get number of overlapping functions
      int get_noverlap() const;

      /// Get identifier
      int get_id() const;
      /// Get order
      int get_order() const;

      /// Drop first function
      virtual void drop_first()=0;
      /// Drop last function
      virtual void drop_last()=0;

      /// Evaluate polynomials at given points
      virtual arma::mat eval(const arma::vec & x) const=0;
      /// Evaluate polynomials and derivatives at given points
      virtual void eval(const arma::vec & x, arma::mat & f, arma::mat & df) const=0;
      /// Evaluate second derivatives at given point
      virtual void eval_lapl(const arma::vec & x, arma::mat & lf) const;

      /// Print out the basis functions
      void print(const std::string & str="") const;
    };
    /// Get the wanted basis
    PolynomialBasis * get_basis(int primbas, int Nnodes);

    /// Primitive polynomials with Hermite
    class HermiteBasis: public PolynomialBasis {
      /// Primitive polynomial basis expansion
      arma::mat bf_C;
      /// Primitive polynomial basis expansion, derivative
      arma::mat df_C;
    public:
      /// Constructor
      HermiteBasis(int n_nodes, int der_order);
      /// Destructor
      ~HermiteBasis();
      /// Get a copy
      HermiteBasis * copy() const override;

      /// Drop first function
      void drop_first() override;
      /// Drop last function
      void drop_last() override;

      /// Evaluate polynomials at given points
      arma::mat eval(const arma::vec & x) const override;
      /// Evaluate polynomials and derivatives at given points
      void eval(const arma::vec & x, arma::mat & f, arma::mat & df) const override;
      /// Evaluate second derivatives at given points
      void eval_lapl(const arma::vec & x, arma::mat & lf) const override;
    };

    /// Legendre functions
    class LegendreBasis: public PolynomialBasis {
      /// Maximum order
      int lmax;
      /// Transformation matrix
      arma::mat T;

      /// Evaluate Legendre polynomials
      arma::mat f_eval(const arma::vec & x) const;
      /// Evaluate Legendre polynomials' derivatives
      arma::mat df_eval(const arma::vec & x) const;
      /// Evaluate Legendre polynomials' second derivatives
      arma::mat lf_eval(const arma::vec & x) const;
    public:
      /// Constructor
      LegendreBasis(int nfuncs, int id);
      /// Destructor
      ~LegendreBasis();
      /// Get a copy
      LegendreBasis * copy() const override;

      /// Drop first function
      void drop_first() override;
      /// Drop last function
      void drop_last() override;

      /// Evaluate polynomials at given points
      arma::mat eval(const arma::vec & x) const override;
      /// Evaluate polynomials and derivatives at given points
      void eval(const arma::vec & x, arma::mat & f, arma::mat & df) const override;
      /// Evaluate second derivatives at given points
      void eval_lapl(const arma::vec & x, arma::mat & lf) const override;
    };

    /// Lagrange interpolating polynomials
    class LIPBasis: public PolynomialBasis {
      /// Control nodes
      arma::vec x0;
      /// Indices of enabled functions
      arma::uvec enabled;
    public:
      /// Constructor
      LIPBasis(const arma::vec & x0, int id);
      /// Destructor
      ~LIPBasis();
      /// Get a copy
      LIPBasis * copy() const override;

      /// Drop first function
      void drop_first() override;
      /// Drop last function
      void drop_last() override;

      /// Evaluate polynomials at given points
      arma::mat eval(const arma::vec & x) const override;
      /// Evaluate polynomials and derivatives at given points
      void eval(const arma::vec & x, arma::mat & f, arma::mat & df) const override;
      /// Evaluate second derivatives at given points
      void eval_lapl(const arma::vec & x, arma::mat & lf) const override;
    };
  }
}
#endif

/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file Chebyshev2.h
 * @brief Pseudo-spectral parameterization for Chebyshev polynomials of the
 * second kind.
 *
 * In a pseudo-spectral case, rather than the parameters acting as
 * weights for the bases polynomials (as in Chebyshev2Basis), here the
 * parameters are the *values* at a specific set of points in the interval, the
 * "Chebyshev points". These values uniquely determine the polynomial that
 * interpolates them at the Chebyshev points.
 *
 * This is different from Chebyshev.h since it leverage ideas from
 * pseudo-spectral optimization, i.e. we don't decompose into basis functions,
 * rather estimate function values at the Chebyshev points.
 *
 * Please refer to Agrawal21icra for more details.
 *
 * @author Varun Agrawal, Jing Dong, Frank Dellaert
 * @date July 4, 2020
 */

#pragma once

#include <gtsam/base/Manifold.h>
#include <gtsam/base/OptionalJacobian.h>
#include <gtsam/basis/Basis.h>

namespace gtsam {

/**
 * Chebyshev Interpolation on Chebyshev points of the second kind
 * Note that N here, the number of points, is one less than N from
 * 'Approximation Theory and Approximation Practice by L. N. Trefethen (pg.42)'.
 */
class GTSAM_EXPORT Chebyshev2 : public Basis<Chebyshev2> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  using Base = Basis<Chebyshev2>;
  using Parameters = Eigen::Matrix<double, /*Nx1*/ -1, 1>;
  using DiffMatrix = Eigen::Matrix<double, /*NxN*/ -1, -1>;

  /**
   * @brief Specific Chebyshev point, within [-1,1] interval.
   *
   * @param N The degree of the polynomial
   * @param j The index of the Chebyshev point
   * @return double
   */
  static double Point(size_t N, int j);

  /**
   * @brief Specific Chebyshev point, within [a,b] interval.
   *
   * @param N The degree of the polynomial
   * @param j The index of the Chebyshev point
   * @param a Lower bound of interval (default: -1)
   * @param b Upper bound of interval (default: 1)
   * @return double
   */
  static double Point(size_t N, int j, double a, double b);

  /// All Chebyshev points
  static Vector Points(size_t N);

  /// All Chebyshev points, within [a,b] interval
  static Vector Points(size_t N, double a, double b);

  /**
   * Evaluate Chebyshev Weights on [-1,1] at any x up to order N-1 (N values)
   * These weights implement barycentric interpolation at a specific x.
   * More precisely, f(x) ~ [w0;...;wN] * [f0;...;fN], where the fj are the
   * values of the function f at the Chebyshev points. As such, for a given x we
   * obtain a linear map from parameter vectors f to interpolated values f(x).
   * Optional [a,b] interval can be specified as well.
   */
  static Weights CalculateWeights(size_t N, double x, double a = -1, double b = 1);

  /**
   *  Evaluate derivative of barycentric weights.
   *  This is easy and efficient via the DifferentiationMatrix.
   */
  static Weights DerivativeWeights(size_t N, double x, double a = -1, double b = 1);

  /// Compute D = differentiation matrix, Trefethen00book p.53
  /// When given a parameter vector f of function values at the Chebyshev
  /// points, D*f are the values of f'.
  /// https://people.maths.ox.ac.uk/trefethen/8all.pdf Theorem 8.4
  static DiffMatrix DifferentiationMatrix(size_t N);

  /// Compute D = differentiation matrix, for interval [a,b]
  static DiffMatrix DifferentiationMatrix(size_t N, double a, double b);

  /// IntegrationMatrix returns the (N+1)×(N+1) matrix P such that for any f,
  /// F = P * f recovers F (the antiderivative) satisfying f = D * F and F(0)=0.
  static Matrix IntegrationMatrix(size_t N);

  /// IntegrationMatrix returns the (N+1)×(N+1) matrix P for interval [a,b]
  static Matrix IntegrationMatrix(size_t N, double a, double b);

  /**
   *  Calculate Clenshaw-Curtis integration weights.
   *  Trefethen00book, pg 128, clencurt.m
   *  Note that N in clencurt.m is 1 less than our N
   */
  static Weights IntegrationWeights(size_t N);

  /// Calculate Clenshaw-Curtis integration weights, for interval [a,b]
  static Weights IntegrationWeights(size_t N, double a, double b);

  /**
   * Calculate Double Clenshaw-Curtis integration weights
   * We compute them as W * P, where W are the Clenshaw-Curtis weights and P is
   * the integration matrix.
   */
  static Weights DoubleIntegrationWeights(size_t N);

  /// Calculate Double Clenshaw-Curtis integration weights, for interval [a,b]
  static Weights DoubleIntegrationWeights(size_t N, double a, double b);

  /// Create matrix of values at Chebyshev points given vector-valued function.
  static Vector vector(std::function<double(double)> f,
    size_t N, double a = -1, double b = 1);

  /// Create matrix of values at Chebyshev points given vector-valued function.
  template <size_t M>
  static Matrix matrix(std::function<Eigen::Matrix<double, M, 1>(double)> f,
    size_t N, double a = -1, double b = 1) {
    Matrix Xmat(M, N);
    const Vector points = Points(N, a, b);
    for (size_t j = 0; j < N; j++) Xmat.col(j) = f(points(j));
    return Xmat;
  }
};  // \ Chebyshev2

}  // namespace gtsam

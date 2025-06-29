/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file DiscreteFactorGraph.h
 * @date Feb 14, 2011
 * @author Duy-Nguyen Ta
 * @author Frank Dellaert
 * @author Varun Agrawal
 */

#pragma once

#include <gtsam/discrete/DecisionTreeFactor.h>
#include <gtsam/discrete/DiscreteLookupDAG.h>
#include <gtsam/inference/EliminateableFactorGraph.h>
#include <gtsam/inference/FactorGraph.h>
#include <gtsam/inference/Ordering.h>
#include <gtsam/base/FastSet.h>

#include <string>
#include <utility>
#include <vector>

namespace gtsam {

// Forward declarations
class DiscreteFactorGraph;
class DiscreteConditional;
class DiscreteBayesNet;
class DiscreteEliminationTree;
class DiscreteBayesTree;
class DiscreteJunctionTree;

/**
 * @brief Main elimination function for DiscreteFactorGraph.
 *
 * @param factors The factor graph to eliminate.
 * @param frontalKeys An ordering for which variables to eliminate.
 * @return A pair of the resulting conditional and the separator factor.
 * @ingroup discrete
 */
GTSAM_EXPORT
std::pair<DiscreteConditional::shared_ptr, DiscreteFactor::shared_ptr>
EliminateDiscrete(const DiscreteFactorGraph& factors,
                  const Ordering& frontalKeys);

/**
 * @brief Alternate elimination function for that creates non-normalized lookup tables.
 *
 * @param factors The factor graph to eliminate.
 * @param frontalKeys An ordering for which variables to eliminate.
 * @return A pair of the resulting lookup table and the separator factor.
 * @ingroup discrete
 */
GTSAM_EXPORT
std::pair<DiscreteConditional::shared_ptr, DiscreteFactor::shared_ptr>
EliminateForMPE(const DiscreteFactorGraph& factors,
                const Ordering& frontalKeys);

template<> struct EliminationTraits<DiscreteFactorGraph>
{
  typedef DiscreteFactor FactorType;                   ///< Type of factors in factor graph
  typedef DiscreteFactorGraph FactorGraphType;         ///< Type of the factor graph (e.g. DiscreteFactorGraph)
  typedef DiscreteConditional ConditionalType;         ///< Type of conditionals from elimination
  typedef DiscreteBayesNet BayesNetType;               ///< Type of Bayes net from sequential elimination
  typedef DiscreteEliminationTree EliminationTreeType; ///< Type of elimination tree
  typedef DiscreteBayesTree BayesTreeType;             ///< Type of Bayes tree
  typedef DiscreteJunctionTree JunctionTreeType;       ///< Type of Junction tree
  
  /// The default dense elimination function
  static std::pair<std::shared_ptr<ConditionalType>,
                   std::shared_ptr<FactorType> >
  DefaultEliminate(const FactorGraphType& factors, const Ordering& keys) {
    return EliminateDiscrete(factors, keys);
  }
  
  /// The default ordering generation function
  static Ordering DefaultOrderingFunc(
      const FactorGraphType& graph,
      std::optional<std::reference_wrapper<const VariableIndex>> variableIndex) {
    return Ordering::Colamd((*variableIndex).get());
  }
};

/**
 * A Discrete Factor Graph is a factor graph where all factors are Discrete, i.e.
 *   Factor == DiscreteFactor
 * @ingroup discrete
 */
class GTSAM_EXPORT DiscreteFactorGraph
    : public FactorGraph<DiscreteFactor>,
      public EliminateableFactorGraph<DiscreteFactorGraph> {
 public:
  using This = DiscreteFactorGraph;          ///< this class
  using Base = FactorGraph<DiscreteFactor>;  ///< base factor graph type
  using BaseEliminateable =
      EliminateableFactorGraph<This>;          ///< for elimination
  using shared_ptr = std::shared_ptr<This>;  ///< shared_ptr to This

  using Values = DiscreteValues;  ///< backwards compatibility

  using Indices = KeyVector;  ///> map from keys to values

  /** Default constructor */
  DiscreteFactorGraph() {}

  /** Construct from iterator over factors */
  template <typename ITERATOR>
  DiscreteFactorGraph(ITERATOR firstFactor, ITERATOR lastFactor)
      : Base(firstFactor, lastFactor) {}

  /** Construct from container of factors (shared_ptr or plain objects) */
  template <class CONTAINER>
  explicit DiscreteFactorGraph(const CONTAINER& factors) : Base(factors) {}

  /** Implicit copy/downcast constructor to override explicit template container
   * constructor */
  template <class DERIVED_FACTOR>
  DiscreteFactorGraph(const FactorGraph<DERIVED_FACTOR>& graph) : Base(graph) {}

  /// Destructor
  virtual ~DiscreteFactorGraph() {}

  /// @name Testable
  /// @{

  bool equals(const This& fg, double tol = 1e-9) const;

  /// @}

  //TODO(Varun): Make compatible with TableFactor
  /** Add a decision-tree factor */
  template <typename... Args>
  void add(Args&&... args) {
    emplace_shared<DecisionTreeFactor>(std::forward<Args>(args)...);
  }

  /** Return the set of variables involved in the factors (set union) */
  KeySet keys() const;

  /// Return the DiscreteKeys in this factor graph.
  DiscreteKeys discreteKeys() const;

  /** return product of all factors as a single factor */
  DiscreteFactor::shared_ptr product() const;

  /**
   * @brief Return product of all `factors` as a single factor,
   * which is scaled by the max value to prevent underflow
   *
   * @param factors The factors to multiply as a DiscreteFactorGraph.
   * @return DiscreteFactor::shared_ptr
   */
  DiscreteFactor::shared_ptr scaledProduct() const;

  /** 
   * Evaluates the factor graph given values, returns the joint probability of
   * the factor graph given specific instantiation of values
   */
  double operator()(const DiscreteValues& values) const;

  /// print
  void print(
      const std::string& s = "DiscreteFactorGraph",
      const KeyFormatter& formatter = DefaultKeyFormatter) const override;

  /**
   * @brief Implement the sum-product algorithm
   *
   * @param orderingType : one of COLAMD, METIS, NATURAL, CUSTOM
   * @return DiscreteBayesNet encoding posterior P(X|Z)
   */
  DiscreteBayesNet sumProduct(
      OptionalOrderingType orderingType = {}) const;

  /**
   * @brief Implement the sum-product algorithm
   *
   * @param ordering
   * @return DiscreteBayesNet encoding posterior P(X|Z)
   */
  DiscreteBayesNet sumProduct(const Ordering& ordering) const;

  /**
   * @brief Implement the max-product algorithm
   *
   * @param orderingType : one of COLAMD, METIS, NATURAL, CUSTOM
   * @return DiscreteLookupDAG DAG with lookup tables
   */
  DiscreteLookupDAG maxProduct(
      OptionalOrderingType orderingType = {}) const;

  /**
   * @brief Implement the max-product algorithm
   *
   * @param ordering
   * @return DiscreteLookupDAG `DAG with lookup tables
   */
  DiscreteLookupDAG maxProduct(const Ordering& ordering) const;

  /**
   * @brief Find the maximum probable explanation (MPE) by doing max-product.
   *
   * @param orderingType
   * @return DiscreteValues : MPE
   */
  DiscreteValues optimize(
      OptionalOrderingType orderingType = {}) const;

  /**
   * @brief Find the maximum probable explanation (MPE) by doing max-product.
   *
   * @param ordering
   * @return DiscreteValues : MPE
   */
  DiscreteValues optimize(const Ordering& ordering) const;

  /// @name Wrapper support
  /// @{

  /**
   * @brief Render as markdown tables
   *
   * @param keyFormatter GTSAM-style Key formatter.
   * @param names optional, a map from Key to category names.
   * @return std::string a (potentially long) markdown string.
   */
  std::string markdown(const KeyFormatter& keyFormatter = DefaultKeyFormatter,
                       const DiscreteFactor::Names& names = {}) const;

  /**
   * @brief Render as html tables
   *
   * @param keyFormatter GTSAM-style Key formatter.
   * @param names optional, a map from Key to category names.
   * @return std::string a (potentially long) html string.
   */
  std::string html(const KeyFormatter& keyFormatter = DefaultKeyFormatter,
                   const DiscreteFactor::Names& names = {}) const;

  /// @}
  /// @name HybridValues methods.
  /// @{

  using Base::error;  // Expose error(const HybridValues&) method..

  /// @}
};  // \ DiscreteFactorGraph

/// traits
template <>
struct traits<DiscreteFactorGraph> : public Testable<DiscreteFactorGraph> {};

}  // namespace gtsam

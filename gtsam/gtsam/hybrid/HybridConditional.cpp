/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 *  @file HybridConditional.cpp
 *  @date Mar 11, 2022
 *  @author Fan Jiang
 *  @author Varun Agrawal
 */

#include <gtsam/hybrid/HybridConditional.h>
#include <gtsam/hybrid/HybridFactor.h>
#include <gtsam/hybrid/HybridValues.h>
#include <gtsam/inference/Conditional-inst.h>
#include <gtsam/inference/Key.h>

namespace gtsam {

/* ************************************************************************ */
HybridConditional::HybridConditional(const KeyVector &continuousFrontals,
                                     const DiscreteKeys &discreteFrontals,
                                     const KeyVector &continuousParents,
                                     const DiscreteKeys &discreteParents)
    : HybridConditional(CollectKeys(continuousFrontals, continuousParents),
                        CollectDiscreteKeys(discreteFrontals, discreteParents),
                        continuousFrontals.size() + discreteFrontals.size()) {}

/* ************************************************************************ */
HybridConditional::HybridConditional(
    const std::shared_ptr<GaussianConditional> &continuousConditional)
    : HybridConditional(continuousConditional->keys(), {},
                        continuousConditional->nrFrontals()) {
  inner_ = continuousConditional;
}

/* ************************************************************************ */
HybridConditional::HybridConditional(
    const std::shared_ptr<DiscreteConditional> &discreteConditional)
    : HybridConditional({}, discreteConditional->discreteKeys(),
                        discreteConditional->nrFrontals()) {
  inner_ = discreteConditional;
}

/* ************************************************************************ */
HybridConditional::HybridConditional(
    const std::shared_ptr<HybridGaussianConditional> &hybridGaussianCond)
    : BaseFactor(hybridGaussianCond->continuousKeys(),
                 hybridGaussianCond->discreteKeys()),
      BaseConditional(hybridGaussianCond->nrFrontals()) {
  inner_ = hybridGaussianCond;
}

/* ************************************************************************ */
void HybridConditional::print(const std::string &s,
                              const KeyFormatter &formatter) const {
  std::cout << s;

  if (inner_) {
    inner_->print("", formatter);
  } else {
    if (isContinuous()) std::cout << "Continuous ";
    if (isDiscrete()) std::cout << "Discrete ";
    if (isHybrid()) std::cout << "Hybrid ";
    BaseConditional::print("", formatter);

    std::cout << "P(";
    size_t index = 0;
    const size_t N = keys().size();
    const size_t contN = N - discreteKeys_.size();
    while (index < N) {
      if (index > 0) {
        if (index == nrFrontals_)
          std::cout << " | ";
        else
          std::cout << ", ";
      }
      if (index < contN) {
        std::cout << formatter(keys()[index]);
      } else {
        auto &dk = discreteKeys_[index - contN];
        std::cout << "(" << formatter(dk.first) << ", " << dk.second << ")";
      }
      index++;
    }
  }
}

/* ************************************************************************ */
bool HybridConditional::equals(const HybridFactor &other, double tol) const {
  const This *e = dynamic_cast<const This *>(&other);
  if (e == nullptr) return false;
  if (auto gm = asHybrid()) {
    auto other = e->asHybrid();
    return other != nullptr && gm->equals(*other, tol);
  } else if (auto gc = asGaussian()) {
    auto other = e->asGaussian();
    return other != nullptr && gc->equals(*other, tol);
  } else if (auto dc = asDiscrete()) {
    auto other = e->asDiscrete();
    return other != nullptr && dc->equals(*other, tol);
  } else
    return inner_ ? (e->inner_ ? inner_->equals(*(e->inner_), tol) : false)
                  : !(e->inner_);
}

/* ************************************************************************ */
double HybridConditional::error(const HybridValues &hybridValues) const {
  if (auto gc = asGaussian()) {
    return gc->error(hybridValues.continuous());
  } else if (auto gm = asHybrid()) {
    return gm->error(hybridValues);
  } else if (auto dc = asDiscrete()) {
    return dc->error(hybridValues.discrete());
  } else
    throw std::runtime_error(
        "HybridConditional::error: conditional type not handled");
}

/* ************************************************************************ */
AlgebraicDecisionTree<Key> HybridConditional::errorTree(
    const VectorValues &continuousValues) const {
  if (auto gc = asGaussian()) {
    return {gc->error(continuousValues)};  // NOTE: a "constant" tree
  } else if (auto gm = asHybrid()) {
    return gm->errorTree(continuousValues);
  } else if (auto dc = asDiscrete()) {
    return dc->errorTree();
  } else
    throw std::runtime_error(
        "HybridConditional::error: conditional type not handled");
}

/* ************************************************************************ */
double HybridConditional::logProbability(const HybridValues &values) const {
  if (auto gc = asGaussian()) {
    return gc->logProbability(values.continuous());
  } else if (auto gm = asHybrid()) {
    return gm->logProbability(values);
  } else if (auto dc = asDiscrete()) {
    return dc->logProbability(values.discrete());
  } else
    throw std::runtime_error(
        "HybridConditional::logProbability: conditional type not handled");
}

/* ************************************************************************ */
double HybridConditional::negLogConstant() const {
  if (auto gc = asGaussian()) {
    return gc->negLogConstant();
  } else if (auto gm = asHybrid()) {
    return gm->negLogConstant();
  } else if (auto dc = asDiscrete()) {
    return dc->negLogConstant();  // 0.0!
  } else
    throw std::runtime_error(
        "HybridConditional::negLogConstant: conditional type not handled");
}

/* ************************************************************************ */
double HybridConditional::evaluate(const HybridValues &values) const {
  return std::exp(logProbability(values));
}

/* ************************************************************************ */
std::shared_ptr<Factor> HybridConditional::restrict(
    const DiscreteValues &assignment) const {
  if (auto gc = asGaussian()) {
    return std::make_shared<HybridConditional>(gc);
  } else if (auto dc = asDiscrete()) {
    return std::make_shared<HybridConditional>(dc);
  };

  auto hgc = asHybrid();
  if (!hgc)
    throw std::runtime_error(
        "HybridConditional::restrict: conditional type not handled");

  // Case 1: Fully determined, return corresponding Gaussian conditional
  auto parentValues = assignment.filter(discreteKeys_);
  if (parentValues.size() == discreteKeys_.size()) {
    return std::make_shared<HybridConditional>(hgc->choose(parentValues));
  }

  // Case 2: Some live parents remain, build a new tree
  auto remainingKeys = assignment.missingKeys(discreteKeys_);
  if (!remainingKeys.empty()) {
    auto newTree = hgc->factors();
    for (const auto &[key, value] : parentValues) {
      newTree = newTree.choose(key, value);
    }
    return std::make_shared<HybridConditional>(
        std::make_shared<HybridGaussianConditional>(remainingKeys, newTree));
  }

  // Case 3: No changes needed, return original
  return std::make_shared<HybridConditional>(hgc);
}

/* ************************************************************************ */
}  // namespace gtsam

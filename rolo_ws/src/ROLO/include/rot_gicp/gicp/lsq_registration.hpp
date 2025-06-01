#ifndef FAST_GICP_LSQ_REGISTRATION_HPP
#define FAST_GICP_LSQ_REGISTRATION_HPP

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/registration/registration.h>

namespace fast_gicp {

enum class LSQ_OPTIMIZER_TYPE { GaussNewton, LevenbergMarquardt, SO3_LevenbergMarquardt };

template<typename PointSource, typename PointTarget>
class LsqRegistration : public pcl::Registration<PointSource, PointTarget, float> {
public:
  using Scalar = float;
  using Matrix4 = typename pcl::Registration<PointSource, PointTarget, Scalar>::Matrix4;

  using PointCloudSource = typename pcl::Registration<PointSource, PointTarget, Scalar>::PointCloudSource;
  using PointCloudSourcePtr = typename PointCloudSource::Ptr;
  using PointCloudSourceConstPtr = typename PointCloudSource::ConstPtr;

  using PointCloudTarget = typename pcl::Registration<PointSource, PointTarget, Scalar>::PointCloudTarget;
  using PointCloudTargetPtr = typename PointCloudTarget::Ptr;
  using PointCloudTargetConstPtr = typename PointCloudTarget::ConstPtr;

#if PCL_VERSION >= PCL_VERSION_CALC(1, 10, 0)
  using Ptr = pcl::shared_ptr<LsqRegistration<PointSource, PointTarget>>;
  using ConstPtr = pcl::shared_ptr<const LsqRegistration<PointSource, PointTarget>>;
#else
  using Ptr = boost::shared_ptr<LsqRegistration<PointSource, PointTarget>>;
  using ConstPtr = boost::shared_ptr<const LsqRegistration<PointSource, PointTarget>>;
#endif

protected:
  using pcl::Registration<PointSource, PointTarget, Scalar>::input_;
  using pcl::Registration<PointSource, PointTarget, Scalar>::nr_iterations_;
  using pcl::Registration<PointSource, PointTarget, Scalar>::max_iterations_;
  using pcl::Registration<PointSource, PointTarget, Scalar>::final_transformation_;
  using pcl::Registration<PointSource, PointTarget, Scalar>::transformation_epsilon_;
  using pcl::Registration<PointSource, PointTarget, Scalar>::converged_;

public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  LsqRegistration();
  virtual ~LsqRegistration();

  void setRotationEpsilon(double eps);
  void setInitialLambdaFactor(double init_lambda_factor);
  void setDebugPrint(bool lm_debug_print);

  const Eigen::Matrix<double, 6, 6>& getFinalHessian() const;

  double evaluateCost(const Eigen::Matrix4f& relative_pose, Eigen::Matrix<double, 6, 6>* H = nullptr, Eigen::Matrix<double, 6, 1>* b = nullptr);

  virtual void swapSourceAndTarget() {}
  virtual void clearSource() {}
  virtual void clearTarget() {}
  void setOptimizerType(LSQ_OPTIMIZER_TYPE optimizier_type_);

protected:

  virtual void computeTranslation(PointCloudSource& output, Eigen::Vector3d& trans,
                          const Eigen::Vector3d& init_guess, const Eigen::Vector3d& last_t0, 
                          const double interval_tn, const double interval_tn_1, const float ct_lambda);

  bool rot_step_lm(Eigen::Vector3d& t0, Eigen::Vector3d& delta);

  virtual void computeTransformation(PointCloudSource& output, const Matrix4& guess) override;

  bool is_converged(const Eigen::Isometry3d& delta) const;

  virtual double linearize(const Eigen::Isometry3d& trans, Eigen::Matrix<double, 6, 6>* H = nullptr, Eigen::Matrix<double, 6, 1>* b = nullptr) = 0;
  virtual double compute_error(const Eigen::Isometry3d& trans) = 0;
  virtual double compute_t_error(const Eigen::Vector3d& trans, const Eigen::Vector3d& init_guess, const Eigen::Vector3d& last_t0, 
                                 const double& interval_tn, const double& interval_tn_1) = 0;

  bool step_optimize(Eigen::Isometry3d& x0, Eigen::Isometry3d& delta);
  bool step_gn(Eigen::Isometry3d& x0, Eigen::Isometry3d& delta);
  bool step_lm(Eigen::Isometry3d& x0, Eigen::Isometry3d& delta);

  bool is_rot_converged(const Eigen::Isometry3d& delta) const;
  bool rot_step_lm(Eigen::Isometry3d& x0, Eigen::Isometry3d& delta);
  virtual double so3_linearize(const Eigen::Isometry3d& trans, Eigen::Matrix<double, 3, 3>* H = nullptr, Eigen::Matrix<double, 3, 1>* b = nullptr) {};
  virtual double t3_linearize(const Eigen::Vector3d& trans, const Eigen::Vector3d& init_guess, const Eigen::Vector3d& last_t0, 
                              const double interval_tn, const double interval_tn_1,
                              Eigen::Matrix<double, 6, 6>* H, Eigen::Matrix<double, 6, 1>* b) {};
  bool step_t_optimize(Eigen::Vector3d& x0, Eigen::Vector3d& delta,
                       const Eigen::Vector3d& init_guess, const Eigen::Vector3d& last_t0, 
                       const double& interval_tn, const double& interval_tn_1);
  bool is_t_converged(const Eigen::Vector3d& delta) const;

protected:
  double rotation_epsilon_;

  LSQ_OPTIMIZER_TYPE lsq_optimizer_type_;
  int lm_max_iterations_;
  double lm_init_lambda_factor_;
  double lm_lambda_;
  bool lm_debug_print_;

  Eigen::Matrix<double, 6, 6> final_hessian_;
  Eigen::Matrix<double, 3, 3> so3_final_hessian_;
};
}  // namespace fast_gicp

#endif
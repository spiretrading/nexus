#ifndef SPIRE_HPP
#define SPIRE_HPP
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal_type.hpp>
#ifdef _DEBUG
  #include <QDebug>
#endif

namespace Spire {
  class CancelKeySequenceValidationModel;
  class Definitions;
  class KeySequenceValidationModel;
  class LocalRangeInputModel;
  class LocalTechnicalsModel;
  template<typename T> class QtFuture;
  template<typename T> class QtPromise;
  class RangeInputModel;
  class RealSpinBoxModel;
  class Scalar;
  template<typename T> class SpinBoxModel;
  class SpireController;
  template<typename T> struct SubscriptionResult;
  class TaskKeySequenceValidationModel;
  class TechnicalsModel;
  class TestKeySequenceValidationModel;

  //! Defines the common type of boost signal used throughout Spire.
  template<typename F>
  using Signal = typename boost::signals2::signal_type<F,
    boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>>::type;
}

#endif

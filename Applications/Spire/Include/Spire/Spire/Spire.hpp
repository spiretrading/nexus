#ifndef SPIRE_HPP
#define SPIRE_HPP
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal_type.hpp>
#ifdef _DEBUG
  #include <QDebug>
#endif

namespace Spire {
  class AnyListModel;
  template<typename T> class ArrayListModel;
  class CancelKeySequenceValidationModel;
  template<typename T> class ColumnViewListModel;
  template<typename T> class CompositeValueModel;
  template<typename T> class ConstantValueModel;
  class Definitions;
  class FieldPointer;
  class KeySequenceValidationModel;
  template<typename T> class ListIndexValueModel;
  template<typename T> class ListModel;
  template<typename T> class ListValueModel;
  template<typename T> class LocalCompositeValueModel;
  class LocalRangeInputModel;
  template<typename T> class LocalScalarValueModel;
  class LocalTechnicalsModel;
  template<typename T> class LocalValueModel;
  template<typename T> class QtFuture;
  template<typename T> class QtPromise;
  class RangeInputModel;
  class RealSpinBoxModel;
  template<typename T> class RowViewListModel;
  class Scalar;
  template<typename T> class ScalarValueModel;
  template<typename T> class ScalarValueModelDecorator;
  template<typename T> class SharedValueModel;
  class SharedValueModelBox;
  template<typename T> class SpinBoxModel;
  class SpireController;
  template<typename T> struct SubscriptionResult;
  class TaskKeySequenceValidationModel;
  class TechnicalsModel;
  class TestKeySequenceValidationModel;
  template<typename T> class ToDecimalModel;
  template<typename T> class ToTextModel;
  template<typename T, typename U, typename F, typename G>
    class TransformValueModel;
  template<typename T> class ValueModel;

  /** Defines the common type of boost signal used throughout Spire. */
  template<typename F>
  using Signal = typename boost::signals2::signal_type<F,
    boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>>::type;
}

#endif

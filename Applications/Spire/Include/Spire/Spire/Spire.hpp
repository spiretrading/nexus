#ifndef SPIRE_HPP
#define SPIRE_HPP
#ifdef _DEBUG
  #include <QDebug>
#endif

namespace Spire {
  class Definitions;
  class LocalRangeInputModel;
  class LocalTechnicalsModel;
  template<typename T> class QtFuture;
  template<typename T> class QtPromise;
  class RangeInputModel;
  class Scalar;
  class SpireController;
  template<typename T> struct SubscriptionResult;
  class TechnicalsModel;
}

#endif

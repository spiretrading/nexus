#ifndef SPIRE_HPP
#define SPIRE_HPP
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal_type.hpp>

namespace spire {
  template<typename T> class qt_promise;
  class spire_controller;

  //! Defines the common type of boost signal used throughout Spire.
  template<typename F>
  using signal = typename boost::signals2::signal_type<F,
    boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>>::type;
}

#endif

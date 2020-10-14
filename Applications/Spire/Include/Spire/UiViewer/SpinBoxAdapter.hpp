#ifndef SPIRE_SPIN_BOX_TEST_WIDGET_ADAPTER_HPP
#define SPIRE_SPIN_BOX_TEST_WIDGET_ADAPTER_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class SpinBoxAdapter : public QWidget {
    public:

      using ChangeSignal = Signal<void (const QString& value)>;

      SpinBoxAdapter(QWidget* parent = nullptr);

      virtual ~SpinBoxAdapter() = default;

      virtual bool reset(const QString& initial, const QString& minimum,
        const QString& maximum, const QString& increment) = 0;

      virtual QString get_initial() const = 0;

      virtual bool set_initial(const QString& initial) = 0;

      virtual QString get_minimum() const = 0;

      virtual bool set_minimum(const QString& minimum) = 0;

      virtual QString get_maximum() const = 0;

      virtual bool set_maximum(const QString& maximum) = 0;

      virtual QString get_increment(Qt::KeyboardModifiers modifiers) const = 0;

      virtual bool set_increment(Qt::KeyboardModifiers modifiers,
        const QString& increment) = 0;

      virtual boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const = 0;
  };
}

#endif

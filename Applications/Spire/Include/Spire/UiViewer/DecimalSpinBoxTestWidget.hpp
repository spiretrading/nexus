#ifndef SPIRE_DECIMAL_SPIN_BOX_TEST_WIDGET_HPP
#define SPIRE_DECIMAL_SPIN_BOX_TEST_WIDGET_HPP
#include <QHBoxLayout>
#include "Spire/Ui/DecimalSpinBox.hpp"
#include "Spire/UiViewer/SpinBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for interacting with a DecimalSpinBox being tested.
  class DecimalSpinBoxTestWidget : public SpinBoxAdapter {
    public:

      //! Constructs a DecimalSpinBoxTestWidget.
      /*
        \param parent The parent widget.
      */
      explicit DecimalSpinBoxTestWidget(QWidget* parent = nullptr);

      bool reset(const QString& initial, const QString& minimum,
        const QString& maximum, const QString& increment) override;

      QString get_initial() const override;

      bool set_initial(const QString& initial) override;

      QString get_minimum() const override;

      bool set_minimum(const QString& minimum) override;

      QString get_maximum() const override;

      bool set_maximum(const QString& maximum) override;

      QString get_increment(Qt::KeyboardModifiers modifiers) const override;

      bool set_increment(Qt::KeyboardModifiers modifiers,
        const QString& increment) override;

      boost::signals2::connection connect_change_signal(
        const ChangeSignal::slot_type& slot) const override;

    private:
      mutable ChangeSignal m_change_signal;
      DecimalSpinBox* m_spin_box;
      std::shared_ptr<DecimalSpinBoxModel> m_model;
      QHBoxLayout* m_layout;
  };
}

#endif

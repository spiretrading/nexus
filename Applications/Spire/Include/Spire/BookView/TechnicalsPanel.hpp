#ifndef SPIRE_TECHNICALS_PANEL_HPP
#define SPIRE_TECHNICALS_PANEL_HPP
#include <QWidget>
#include <QGridLayout>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/LabeledDataWidget.hpp"
#include "Spire/Spire/Signal.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Displays a security's technical indicators.
  class TechnicalsPanel : public QWidget {
    public:

      //! Constructs a TechnicalsPanel.
      explicit TechnicalsPanel(QWidget* parent = nullptr);

      //! Sets the model the panel gets its data from.
      /*!
        \param model The associated model.
      */
      void set_model(std::shared_ptr<BookViewModel> model);

      //! Sets the model to null, and resets the text of all labels to their
      //! default values.
      void reset_model();

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      QGridLayout* m_layout;
      LabeledDataWidget* m_high_label_widget;
      LabeledDataWidget* m_open_label_widget;
      LabeledDataWidget* m_defaults_label_widget;
      LabeledDataWidget* m_low_label_widget;
      LabeledDataWidget* m_close_label_widget;
      LabeledDataWidget* m_volume_label_widget;
      std::shared_ptr<BookViewModel> m_model;
      boost::signals2::scoped_connection m_close_connection;
      boost::signals2::scoped_connection m_high_connection;
      boost::signals2::scoped_connection m_low_connection;
      boost::signals2::scoped_connection m_open_connection;
      boost::signals2::scoped_connection m_volume_connection;
      CustomVariantItemDelegate* m_item_delegate;

      void set_labeled_data_long_form_text();
      void set_labeled_data_short_form_text();
      void on_close_signal(const Nexus::Money& close);
      void on_high_signal(const Nexus::Money& high);
      void on_low_signal(const Nexus::Money& low);
      void on_open_signal(const Nexus::Money& open);
      void on_volume_signal(const Nexus::Quantity& volume);
  };
}

#endif

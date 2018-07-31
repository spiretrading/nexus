#ifndef SPIRE_TECHNICALS_PANEL_HPP
#define SPIRE_TECHNICALS_PANEL_HPP
#include <QWidget>
#include <QGridLayout>
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/labeled_data_widget.hpp"

namespace spire {

  //! Displays a security's technical indicators.
  class technicals_panel : public QWidget {
    public:

      //! Constructs a technicals_panel.
      technicals_panel(QWidget* parent = nullptr);

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      QGridLayout* m_layout;
      labeled_data_widget* m_high_label_widget;
      labeled_data_widget* m_open_label_widget;
      labeled_data_widget* m_defaults_label_widget;
      labeled_data_widget* m_low_label_widget;
      labeled_data_widget* m_close_label_widget;
      labeled_data_widget* m_volume_label_widget;

      void set_labeled_data_long_form_text();
      void set_labeled_data_short_form_text();
  };
}

#endif

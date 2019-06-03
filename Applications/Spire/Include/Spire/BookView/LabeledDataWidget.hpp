#ifndef SPIRE_LABELED_DATA_WIDGET_HPP
#define SPIRE_LABELED_DATA_WIDGET_HPP
#include <QWidget>
#include <QLabel>
#include "Spire/BookView/BookView.hpp"

namespace Spire {

  //! Represents a label with a label string and data string.
  class LabeledDataWidget : public QWidget {
    public:

      //! Constructs a LabeledDataWidget with label text and no data text.
      /*
        \param label_text The label text string.
        \param parent The parent to this widget.
      */
      explicit LabeledDataWidget(const QString& label_text,
        QWidget* parent = nullptr);

      //! Constructs a LabeledDataWidget with label text and data text.
      /*
        \param label_text The label text string.
        \param data_text The data text value.
        \param parent The parent to this widget.
      */
      LabeledDataWidget(const QString& label_text, const QString& data_text,
        QWidget* parent = nullptr);

      //! Sets the label text.
      void set_label_text(const QString& text);

      //! Sets the data text.
      void set_data_text(const QString& text);

      //! Sets the width of the label text's widget.
      void set_label_width(int width);

    private:
      QLabel* m_label;
      QLabel* m_data_label;
  };
}

#endif

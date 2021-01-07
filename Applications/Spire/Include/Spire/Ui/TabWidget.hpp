#ifndef SPIRE_TAB_WIDGET_HPP
#define SPIRE_TAB_WIDGET_HPP
#include <QTabWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a Spire-styled QTabWidget.
  class TabWidget : public QTabWidget {
    public:

      //! Represents the supported tab content padding styles.
      enum class PaddingStyle {

        //! The content has no padding.
        NONE,

        //! The content has padding on the left and right sidess.
        HORIZONTAL
      };

      //! Constructs a TabWidget with PaddingStyle::HORIZONTAL.
      /*!
        \param parent The parent widget.
      */
      explicit TabWidget(QWidget* parent = nullptr);

      //! Constructs a TabWidget with a specific PaddingStyle.
      /*!
        \param padding_style The TabWidget's PaddingStyle.
        \param parent The parent widget.
      */
      explicit TabWidget(PaddingStyle padding_style,
        QWidget* parent = nullptr);

    protected:
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif

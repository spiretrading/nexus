#ifndef SPIRE_TAB_WIDGET_HPP
#define SPIRE_TAB_WIDGET_HPP
#include <QTabWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a Spire-styled QTabWidget.
  class TabWidget : public QTabWidget {
    public:

      //! Represents the supported tab bar padding styles.
      enum class PaddingStyle {

        //! The tab bar has no padding.
        NONE,

        //! The tab bar has left padding.
        LEFT
      };

      //! Constructs a TabWidget with PaddingStyle::NONE.
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

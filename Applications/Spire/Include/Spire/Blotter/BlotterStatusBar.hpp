#ifndef SPIRE_BLOTTER_STATUS_BAR_HPP
#define SPIRE_BLOTTER_STATUS_BAR_HPP
#include <QBoxLayout>
#include <QWidget>

namespace Spire {

  /** Displays a blotter's summary values and message status. */
  class BlotterStatusBar : public QWidget {
    public:

      /**
       * Constructs a BlotterStatusBar.
       * @param parent The parent widget.
       */
      explicit BlotterStatusBar(QWidget* parent = nullptr);

      QSize minimumSizeHint() const override;

    private:
      QBoxLayout* m_compact_layout;
  };
}

#endif

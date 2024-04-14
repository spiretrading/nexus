#ifndef SPIRE_ADAPTIVE_BOX_HPP
#define SPIRE_ADAPTIVE_BOX_HPP
#include <QStackedLayout>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Implements a containter choosing the most appropriate layout for
   * different sizes.
   */
  class AdaptiveBox : public QWidget {
    public:

      /**
       * Constructs an AdaptiveBox.
       * @param parent The parent widget.
       */
      explicit AdaptiveBox(QWidget* parent = nullptr);

      /**
       * Add a layout.
       * @param layout The layout to append to the AdaptiveBox.
       */
      void add(QLayout& layout);

      QSize sizeHint() const override;

      QSize minimumSizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QStackedLayout* m_stacked_layout;
      std::unordered_map<QWidget*, std::vector<QLayoutItem*>> m_placeholders;

      void take_control_placeholders(QWidget* owner);
      void release_control_placeholders(QWidget* owner);
      void update_layout();
  };
}

#endif

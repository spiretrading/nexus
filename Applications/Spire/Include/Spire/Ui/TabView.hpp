#ifndef SPIRE_TAB_VIEW_HPP
#define SPIRE_TAB_VIEW_HPP
#include <memory>
#include <vector>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a tab bar that can be used to display one of multiple components.
   */
  class TabView : public QWidget {
    public:

      /** Selects the divider displayed within a tab. */
      using Divider = Styles::StateSelector<void, struct DividerTag>;

      /**
       * Constructs an empty TabView.
       * @param parent The parent widget.
       */
      explicit TabView(QWidget* parent = nullptr);

      /**
       * Adds a new tab to the end of the tab bar.
       * @param label The tab's label.
       * @param body The component to display when the tab is current.
       */
      void add(const QString& label, QWidget& body);

      /**
       * Adds a new tab to the end of the tab bar.
       * @param labels The list of labels to select from based on the size of
       *        the tab.
       * @param body The component to display when the tab is current.
       */
      void add(std::vector<QString> labels, QWidget& body);

      QSize sizeHint() const override;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      std::shared_ptr<ArrayListModel<std::vector<QString>>> m_labels;
      ListView* m_tab_list;
      std::vector<QWidget*> m_bodies;
      boost::optional<int> m_current;

      void on_current(boost::optional<int> current);
  };
}

#endif

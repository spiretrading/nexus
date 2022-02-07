#ifndef SPIRE_TAB_VIEW_HPP
#define SPIRE_TAB_VIEW_HPP
#include <memory>
#include <vector>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
  class TabView : public QWidget {
    public:
      using Divider = Styles::StateSelector<void, struct DividerTag>;

      using LastTab = Styles::StateSelector<void, struct LastTabTag>;

      explicit TabView(QWidget* parent = nullptr);

      void add(const QString& label, QWidget& body);

      void add(std::vector<QString> labels, QWidget& body);

    private:
      std::shared_ptr<ArrayListModel<std::vector<QString>>> m_labels;
      ListView* m_tab_list;
      std::vector<QWidget*> m_bodies;

      void on_current(boost::optional<int> current);
  };
}

#endif

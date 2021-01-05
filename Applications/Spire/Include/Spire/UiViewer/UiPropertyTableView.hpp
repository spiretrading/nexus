#ifndef SPIRE_UI_PROPERTY_TABLE_VIEW_HPP
#define SPIRE_UI_PROPERTY_TABLE_VIEW_HPP
#include <memory>
#include <vector>
#include <QTableWidget>
#include <QWidget>
#include "Spire/UiViewer/UiProperty.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {
  class UiPropertyTableView : public QWidget {
    public:
      explicit UiPropertyTableView(
        std::vector<std::shared_ptr<UiProperty>> properties,
        QWidget* parent = nullptr);

    private:
      std::vector<std::shared_ptr<UiProperty>> m_properties;
      QTableWidget* m_table;
  };
}

#endif

#include "Spire/Ui/TableHeaderCell.hpp"
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct SortIndicator : QWidget {
    static const QImage& ASCENDING_IMAGE() {
      static auto image =
        imageFromSvg(":/Icons/sort_ascending.svg", scale(5, 6));
      return image;
    }

    static const QImage& DESCENDING_IMAGE() {
      static auto image =
        imageFromSvg(":/Icons/sort_descending.svg", scale(5, 6));
      return image;
    }

    std::shared_ptr<ValueModel<TableHeaderCell::Order>> m_order;
    scoped_connection m_order_connection;

    explicit SortIndicator(
        std::shared_ptr<ValueModel<TableHeaderCell::Order>> order)
        : m_order(std::move(order)) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      on_order(m_order->get());
      m_order_connection = m_order->connect_update_signal(
        std::bind_front(&SortIndicator::on_order, this));
    }

    void on_order(TableHeaderCell::Order order) {
      if(auto previous_icon = layout()->takeAt(0)) {
        delete previous_icon->widget();
        delete previous_icon;
      }
      auto icon = [&] () -> Icon* {
        if(order == TableHeaderCell::Order::ASCENDING) {
          return new Icon(ASCENDING_IMAGE());
        } else if(order == TableHeaderCell::Order::DESCENDING) {
          return new Icon(DESCENDING_IMAGE());
        }
        return nullptr;
      }();
      if(icon) {
        icon->setFixedSize(scale(5, 6));
        update_style(*icon, [&] (auto& style) {
          style.get(Any()).set(BackgroundColor(Qt::transparent));
        });
        layout()->addWidget(icon);
      }
    }
  };
}

TableHeaderCell::TableHeaderCell(
    std::shared_ptr<CompositeValueModel<Model>> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)) {
  auto name_label = make_label(m_model->get(&Model::m_name));
  auto sort_indicator = new SortIndicator(m_model->get(&Model::m_order));
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(name_label);
  layout->addWidget(sort_indicator);
  auto style = StyleSheet();
  style.get(Any() > is_a<TextBox>()).set(TextColor(QColor(0x808080)));
  style.get(Hover() > is_a<TextBox>()).set(TextColor(QColor(0x4B23A0)));
  set_style(*this, std::move(style));
}

const std::shared_ptr<CompositeValueModel<TableHeaderCell::Model>>&
    TableHeaderCell::get_model() const {
  return m_model;
}

connection TableHeaderCell::connect_hide_signal(
    const HideSignal::slot_type& slot) const {
  return m_hide_signal.connect(slot);
}

connection TableHeaderCell::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

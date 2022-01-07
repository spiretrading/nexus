#include "Spire/Ui/TableBody.hpp"
#include <QHBoxLayout>
#include <QPainter>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

Spacing Spire::Styles::spacing(int spacing) {
  return Spacing(spacing, spacing);
}

GridColor Spire::Styles::grid_color(QColor color) {
  return GridColor(color, color);
}

QWidget* TableBody::default_view_builder(
    const std::shared_ptr<TableModel>& table, int row, int column) {
  return make_label(displayTextAny(table->at(row, column)));
}

TableBody::TableBody(std::shared_ptr<TableModel> table,
  std::shared_ptr<ListModel<int>> widths, QWidget* parent)
  : TableBody(
      std::move(table), std::make_shared<LocalValueModel<optional<Index>>>(),
      std::move(widths), &default_view_builder, parent) {}

TableBody::TableBody(std::shared_ptr<TableModel> table,
  std::shared_ptr<CurrentModel> current, std::shared_ptr<ListModel<int>> widths,
  QWidget* parent)
  : TableBody(std::move(table), std::move(current), std::move(widths),
      &default_view_builder, parent) {}

TableBody::TableBody(
  std::shared_ptr<TableModel> table, std::shared_ptr<ListModel<int>> widths,
  ViewBuilder view_builder, QWidget* parent)
  : TableBody(std::move(table),
      std::make_shared<LocalValueModel<optional<Index>>>(), std::move(widths),
      std::move(view_builder), parent) {}

TableBody::TableBody(
    std::shared_ptr<TableModel> table, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<ListModel<int>> widths, ViewBuilder view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_current(std::move(current)),
      m_widths(std::move(widths)),
      m_view_builder(std::move(view_builder)) {
  auto row_layout = new QVBoxLayout(this);
  row_layout->setContentsMargins({});
  row_layout->setSpacing(0);
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&TableBody::on_style, this));
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(HorizontalSpacing(scale_width(1))).
      set(VerticalSpacing(scale_width(1))).
      set(grid_color(QColor(0xE0E0E0)));
  });
  for(auto row = 0; row != m_table->get_row_size(); ++row) {
    on_table_operation(TableModel::AddOperation(row));
  }
  m_table_connection = m_table->connect_operation_signal(
    std::bind_front(&TableBody::on_table_operation, this));
  m_widths_connection = m_widths->connect_operation_signal(
    std::bind_front(&TableBody::on_widths_update, this));
}

const std::shared_ptr<TableModel>& TableBody::get_table() const {
  return m_table;
}

const std::shared_ptr<TableBody::CurrentModel>& TableBody::get_current() const {
  return m_current;
}

void TableBody::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(m_styles.m_vertical_spacing != 0 &&
      m_styles.m_horizontal_grid_color.alphaF() != 0) {
    auto draw_border = [&] (int top) {
      painter.save();
      painter.fillRect(QRect(0, top, width(), m_styles.m_vertical_spacing),
        m_styles.m_horizontal_grid_color);
      painter.restore();
    };
    for(auto row = 0; row != m_table->get_row_size(); ++row) {
      draw_border(
        layout()->itemAt(row)->geometry().top() - m_styles.m_vertical_spacing);
    }
    draw_border(height() - m_styles.m_vertical_spacing);
  }
  if(m_styles.m_horizontal_spacing != 0 &&
      m_styles.m_vertical_grid_color.alphaF() != 0) {
    auto draw_border = [&] (int left) {
      painter.save();
      painter.fillRect(QRect(left, 0, m_styles.m_horizontal_spacing, height()),
        m_styles.m_vertical_grid_color);
      painter.restore();
    };
    auto left = 0;
    for(auto column = 0; column != m_table->get_column_size(); ++column) {
      draw_border(left);
      if(column != m_widths->get_size()) {
        left += m_widths->get(column);
      }
    }
    draw_border(width() - m_styles.m_horizontal_spacing);
  }
  QWidget::paintEvent(event);
}

void TableBody::on_table_operation(const TableModel::Operation& operation) {
  auto& row_layout = *static_cast<QVBoxLayout*>(layout());
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      auto column_layout = new QHBoxLayout();
      column_layout->setContentsMargins({});
      column_layout->setSpacing(m_styles.m_horizontal_spacing);
      for(auto column = 0; column != m_table->get_column_size(); ++column) {
        auto item = m_view_builder(m_table, operation.m_index, column);
        if(column != m_table->get_column_size() - 1) {
          item->setSizePolicy(
            QSizePolicy::Fixed, item->sizePolicy().verticalPolicy());
          item->setFixedWidth(
            m_widths->get(column) - m_styles.m_horizontal_spacing);
        } else {
          item->setSizePolicy(
            QSizePolicy::Expanding, item->sizePolicy().verticalPolicy());
        }
        column_layout->addWidget(item);
      }
      row_layout.addLayout(column_layout);
    },
    [&] (const TableModel::RemoveOperation& operation) {
      auto& source_layout = *row_layout.itemAt(operation.m_index);
      row_layout.removeItem(&source_layout);
      while(auto item = source_layout.layout()->takeAt(0)) {
        delete item->widget();
        delete item;
      }
    },
    [&] (const TableModel::MoveOperation& operation) {
      auto& source_layout = *row_layout.itemAt(operation.m_source);
      row_layout.removeItem(&source_layout);
      auto destination = [&] {
        if(operation.m_source < operation.m_destination) {
          return operation.m_destination - 1;
        }
        return operation.m_destination;
      }();
      row_layout.insertItem(destination, &source_layout);
    });
}

void TableBody::on_widths_update(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::UpdateOperation& operation) {
      auto& row_layout = *layout();
      for(auto row = 0; row != row_layout.count(); ++row) {
        auto& column_layout = *row_layout.itemAt(row)->layout();
        auto& item = *column_layout.itemAt(operation.m_index)->widget();
        item.setFixedWidth(
          m_widths->get(operation.m_index) - m_styles.m_horizontal_spacing);
      }
    });
}

void TableBody::on_style() {
  auto& stylist = find_stylist(*this);
  m_styles = {};
  m_styles.m_horizontal_grid_color = Qt::transparent;
  m_styles.m_vertical_grid_color = Qt::transparent;
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const HorizontalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_styles.m_horizontal_spacing = spacing;
        });
      },
      [&] (const VerticalSpacing& spacing) {
        stylist.evaluate(spacing, [=] (auto spacing) {
          m_styles.m_vertical_spacing = spacing;
        });
      },
      [&] (const HorizontalGridColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_horizontal_grid_color = color;
        });
      },
      [&] (const VerticalGridColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_styles.m_vertical_grid_color = color;
        });
      });
  }
  auto& row_layout = *layout();
  for(auto row = 0; row != row_layout.count(); ++row) {
    row_layout.itemAt(row)->layout()->setSpacing(m_styles.m_horizontal_spacing);
  }
  row_layout.setSpacing(m_styles.m_vertical_spacing);
  row_layout.setContentsMargins({
    m_styles.m_horizontal_spacing, m_styles.m_vertical_spacing,
    m_styles.m_horizontal_spacing, m_styles.m_vertical_spacing});
  for(auto row = 0; row != row_layout.count(); ++row) {
    auto& column_layout = *row_layout.itemAt(row)->layout();
    for(auto column = 0; column != column_layout.count(); ++column) {
      auto& item = *column_layout.itemAt(column)->widget();
      item.setFixedWidth(m_widths->get(column) - m_styles.m_horizontal_spacing);
    }
  }
  update();
}

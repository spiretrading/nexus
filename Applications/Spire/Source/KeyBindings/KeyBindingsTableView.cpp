#include "Spire/KeyBindings/KeyBindingsTableView.hpp"
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include "Spire/KeyBindings/KeySequenceItemDelegate.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/IconButton.hpp"

using namespace Spire;

namespace {
  auto create_delete_button(QWidget* parent) {
    auto button_size = scale(8, 8);
    auto close_box = QRect(QPoint(0, 0), scale(8, 8));
    return new IconButton(
      imageFromSvg(":/Icons/close-purple.svg", button_size, close_box),
      imageFromSvg(":/Icons/close-red.svg", button_size, close_box), parent);
  }
}

KeyBindingsTableView::KeyBindingsTableView(QHeaderView* header,
    bool can_delete_rows, QWidget* parent)
    : ScrollArea(true, parent),
      m_header(header),
      m_can_delete_rows(can_delete_rows) {
  m_header->setParent(this);
  auto main_widget = new QWidget(this);
  auto layout = new QVBoxLayout(main_widget);
  if(m_can_delete_rows) {
    layout->setContentsMargins(scale_width(26), 0, 0, 0);
  } else {
    layout->setContentsMargins({});
  }
  layout->setSpacing(0);
  connect(m_header, &QHeaderView::sectionResized, this,
    &KeyBindingsTableView::on_header_resize);
  connect(m_header, &QHeaderView::sectionMoved, this,
    &KeyBindingsTableView::on_header_move);
  auto header_layout = new QVBoxLayout();
  header_layout->setContentsMargins({});
  header_layout->setSpacing(0);
  header_layout->addWidget(m_header);
  layout->addLayout(header_layout);
  m_table = new CustomGridTableView(this);
  layout->addWidget(m_table);
  m_table->setStyleSheet(QString(R"(
    QTableView {
      background-color: #FFFFFF;
      font-family: Roboto;
      font-size: %1px;
      gridline-color: #C8C8C8;
      outline: 0;
      padding-bottom: %3px;
      padding-left: %2px;
      padding-right: %2px;
    }

    QTableView::item {
      border: none;
      padding-left: %2px;
    }

    QTableView::item:selected {
      color: black;
    })").arg(scale_height(12)).arg(scale_width(8)).arg(scale_height(8)));
  m_table->setFrameShape(QFrame::NoFrame);
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
    &KeyBindingsTableView::on_horizontal_slider_value_changed);
  m_table->horizontalHeader()->hide();
  m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  m_table->horizontalHeader()->setStretchLastSection(
    m_header->stretchLastSection());
  m_table->verticalHeader()->setDefaultSectionSize(scale_height(26));
  m_table->verticalHeader()->hide();
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
  connect(m_table, &QTableView::clicked, this,
    &KeyBindingsTableView::on_table_clicked);
  setWidget(main_widget);
}

void KeyBindingsTableView::set_column_delegate(int column,
    QStyledItemDelegate* delegate) {
  m_table->setItemDelegateForColumn(column, delegate);
}

void KeyBindingsTableView::set_column_width(int column, int width) {
  m_header->resizeSection(column, width);
}

void KeyBindingsTableView::set_model(QAbstractTableModel* model) {
  m_header->setModel(model);
  auto old_model = m_table->model();
  auto old_selection_model = m_table->selectionModel();
  m_table->setModel(model);
  old_model->deleteLater();
  old_selection_model->deleteLater();
  if(m_can_delete_rows) {
    connect(model, &QAbstractItemModel::dataChanged,
      [=] (auto top_left, auto bottom_right) {
        update_delete_buttons();
      });
    update_delete_buttons();
  }
}

void KeyBindingsTableView::set_height(int height) {
  widget()->setFixedHeight(height);
  m_table->setFixedHeight(height);
}

void KeyBindingsTableView::set_width(int width) {
  widget()->setFixedWidth(width);
  if(m_can_delete_rows) {
    width -= scale_width(26);
  }
  m_header->setFixedWidth(width);
  m_table->setFixedWidth(width);
}

void KeyBindingsTableView::update_delete_buttons() {
  for(auto* button : m_delete_buttons) {
    button->deleteLater();
  }
  m_delete_buttons.clear();
  for(auto i = 0; i < m_table->model()->rowCount(); ++i) {
    auto button = create_delete_button(this);
    button->move(scale_width(12), i * scale_height(26) + scale_height(39));
    button->connect_clicked_signal([=] {
      on_delete_button_clicked(i);
    });
  }
}

void KeyBindingsTableView::on_delete_button_clicked(int index) {
  m_table->model()->removeRow(index);
}

void KeyBindingsTableView::on_header_resize(int index, int old_size,
    int new_size) {
  if(index == 8) {
    return;
  }
  m_table->horizontalHeader()->resizeSection(index,
     m_header->sectionSize(index));
  auto width = [&] {
    auto width = 0;
    for(auto i = 0; i < m_table->horizontalHeader()->count(); ++i) {
      width += m_header->sectionSize(i);
    }
    return width;
  }();
  set_width(max(width, scale_width(871)));
}

void KeyBindingsTableView::on_header_move(int logical_index, int old_index,
    int new_index) {
  m_table->horizontalHeader()->moveSection(old_index, new_index);
}

void KeyBindingsTableView::on_horizontal_slider_value_changed(int new_value) {
  if(new_value != 0) {
    m_header->move(widget()->pos().x(), m_header->pos().y());
  } else {
    m_header->move(0, m_header->pos().y());
  }
}

void KeyBindingsTableView::on_table_clicked(const QModelIndex& index) {
  if(index.flags().testFlag(Qt::ItemIsEditable)) {
    m_table->edit(index);
  }
}

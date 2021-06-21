#include "Spire/Ui/ListView.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

namespace {
  const auto DEFAULT_GAP = 0;
  const auto DEFAULT_OVERFLOW_GAP = DEFAULT_GAP;
}

ListView::ListView(std::shared_ptr<CurrentModel> current_model,
    std::shared_ptr<ListModel> list_model,
    std::function<QWidget* (std::shared_ptr<ListModel>, int index)> factory,
    QWidget* parent)
    : QWidget(parent),
      m_current_model(std::move(current_model)),
      m_list_model(std::move(list_model)),
      m_factory(std::move(factory)),
      m_direction(Qt::Vertical),
      m_navigation(EdgeNavigation::WRAP),
      m_overflow(Overflow::NONE),
      m_gap(5),
      m_overflow_gap(m_gap) {
  m_items.resize(m_list_model->get_size());
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    m_items[i] = m_factory(m_list_model, i);
  }
  update_layout();
}

const std::shared_ptr<ListView::CurrentModel>&
    ListView::get_current_model() const {
  return m_current_model;
}

const std::shared_ptr<ListModel>& ListView::get_list_model() const {
  return m_list_model;
}

Qt::Orientation ListView::get_direction() const {
  return m_direction;
}

void ListView::set_direction(Qt::Orientation direction) {
  m_direction = direction;
  update_layout();
}

ListView::EdgeNavigation ListView::get_edge_navigation() const {
  return EdgeNavigation();
}

void ListView::set_edge_navigation(EdgeNavigation navigation) {
  m_navigation = navigation;
}

ListView::Overflow ListView::get_overflow() const {
  return m_overflow;
}

void ListView::set_overflow(Overflow overflow) {
  m_overflow = overflow;
  update_layout();
}

connection ListView::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

connection ListView::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void ListView::resizeEvent(QResizeEvent* event) {
  update_layout();
}

void ListView::update_layout() {
  delete layout();
  if(m_items.empty()) {
    return;
  }
  if(m_direction == Qt::Vertical) {
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    if(m_overflow == Overflow::NONE) {
      auto column_layout = new QVBoxLayout();
      column_layout->setSpacing(0);
      column_layout->setContentsMargins({});
      column_layout->addWidget(m_items.front());
      for(auto i = m_items.begin() + 1; i != m_items.end(); ++i) {
        column_layout->addSpacing(m_gap);
        column_layout->addWidget(*i);
      }
      layout->addLayout(column_layout);
    } else {
      auto column_height = 0;
      auto column_layout = new QVBoxLayout();
      column_layout->setSpacing(0);
      column_layout->setContentsMargins({});
      column_layout->addWidget(m_items.front(), 0, Qt::AlignTop);
      column_height += m_items.front()->height();
      for(auto i = m_items.begin() + 1; i != m_items.end(); ++i) {
        column_height += (*i)->height() + m_gap;
        if(column_height <= height()) {
          column_layout->addSpacing(m_gap);
          column_layout->addWidget(*i, 0, Qt::AlignTop);
        } else {
          column_layout->addStretch();
          layout->addLayout(column_layout);
          layout->addSpacing(m_overflow_gap);
          column_layout = new QVBoxLayout();
          column_layout->setSpacing(0);
          column_layout->setContentsMargins({});
          column_layout->addWidget(*i, 0, Qt::AlignTop);
          column_height = (*i)->height();
        }
      }
      column_layout->addStretch();
      layout->addLayout(column_layout);
    }
  } else {
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    if(m_overflow == Overflow::NONE) {
      auto row_layout = new QHBoxLayout();
      row_layout->setSpacing(0);
      row_layout->setContentsMargins({});
      row_layout->addWidget(m_items.front(), 0, Qt::AlignTop);
      for(auto i = m_items.begin() + 1; i != m_items.end(); ++i) {
        row_layout->addSpacing(m_gap);
        row_layout->addWidget(*i, 0, Qt::AlignTop);
      }
      layout->addLayout(row_layout);
    } else {
      auto row_width = 0;
      auto row_layout = new QHBoxLayout();
      row_layout->setSpacing(0);
      row_layout->setContentsMargins({});
      row_layout->addWidget(m_items.front(), 0, Qt::AlignLeft | Qt::AlignTop);
      row_width += m_items.front()->width();
      for(auto i = m_items.begin() + 1; i != m_items.end(); ++i) {
        row_width += (*i)->width() + m_gap;
        if(row_width <= width()) {
          row_layout->addSpacing(m_gap);
          row_layout->addWidget(*i, 0, Qt::AlignLeft | Qt::AlignTop);
        } else {
          row_layout->addStretch();
          layout->addLayout(row_layout);
          layout->addSpacing(m_overflow_gap);
          row_layout = new QHBoxLayout();
          row_layout->setSpacing(0);
          row_layout->setContentsMargins({});
          row_layout->addWidget(*i, 0, Qt::AlignLeft | Qt::AlignTop);
          row_width = (*i)->width();
        }
      }
      row_layout->addStretch();
      layout->addLayout(row_layout);
    }
  }
  adjustSize();
}

#include "Spire/KeyBindings/TableRowDragDrop.hpp"
#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/SortedTableModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/TableView.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto DRAG_SCROLL_THRESHOLD = 50;

  struct AnyListToTableModel : TableModel {
    std::shared_ptr<AnyListModel> m_source;

    explicit AnyListToTableModel(std::shared_ptr<AnyListModel> source)
      : m_source(std::move(source)) {}

    int get_row_size() const override {
      return m_source->get_size();
    }

    int get_column_size() const override {
      return 0;
    }

    AnyRef at(int row, int column) const override {
      return AnyRef();
    }

    QValidator::State set(int row, int column, const std::any& value) override {
      return QValidator::State::Invalid;
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return connection();
    }
  };
}

class TableRowDragDrop::PreviewRow : public QWidget {
  public:
    explicit PreviewRow(QWidget* parent = nullptr)
        : QWidget(parent),
          m_grid_size(0) {
      m_box = new Box();
      enclose(*this, *m_box);
      proxy_style(*this, *m_box);
      update_style(*this, [] (auto& style) {
        style.get(Any()).
          set(BorderBottomColor(QColor(0x4B23A0))).
          set(BorderTopColor(QColor(0x4B23A0)));
      });
    }

    void set_pixmap(const QPixmap& pixmap) {
      m_pixmap = pixmap;
      resize(m_pixmap.size() + QSize(0, 2 * m_grid_size));
    }

    void set_grid_size(int size) {
      if(m_grid_size == size) {
        return;
      }
      m_grid_size = size;
      update_style(*this, [=] (auto& style) {
        style.get(Any()).
          set(BorderBottomSize(m_grid_size)).
          set(BorderTopSize(m_grid_size));
      });
      if(!m_pixmap.isNull()) {
        resize(m_pixmap.size() + QSize(0, 2 * m_grid_size));
      }
    }

    void paintEvent(QPaintEvent* event) override {
      if(!m_pixmap.isNull()) {
        QPainter painter(this);
        painter.setOpacity(0.8);
        painter.drawPixmap(0, 0, m_pixmap);
      }
    }

  private:
    QPixmap m_pixmap;
    Box* m_box;
    int m_grid_size;
};

class TableRowDragDrop::PaddingRow : public QWidget {
  public:
    explicit PaddingRow(QWidget* parent = nullptr)
        : QWidget(parent) {
      setAttribute(Qt::WA_TransparentForMouseEvents);
      auto box = new Box();
      enclose(*this, *box);
      proxy_style(*this, *box);
      update_style(*this, [] (auto& style) {
        style.get(Any()).
          set(BackgroundColor(QColor(0xFFFFFF)));
        style.get(Any() > DropUpIndicator()).
          set(BorderBottomSize(0)).
          set(BorderTopSize(scale_height(2))).
          set(BorderTopColor(QColor(0x4B23A0)));
        style.get(Any() > DropDownIndicator()).
          set(BorderTopSize(0)).
          set(BorderBottomSize(scale_height(2))).
          set(BorderBottomColor(QColor(0x4B23A0)));
        });
    }
};

TableRowDragDrop::TableRowDragDrop(std::shared_ptr<AnyListModel> model,
    std::shared_ptr<ListModel<std::shared_ptr<TableRow>>> rows,
    TableView& table_view)
    : m_model(std::move(model)),
      m_rows(std::move(rows)),
      m_table_view(&table_view),
      m_is_sorted(false),
      m_row_count(m_model->get_size()) {
  m_table_header = static_cast<TableHeader*>(static_cast<Box*>(
    m_table_view->layout()->itemAt(0)->widget())->get_body()->layout()->
      itemAt(0)->widget());
  m_table_header->connect_sort_signal(
    std::bind_front(&TableRowDragDrop::on_sort, this));
  m_scroll_box =
    static_cast<ScrollBox*>(m_table_view->layout()->itemAt(1)->widget());
  m_table_body = static_cast<TableBody*>(&m_scroll_box->get_body());
  m_table_body->setAcceptDrops(true);
  m_table_body->installEventFilter(this);
  m_sorted_model =
    std::static_pointer_cast<SortedTableModel>(m_table_body->get_table());
  m_preview_row = new PreviewRow(m_table_body);
  m_preview_row->hide();
  m_padding_row = new PaddingRow(m_table_body);
  m_padding_row->hide();
  for(auto i = 0; i < m_rows->get_size(); ++i) {
    if(m_rows->get(i)->is_draggable()) {
      m_rows->get(i)->get_grab_handle()->installEventFilter(this);
    }
  }
  m_operation_connection = m_sorted_model->connect_operation_signal(
    std::bind_front(&TableRowDragDrop::on_operation, this));
}

bool TableRowDragDrop::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_table_body) {
    if(event->type() == QEvent::DragEnter) {
      auto drag_event = *static_cast<QDragEnterEvent*>(event);
      if(!drag_event.mimeData()->data("row").isEmpty() &&
          m_table_body->isAncestorOf(
            static_cast<QWidget*>(drag_event.source()))) {
        event->accept();
      } else {
        event->ignore();
      }
    } else if(event->type() == QEvent::DragMove) {
      auto drag_event = *static_cast<QDragMoveEvent*>(event);
      if(drag_event.mimeData()->data("row").isEmpty()) {
        return QObject::eventFilter(watched, event);
      }
      auto y = [&] {
        auto y = drag_event.pos().y() - m_preview_row->height() / 2;
        if(y < 0) {
          return 0;
        }
        if(y + m_preview_row->height() > m_table_body->rect().bottom()) {
          return m_table_body->rect().bottom() - m_preview_row->height();
        }
        return y;
      }();
      auto x = m_table_body->layout()->contentsMargins().left();
      m_preview_row->move(x, y);
      auto global_pos = m_table_body->mapToGlobal(drag_event.pos());
      auto pos_in_scroll_box = m_scroll_box->mapFromGlobal(global_pos);
      if(pos_in_scroll_box.y() < DRAG_SCROLL_THRESHOLD) {
        scroll_line_up(m_scroll_box->get_vertical_scroll_bar(),
          DRAG_SCROLL_THRESHOLD - pos_in_scroll_box.y());
      } else if(pos_in_scroll_box.y() >
          m_scroll_box->rect().bottom() - DRAG_SCROLL_THRESHOLD) {
        scroll_line_down(m_scroll_box->get_vertical_scroll_bar(),
          pos_in_scroll_box.y() - m_scroll_box->rect().bottom() +
            DRAG_SCROLL_THRESHOLD);
      }
      auto drop_index = [&] {
        for(auto i = 0; i < m_rows->get_size(); ++i) {
          if(auto row = m_rows->get(i)->get_row();
              m_rows->get(i)->is_draggable() &&
              row->rect().contains(row->mapFromGlobal(global_pos))) {
            return i;
          }
        }
        return -1;
      }();
      if(drop_index >= 0 && m_drag_index && *m_drag_index != drop_index) {
        auto row = m_rows->get(drop_index)->get_row();
        move(*m_drag_index, drop_index);
        clear_sort_order();
        m_padding_row->setGeometry(x, row->y(), row->width(), row->height());
        if(drop_index > *m_drag_index) {
          match(*m_padding_row, DropDownIndicator());
          unmatch(*m_padding_row, DropUpIndicator());
        } else {
          match(*m_padding_row, DropUpIndicator());
          unmatch(*m_padding_row, DropDownIndicator());
        }
        m_drag_index = drop_index;
      }
    }
  } else {
    if(event->type() == QEvent::MouseButtonPress) {
      auto grab_handle = static_cast<QWidget*>(watched);
      auto mouse_event = *static_cast<QMouseEvent*>(event);
      m_press_pos = grab_handle->mapToGlobal(mouse_event.pos());
      m_grab_handle = grab_handle;
    } else if(event->type() == QEvent::MouseButtonRelease) {
      m_press_pos = none;
    } else if(event->type() == QEvent::MouseMove) {
      auto mouse_event = *static_cast<QMouseEvent*>(event);
      if(!m_press_pos || m_press_pos &&
          abs(m_grab_handle->mapToGlobal(mouse_event.pos()).y() -
            m_press_pos->y()) <= 4) {
        return QObject::eventFilter(watched, event);
      }
      m_drag_index = [&] () -> optional<int> {
        for(auto i = 0; i < m_rows->get_size(); ++i) {
          if(m_rows->get(i)->get_grab_handle() == m_grab_handle) {
            return i;
          }
        }
        return none;
      }();
      if(!m_drag_index) {
        return QObject::eventFilter(watched, event);
      }
      auto drag = new QDrag(m_grab_handle);
      auto mime = new QMimeData();
      mime->setData("row", QByteArray::number(*m_drag_index));
      drag->setMimeData(mime);
      start_drag(m_grab_handle->mapToGlobal(mouse_event.pos()));
      drag->exec();
      end_drag();
    }
  }
  return QObject::eventFilter(watched, event);
}

void TableRowDragDrop::move(int source, int destination) {
  if(m_model->get_size() == m_row_count) {
    if(m_is_sorted) {
      auto translated_model = TranslatedTableModel(m_sorted_model);
      translated_model.move(source, destination);
      m_model->transact([&] {
        for(auto i = 0; i < translated_model.get_row_size(); ++i) {
          if(m_rows->get(i)->is_draggable()) {
            m_model->move(m_rows->get(
              translated_model.get_translation_to_source(i))->get_row_index(),
                i);
          }
        }
      });
    } else {
      m_model->move(source, destination);
    }
  } else {
    if(m_is_sorted) {
      auto source_model_index_cache = std::vector<int>(m_rows->get_size());
      for(auto i = 0; i < m_rows->get_size(); ++i) {
        source_model_index_cache[i] = m_rows->get(i)->get_row_index();
      }
      auto get_min_source_index = [&] (int lower_bound) {
        auto min = m_model->get_size() - 1;
        for(auto i = 0; i < m_rows->get_size(); ++i) {
          if(m_rows->get(i)->is_draggable() &&
              m_rows->get(i)->get_row_index() > lower_bound &&
              min > m_rows->get(i)->get_row_index()) {
            min = m_rows->get(i)->get_row_index();
          }
        }
        return min;
      };
      auto translated_sorted_model = TranslatedTableModel(m_sorted_model);
      translated_sorted_model.move(source, destination);
      auto translated_source_model =
        TranslatedTableModel(std::make_shared<AnyListToTableModel>(m_model));
      m_model->transact([&] {
        auto last_destination= -1;
        for(auto i = 0; i < translated_sorted_model.get_row_size(); ++i) {
          if(m_rows->get(i)->is_draggable()) {
            auto source = translated_source_model.get_source_to_translation(
              source_model_index_cache[
                translated_sorted_model.get_translation_to_source(i)]);
            auto destination = get_min_source_index(last_destination);
            m_model->move(source, destination);
            translated_source_model.move(source, destination);
            last_destination = destination;
          }
        }
      });
    } else {
      m_model->move(m_rows->get(source)->get_row_index(),
        m_rows->get(destination)->get_row_index());
    }
  }
}

void TableRowDragDrop::clear_sort_order() {
  m_sorted_model->set_column_order({});
  if(m_is_sorted) {
    auto header_items = m_table_header->get_items();
    for(auto i = 0; i < header_items->get_size(); ++i) {
      if(auto item = header_items->get(i);
          item.m_order != TableHeaderItem::Order::UNORDERED &&
          item.m_order != TableHeaderItem::Order::NONE) {
        item.m_order = TableHeaderItem::Order::NONE;
        header_items->set(i, item);
      }
    }
    m_is_sorted = false;
  }
  if(auto current = m_table_body->get_current()->get()) {
    if(!m_rows->get(current->m_row)->is_filtered()) {
      m_rows->get(current->m_row)->set_out_of_range(false);
    }
  }
}

void TableRowDragDrop::start_drag(const QPoint& pos) {
  auto row = m_rows->get(*m_drag_index)->get_row();
  auto x = m_table_body->layout()->contentsMargins().left();
  m_padding_row->setGeometry(x, row->y(), row->width(), row->height());
  m_padding_row->show();
  m_padding_row->raise();
  auto pixmap = QPixmap(row->rect().size());
  pixmap.fill(QColor(0xFFFFFF));
  row->render(&pixmap, QPoint(), QRegion(row->rect()), QWidget::DrawChildren);
  m_preview_row->set_grid_size(m_table_body->layout()->spacing());
  m_preview_row->set_pixmap(pixmap);
  m_preview_row->move(x, m_table_body->mapFromGlobal(pos).y());
  m_preview_row->show();
  m_preview_row->raise();
  if(auto current = m_table_body->get_current()->get()) {
    match(*m_table_body->get_item(*current), DraggingCurrent());
  }
  if(pos.y() >= m_press_pos->y()) {
    match(*m_padding_row, DropUpIndicator());
    unmatch(*m_padding_row, DropDownIndicator());
  } else {
    unmatch(*m_padding_row, DropUpIndicator());
    match(*m_padding_row, DropDownIndicator());
  }
}

void TableRowDragDrop::end_drag() {
  if(auto current = m_table_body->get_current()->get()) {
    unmatch(*m_table_body->get_item(*current), DraggingCurrent());
    if(m_drag_index && current->m_row != *m_drag_index) {
      m_table_body->get_current()->set(TableView::Index(*m_drag_index, 1));
    }
  } else if(m_drag_index) {
    m_table_body->get_current()->set(TableView::Index(*m_drag_index, 1));
  }
  unmatch(*m_padding_row, DropUpIndicator());
  unmatch(*m_padding_row, DropDownIndicator());
  m_preview_row->set_pixmap(QPixmap(0, 0));
  m_preview_row->hide();
  m_padding_row->hide();
  m_press_pos = none;
  m_drag_index = none;
}

void TableRowDragDrop::on_operation(const TableModel::Operation& operation) {
  visit(operation,
    [&] (const TableModel::AddOperation& operation) {
      if(m_rows->get(operation.m_index)->is_draggable()) {
        m_rows->get(operation.m_index)->get_grab_handle()->
          installEventFilter(this);
      }
      ++m_row_count;
    },
    [&] (const TableModel::RemoveOperation& operation) {
      --m_row_count;
    });
}

void TableRowDragDrop::on_sort(int index, TableHeaderItem::Order order) {
  m_is_sorted = true;
}

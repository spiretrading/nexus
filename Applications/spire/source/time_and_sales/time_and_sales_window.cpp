#include "spire/time_and_sales/time_and_sales_window.hpp"
#include <QFileDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMovie>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTableView>
#include <QVBoxLayout>
#include "spire/security_input/security_input_dialog.hpp"
#include "spire/security_input/security_input_model.hpp"
#include "spire/time_and_sales/empty_time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_properties_dialog.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/spire/export_model.hpp"
#include "spire/ui/custom_qt_variants.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/item_padding_delegate.hpp"
#include "spire/ui/overlay_widget.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

time_and_sales_window::time_and_sales_window(
    const time_and_sales_properties& properties, 
    security_input_model& input_model, QWidget* parent)
    : QWidget(parent),
      m_input_model(&input_model),
      m_table(nullptr),
      m_v_scroll_bar_timer(this),
      m_h_scroll_bar_timer(this) {
  m_body = new QWidget(this);
  m_body->setMinimumSize(scale(40, 200));
  resize(scale_width(182), scale_height(452));
  m_body->setStyleSheet("background-color: #FFFFFF;");
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(m_body, this);
  setWindowTitle(tr("Time and Sales"));
  window->set_svg_icon(":/icons/time-sale-black.svg",
    ":/icons/time-sale-grey.svg");
  window_layout->addWidget(window);
  this->QWidget::window()->installEventFilter(this);
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto padding_widget = new QWidget(this);
  padding_widget->setFixedHeight(scale_height(4));
  padding_widget->setStyleSheet("background-color: #F5F5F5;");
  layout->addWidget(padding_widget);
  m_empty_window_label = new QLabel(tr("Enter a ticker symbol."), this);
  m_empty_window_label->setAlignment(Qt::AlignCenter);
  m_empty_window_label->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(11)));
  layout->addWidget(m_empty_window_label);
  create_table_with_container();
  layout->addWidget(m_table_container);
  m_table_container->hide();
  m_v_scroll_bar_timer.setInterval(500);
  connect(&m_v_scroll_bar_timer, &QTimer::timeout, this,
    &time_and_sales_window::fade_out_vertical_scroll_bar);
  m_h_scroll_bar_timer.setInterval(500);
  connect(&m_h_scroll_bar_timer, &QTimer::timeout, this,
    &time_and_sales_window::fade_out_horizontal_scroll_bar);
  m_volume_label = new QLabel(tr("Volume:"), this);
  m_volume_label->setFocusPolicy(Qt::NoFocus);
  m_volume_label->setFixedHeight(scale_height(20));
  m_volume_label->setStyleSheet(QString(R"(
    background-color: #F5F5F5;
    color: #333333;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;
    padding-left: %2px;)").arg(scale_height(10)).arg(scale_width(8)));
  layout->addWidget(m_volume_label);
  set_model(std::make_shared<empty_time_and_sales_model>(Security()));
  set_properties(properties);
}

void time_and_sales_window::set_model(
    std::shared_ptr<time_and_sales_model> model) {
  if(m_model.is_initialized()) {
    if(m_empty_window_label != nullptr) {
      delete m_empty_window_label;
      m_empty_window_label = nullptr;
    }
    m_table_container->show();
    QTimer::singleShot(1000, this, [=] { show_loading_widget(); });
  }
  model->connect_volume_signal([=] (const Quantity& v) { on_volume(v); });
  m_model.emplace(std::move(model), m_properties);
  connect(m_model.get_ptr(), &QAbstractTableModel::rowsAboutToBeInserted, this,
    &time_and_sales_window::on_rows_about_to_be_inserted);
  auto filter = new custom_variant_sort_filter_proxy_model(m_table);
  filter->setSourceModel(&m_model.get());
  m_table->setModel(filter);
}

const time_and_sales_properties&
    time_and_sales_window::get_properties() const {
  return m_properties;
}

void time_and_sales_window::set_properties(
    const time_and_sales_properties& properties) {
  m_properties = properties;
  m_model->set_properties(m_properties);
  if(m_table == nullptr) {
    return;
  }
  m_table->setShowGrid(m_properties.m_show_grid);
  QFontMetrics metrics(m_properties.m_font);
  auto row_height = metrics.height() + scale_height(2);
  m_table->verticalHeader()->setDefaultSectionSize(row_height);
  auto header_font = m_table->verticalHeader()->font();
  if(m_properties.m_font.pointSize() >= 11) {
    header_font.setPointSizeF(m_properties.m_font.pointSize() * 0.8);
  } else {
    header_font.setPointSize(9);
  }
  m_table->horizontalHeader()->setFont(header_font);
  QFontMetrics header_metrics(header_font);
  m_table->horizontalHeader()->setFixedHeight(header_metrics.height() * 1.8);
  m_table->viewport()->update();
}

connection time_and_sales_window::connect_security_change_signal(
    const change_security_signal::slot_type& slot) const {
  return m_change_security_signal.connect(slot);
}

connection time_and_sales_window::connect_closed_signal(
    const closed_signal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void time_and_sales_window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

void time_and_sales_window::contextMenuEvent(QContextMenuEvent* event) {
  auto contents = m_body->layout()->itemAt(1)->widget();
  QRect widget_geometry(contents->mapToGlobal(contents->geometry().topLeft()),
    contents->mapToGlobal(contents->geometry().bottomRight()));
  if(widget_geometry.contains(event->globalPos())) {
    QMenu context_menu(this);
    QAction properties_action(tr("Properties"), &context_menu);
    connect(&properties_action, &QAction::triggered, this,
      &time_and_sales_window::show_properties_dialog);
    context_menu.addAction(&properties_action);
    QAction export_action(tr("Export Table"), &context_menu);
    connect(&export_action, &QAction::triggered, this,
      &time_and_sales_window::export_table);
    export_action.setEnabled(m_table != nullptr);
    context_menu.addAction(&export_action);
    context_menu.setFixedWidth(scale_width(140));
    context_menu.setWindowFlag(Qt::NoDropShadowWindowHint);
    drop_shadow context_menu_shadow(true, true, &context_menu);
    context_menu.setStyleSheet(QString(R"(
      QMenu {
        background-color: #FFFFFF;
        border: %1px solid #A0A0A0 %2px solid #A0A0A0;
        color: #000000;
        font-family: Roboto;
        font-size: %3px;
        padding: %4px 0px;
      }

      QMenu::item {
        padding: %5px 0px %5px %6px;
      }

      QMenu::item:disabled,
      QMenu::item:disabled:selected,
      QMenu::item:disabled:hover {
        background-color: #FFFFFF;
        color: #C8C8C8;
      }

      QMenu::item:selected, QMenu::item:hover {
        background-color: #8D78EC;
        color: #FFFFFF;
      })")
      .arg(scale_height(1)).arg(scale_width(1))
      .arg(scale_height(12)).arg(scale_height(5))
      .arg(scale_height(3)).arg(scale_width(8)));
    context_menu.exec(event->globalPos());
  }
}

bool time_and_sales_window::eventFilter(QObject* watched, QEvent* event) {
  if(m_table != nullptr) {
    if(watched == m_table_container ||
        watched == m_table->horizontalHeader()->viewport()) {
      if(event->type() == QEvent::Wheel) {
        auto e = static_cast<QWheelEvent*>(event);
        if(e->modifiers() & Qt::ShiftModifier) {
          m_table_container->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
          m_table_container->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
          m_table_container->horizontalScrollBar()->setValue(
            m_table_container->horizontalScrollBar()->value() - e->delta() / 2);
          m_h_scroll_bar_timer.start();
        } else if(!within_h_scroll_bar(e->pos())) {
          m_table_container->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
          m_table_container->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
          m_table_container->verticalScrollBar()->setValue(
            m_table_container->verticalScrollBar()->value() - e->delta() / 2);
          m_v_scroll_bar_timer.start();
        }
      } else if(event->type() == QEvent::MouseMove) {
        auto e = static_cast<QMouseEvent*>(event);
        if(e->pos().x() > width() - m_table_container->verticalScrollBar()->width()) {
          m_table_container->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
          m_table_container->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        } else if(within_h_scroll_bar(e->pos())) {
          m_table_container->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
          m_table_container->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        } else {
          if(!m_v_scroll_bar_timer.isActive() &&
              m_table_container->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
            fade_out_vertical_scroll_bar();
          }
          if(!m_h_scroll_bar_timer.isActive() &&
              m_table_container->horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
            fade_out_horizontal_scroll_bar();
          }
        }
      } else if(event->type() == QEvent::HoverLeave) {
        if(m_table_container->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
          fade_out_vertical_scroll_bar();
        } else if(m_table_container->horizontalScrollBarPolicy() !=
            Qt::ScrollBarAlwaysOff) {
          fade_out_horizontal_scroll_bar();
        }
      }
    } else if(watched == m_table_container->verticalScrollBar()) {
      if(event->type() == QEvent::HoverLeave) {
        fade_out_vertical_scroll_bar();
      }
    } else if(watched == m_table_container->horizontalScrollBar()) {
      if(event->type() == QEvent::HoverLeave) {
        fade_out_horizontal_scroll_bar();
      }
    } else if(watched == window()) {
      if(event->type() == QEvent::Resize) {
        update_table_width();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void time_and_sales_window::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_PageUp) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_front(m_current_security);
      if(s != Security()) {
        set_current(s);
      }
    }
    return;
  } else if(event->key() == Qt::Key_PageDown) {
    if(m_current_security != Security()) {
      auto s = m_securities.push_back(m_current_security);
      if(s != Security()) {
        set_current(s);
      }
    }
    return;
  }
  auto pressed_key = event->text();
  if(pressed_key[0].isLetterOrNumber()) {
    auto dialog = new security_input_dialog(*m_input_model, pressed_key, this);
    dialog->setWindowModality(Qt::NonModal);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QDialog::accepted, this,
      [=] { on_security_input_accept(dialog); });
    connect(dialog, &QDialog::rejected, this,
      [=] { on_security_input_reject(dialog); });
    dialog->move(geometry().center().x() -
      dialog->width() / 2, geometry().center().y() - dialog->height() / 2);
    show_overlay_widget();
    dialog->show();
  }
}

void time_and_sales_window::create_table_with_container() {
  m_table_container = new QScrollArea(this);
  m_table_container->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  m_table_container->installEventFilter(this);
  m_table_container->setWidgetResizable(true);
  m_table_container->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table_container->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto container_contents = new QWidget(this);
  auto layout = new QVBoxLayout(container_contents);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_table = new QTableView(this);
  m_table->setItemDelegate(new item_padding_delegate(scale_width(5),
    new custom_variant_item_delegate(), this));
  m_table->setMouseTracking(true);
  m_table->setAttribute(Qt::WA_Hover);
  m_table->viewport()->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_table->setFocusPolicy(Qt::NoFocus);
  m_table->setSelectionMode(QAbstractItemView::NoSelection);
  m_table->horizontalHeader()->setMinimumSectionSize(scale_width(35));
  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->horizontalHeader()->viewport()->setMouseTracking(true);
  m_table->horizontalHeader()->viewport()->installEventFilter(this);
  m_table->horizontalHeader()->setSectionsClickable(false);
  m_table->horizontalHeader()->setSectionsMovable(true);
  m_table->horizontalHeader()->setFixedHeight(scale_height(26));
  m_table->horizontalHeader()->setDefaultAlignment(
    Qt::AlignLeft | Qt::AlignVCenter);
  m_table->verticalHeader()->setVisible(false);
  m_table_container->horizontalScrollBar()->installEventFilter(this);
  m_table_container->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  m_table_container->horizontalScrollBar()->setAttribute(Qt::WA_Hover);
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_table_container->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
  m_table_container->verticalScrollBar()->setAttribute(Qt::WA_Hover);
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_table->horizontalHeader()->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)));
  m_table->setStyleSheet(QString(R"(
    QTableView {
      background-color: red;
      border: none;
      gridline-color: #C8C8C8;
    }

    QScrollBar::horizontal {
      height: %1px;
    }

    QScrollBar::vertical {
      width: %2px;
    }

    QScrollBar::handle {
      background-color: #C8C8C8;
    }

    QScrollBar::handle:horizontal {
      min-width: %3px;
    }

    QScrollBar::handle:vertical {
      min-height: %4px;
    }

    QScrollBar::add-line, QScrollBar::sub-line,
    QScrollBar::add-page, QScrollBar::sub-page {
      background: none;
      border: none;
      height: 0px;
      width: 0px;
    })").arg(scale_height(12)).arg(scale_height(12))
        .arg(scale_width(30)).arg(scale_height(30)));
  layout->addWidget(m_table);
  layout->addStretch(1);
  auto test_widget = new QLabel("Thanks, Darryl...", this);
  test_widget->setFixedHeight(100);
  test_widget->setStyleSheet("background-color: #FFFFFF;");
  layout->addWidget(test_widget);
  m_table_container->setWidget(container_contents);
}

void time_and_sales_window::export_table() {
  show_overlay_widget();
  auto filepath = QFileDialog::getSaveFileName(this, tr("Export As"),
    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
    tr("/time_and_sales"), "CSV (*.csv)");
  if(!filepath.isNull()) {
    export_model_as_csv(m_model.get(), std::ofstream(filepath.toStdString()));
  }
  m_overlay_widget.reset();
}

void time_and_sales_window::fade_out_horizontal_scroll_bar() {
  m_h_scroll_bar_timer.stop();
  m_table_container->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void time_and_sales_window::fade_out_vertical_scroll_bar() {
  m_v_scroll_bar_timer.stop();
  m_table_container->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void time_and_sales_window::show_loading_widget() {
  if(m_model->rowCount(QModelIndex()) == 0 && m_loading_widget == nullptr) {
    auto backing_widget = new QLabel(m_body);
    auto logo = new QMovie(":/icons/pre-loader.gif", QByteArray(),
      m_body);
    logo->setScaledSize(scale(32, 32));
    backing_widget->setMovie(logo);
    backing_widget->setStyleSheet(
      QString("padding-top: %1px;").arg(scale_height(50)));
    backing_widget->setAlignment(Qt::AlignHCenter);
    backing_widget->movie()->start();
    m_loading_widget = std::make_unique<overlay_widget>(
      m_table_container->viewport(), backing_widget, m_body);
  }
}

void time_and_sales_window::show_overlay_widget() {
  auto contents = m_body->layout()->itemAt(1)->widget();
  m_overlay_widget = std::make_unique<QLabel>(m_body);
  m_overlay_widget->setStyleSheet(
    "background-color: rgba(245, 245, 245, 153);");
  m_overlay_widget->resize(contents->size());
  m_overlay_widget->move(contents->mapTo(contents, contents->pos()));
  m_overlay_widget->show();
}

void time_and_sales_window::show_properties_dialog() {
  time_and_sales_properties_dialog dialog(m_properties, this);
  dialog.connect_apply_signal([=] (auto p) { set_properties(p); });
  show_overlay_widget();
  if(dialog.exec() == QDialog::Accepted) {
    set_properties(dialog.get_properties());
  }
  m_overlay_widget.reset();
}

void time_and_sales_window::set_current(const Security& s) {
  if(s == m_current_security) {
    return;
  }
  m_current_security = s;
  m_change_security_signal(s);
  m_volume_label->setText(tr("Volume:"));
  setWindowTitle(QString::fromStdString(ToString(s)) +
    tr(" - Time and Sales"));
}

void time_and_sales_window::update_table_width() {
  if(m_table->width() > window()->width()) {
    auto width = 0;
    for(auto i = 0; i < m_table->horizontalHeader()->count(); ++i) {
      width += m_table->horizontalHeader()->sectionSize(i);
    }
    m_table->setFixedWidth(width);
  } else {
    m_table->setFixedWidth(window()->width());
  }
}

bool time_and_sales_window::within_h_scroll_bar(const QPoint& pos) {
  return pos.y() > m_table_container->height() - 
    m_table_container->horizontalScrollBar()->height();
}

void time_and_sales_window::on_rows_about_to_be_inserted(
    const QModelIndex& index, int start, int end) {
  m_loading_widget.reset();
  auto row_count = m_model->rowCount(QModelIndex());
  if(row_count > 0) {
    m_table->setFixedHeight(m_table->horizontalHeader()->height() +
      (m_table->rowHeight(0) * (row_count + 1)));
  } else {
    m_table->setFixedHeight(m_table->horizontalHeader()->height() * 2);
  }
  if(m_table_container->verticalScrollBar()->value() != 0) {
    m_table_container->verticalScrollBar()->setValue(
      m_table_container->verticalScrollBar()->value() + m_table->rowHeight(0));
  }
}

void time_and_sales_window::on_security_input_accept(
    security_input_dialog* dialog) {
  auto s = dialog->get_security();
  if(s != Security() && s != m_current_security) {
    m_securities.push(m_current_security);
    set_current(s);
    activateWindow();
  }
  dialog->close();
  m_overlay_widget.reset();
}

void time_and_sales_window::on_security_input_reject(
    security_input_dialog* dialog) {
  dialog->close();
  m_overlay_widget.reset();
}

void time_and_sales_window::on_volume(const Quantity& volume) {
  m_volume_label->setText(tr("Volume:").append(" ") +
    Beam::ToString(volume).c_str());
}

#include "Spire/Ui/TitleBar.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#ifdef Q_OS_WIN
  #include <qt_windows.h>
#endif
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(26, 22);
    return size;
  }

  auto BUTTON_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > Body()).set(BackgroundColor(QColor(0xF5F5F5)));
    style.get((Hover() || Press()) > Body()).
      set(BackgroundColor(QColor(0xE0E0E0)));
    style.get(Active() > is_a<Icon>()).set(Fill(QColor(0x333333)));
    style.get(!Active() > is_a<Icon>()).set(Fill(QColor(0xA0A0A0)));
    return style;
  }

  auto WINDOW_ICON_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).set(Fill(QColor(0x333333)));
    style.get(!Active()).set(Fill(QColor(0xA0A0A0)));
    return style;
  }

  void apply_title_label_style(StyleSheet& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(6))).
      set(FontSize(scale_height(11)));
    style.get(!Active()).set(TextColor(QColor(0xA0A0A0)));
  }

  void apply_close_button_style(StyleSheet& style) {
    style.get((Hover() || Press()) > Body()).
      set(BackgroundColor(QColor(0xE63F44)));
    style.get((Hover() || Press()) > is_a<Icon>()).
      set(Fill(QColor(0xFFFFFF)));
  }

  void apply_box_style(StyleSheet& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(PaddingLeft(scale_height(8)));
  }

  auto make_button(const QString& icon) {
    auto button = make_icon_button(imageFromSvg(icon, BUTTON_SIZE()));
    button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    button->setFixedSize(BUTTON_SIZE());
    set_style(*button, BUTTON_STYLE());
    return button;
  }
}

TitleBar::TitleBar(QImage icon, QWidget* parent)
    : QWidget(parent),
      m_window_icon(nullptr),
      m_is_dragging(false) {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_title_label = make_label("", this);
  m_title_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_title_label->installEventFilter(this);
  update_style(*m_title_label, apply_title_label_style);
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_container_layout = make_hbox_layout(body);
  m_container_layout->addWidget(m_title_label);
  m_minimize_button = make_button(":/Icons/minimize.svg");
  m_minimize_button->connect_click_signal(
    std::bind_front(&TitleBar::on_minimize_button_press, this));
  m_container_layout->addWidget(m_minimize_button);
  m_maximize_button = make_button(":/Icons/maximize.svg");
  m_maximize_button->connect_click_signal(
    std::bind_front(&TitleBar::on_maximize_button_press, this));
  m_container_layout->addWidget(m_maximize_button);
  m_restore_button = make_button(":/Icons/restore.svg");
  m_restore_button->connect_click_signal(
    std::bind_front(&TitleBar::on_restore_button_press, this));
  m_restore_button->hide();
  m_container_layout->addWidget(m_restore_button);
  m_close_button = make_button(":/Icons/close.svg");
  m_close_button->connect_click_signal(
    std::bind_front(&TitleBar::on_close_button_press, this));
  update_style(*m_close_button, apply_close_button_style);
  m_container_layout->addWidget(m_close_button);
  set_icon(icon);
  auto box = new Box(body);
  enclose(*this, *box);
  proxy_style(*this, *box);
  update_style(*this, apply_box_style);
  connect_window_signals();
}

void TitleBar::set_icon(const QImage& icon) {
  auto window_icon = new Icon(icon);
  window_icon->setFixedSize(scale(10, 10));
  set_style(*window_icon, WINDOW_ICON_STYLE());
  if(!m_window_icon) {
    m_container_layout->insertWidget(0, window_icon);
  } else {
    auto item = m_container_layout->replaceWidget(m_window_icon, window_icon,
      Qt::FindDirectChildrenOnly);
    delete item->widget();
    delete item;
  }
  m_window_icon = window_icon;
}

const TextBox& TitleBar::get_title_label() const {
  return *m_title_label;
}

void TitleBar::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ParentChange) {
    connect_window_signals();
  }
}

void TitleBar::mouseMoveEvent(QMouseEvent* event) {
  if(!m_is_dragging) {
    return;
  }
  auto delta = event->globalPos();
  delta -= m_last_pos;
  auto window_pos = window()->pos();
  window_pos += delta;
  m_last_pos = event->globalPos();
  window()->move(window_pos);
}

void TitleBar::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_pos = event->globalPos();
}

void TitleBar::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

bool TitleBar::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::WindowStateChange) {
      if(window()->isMaximized()) {
        m_maximize_button->hide();
        m_restore_button->show();
      } else {
        if(window()->windowFlags().testFlag(Qt::WindowMaximizeButtonHint)) {
          m_maximize_button->show();
        }
        m_restore_button->hide();
      }
    } else if(event->type() == QEvent::WinIdChange) {
      m_minimize_button->setVisible(
        window()->windowFlags().testFlag(Qt::WindowMinimizeButtonHint));
      m_maximize_button->setVisible(
        window()->windowFlags().testFlag(Qt::WindowMaximizeButtonHint));
      m_close_button->setVisible(
        window()->windowFlags().testFlag(Qt::WindowCloseButtonHint));
    }
  } else if(watched == m_title_label && event->type() == QEvent::Resize) {
    on_window_title_change(window()->windowTitle());
  }
  return QWidget::eventFilter(watched, event);
}

void TitleBar::connect_window_signals() {
  connect(window(), &QWidget::windowTitleChanged, this,
    std::bind_front(&TitleBar::on_window_title_change, this));
  window()->installEventFilter(this);
}

void TitleBar::on_window_title_change(const QString& title) {
  auto metrics = QFontMetrics(m_title_label->font());
  auto shortened_text =
    metrics.elidedText(title, Qt::ElideRight, m_title_label->width());
  m_title_label->get_current()->set(shortened_text);
}

void TitleBar::on_minimize_button_press() {
  window()->showMinimized();
}

void TitleBar::on_maximize_button_press() {
  window()->showMaximized();
}

void TitleBar::on_restore_button_press() {
  window()->showNormal();
}

void TitleBar::on_close_button_press() {
  window()->close();
}

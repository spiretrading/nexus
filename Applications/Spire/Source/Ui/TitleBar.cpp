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
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(32, 26);
    return size;
  }

  auto BUTTON_STYLE() {
    auto style = StyleSheet();
    style.get(Any() / Body()).
      set(BackgroundColor(QColor(0xF5F5F5)));
    style.get((Hover() || Press()) / Body()).
      set(BackgroundColor(QColor(0xE0E0E0)));
    style.get(Any() >> is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Active() >> is_a<Icon>()).
      set(Fill(QColor(Qt::black)));
    style.get(!Active() >> is_a<Icon>()).
      set(Fill(QColor(0xA0A0A0)));
    style.get(Hover() >> is_a<Icon>()).
      set(Fill(QColor(Qt::black)));
    return style;
  }

  auto WINDOW_BUTTON_STYLE() {
    auto style = StyleSheet();
    style.get(Any() / Body()).
      set(BackgroundColor(QColor(0xF5F5F5)));
    style.get(Any() >> is_a<Icon>()).
      set(Fill(QColor(Qt::black)));
    style.get(!Active() >> is_a<Icon>()).
      set(Fill(QColor(0xA0A0A0)));
    return style;
  }

  auto create_button(const QString& icon, QWidget* parent) {
    auto button = make_icon_button(imageFromSvg(icon, BUTTON_SIZE()), parent);
    button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    button->setFixedSize(BUTTON_SIZE());
    set_style(*button, BUTTON_STYLE());
    return button;
  }
}

TitleBar::TitleBar(QImage icon, QWidget* parent)
    : QWidget(parent),
      m_window_button(nullptr) {
  setFixedHeight(scale_height(26));
  auto container = new QWidget(this);
  m_container_layout = new QHBoxLayout(container);
  m_container_layout->setContentsMargins({});
  m_container_layout->setSpacing(0);
  m_title_label = make_label("", this);
  update_style(*m_title_label, [&] (auto& style) {
    style.get(ReadOnly() && Disabled()).set(BackgroundColor(QColor(0xF5F5F5)));
    style.get(!Active()).set(TextColor(QColor(0xA0A0A0)));
  });
  m_title_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_container_layout->addWidget(m_title_label);
  m_minimize_button = create_button(":/Icons/minimize.svg", this);
  m_minimize_button->connect_clicked_signal([=] {
    on_minimize_button_press();
  });
  m_container_layout->addWidget(m_minimize_button);
  m_maximize_button = create_button(":/Icons/maximize.svg", this);
  m_maximize_button->connect_clicked_signal([=] {
    on_maximize_button_press();
  });
  m_container_layout->addWidget(m_maximize_button);
  m_restore_button = create_button(":/Icons/restore.svg", this);
  m_restore_button->connect_clicked_signal([=] { on_restore_button_press(); });
  m_restore_button->hide();
  m_container_layout->addWidget(m_restore_button);
  m_close_button = make_icon_button(
    imageFromSvg(":/Icons/close.svg", BUTTON_SIZE()), parent);
  m_close_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_close_button->setFixedSize(BUTTON_SIZE());
  m_close_button->connect_clicked_signal([=] { on_close_button_press(); });
  auto close_button_style = BUTTON_STYLE();
  close_button_style.get((Hover() || Press()) / Body()).
    set(BackgroundColor(QColor(0xE63F44)));
  close_button_style.get((Hover() || Press()) >> is_a<Icon>()).
    set(Fill(QColor(0xFFFFFF)));
  set_style(*m_close_button, std::move(close_button_style));
  m_container_layout->addWidget(m_close_button);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(new Box(container));
  set_icon(icon);
  connect_window_signals();
}

void TitleBar::set_icon(const QImage& icon) {
  delete m_window_button;
  m_window_button = make_icon_button(icon, this);
  m_window_button->setFixedSize(scale(26, 26));
  m_window_button->setFocusPolicy(Qt::NoFocus);
  set_style(*m_window_button, WINDOW_BUTTON_STYLE());
  m_container_layout->insertWidget(0, m_window_button);
}

const TextBox& TitleBar::get_title_label() const {
  return *m_title_label;
}

void TitleBar::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ParentChange) {
    connect_window_signals();
  }
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
  }
  return QWidget::eventFilter(watched, event);
}

void TitleBar::resizeEvent(QResizeEvent* event) {
  on_window_title_change(window()->windowTitle());
}

void TitleBar::connect_window_signals() {
  connect(window(), &QWidget::windowTitleChanged,
    [=] (auto& title) {on_window_title_change(title);});
  window()->installEventFilter(this);
}

void TitleBar::on_window_title_change(const QString& title) {
  QFontMetrics metrics(m_title_label->font());
  auto shortened_text = metrics.elidedText(title,
    Qt::ElideRight, m_title_label->width());
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

#include "Spire/Ui/TitleBar.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QMouseEvent>
#ifdef Q_OS_WIN
  #include <qt_windows.h>
#endif
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/IconButton.hpp"
#include "Spire/Ui/Window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(32, 26);
    return size;
  }

  auto apply_button_style(IconButton* button) {
    button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    button->set_default_color("#333333");
    button->set_hover_color("#333333");
    button->set_blur_color("#D0D0D0");
    button->setFixedSize(BUTTON_SIZE());
  }

  auto create_button(const QString& icon, QWidget* parent) {
    auto button = new IconButton(imageFromSvg(icon, BUTTON_SIZE()), parent);
    apply_button_style(button);
    return button;
  }
}

TitleBar::TitleBar(const QImage& icon, QWidget* parent)
    : QWidget(parent) {
  setFixedHeight(scale_height(26));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_icon_button = new IconButton(m_icon, this);
  apply_button_style(m_icon_button);
  m_icon_button->set_hover_background_color(Qt::transparent);
  layout->addWidget(m_icon_button);
  m_title_label = new QLabel("", this);
  m_title_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
  set_title_text_stylesheet(QColor("#000000"));
  layout->addWidget(m_title_label);
  layout->addSpacerItem(new QSpacerItem(scale_width(8), 10, QSizePolicy::Fixed,
    QSizePolicy::Expanding));
  m_minimize_button = create_button(":/Icons/minimize.svg", this);
  m_minimize_button->connect_clicked_signal(
    [=] { on_minimize_button_press(); });
  layout->addWidget(m_minimize_button);
  m_maximize_button = create_button(":/Icons/maximize.svg", this);
  m_maximize_button->connect_clicked_signal(
    [=] { on_maximize_button_press(); });
  layout->addWidget(m_maximize_button);
  m_restore_button = create_button(":/Icons/restore.svg", this);
  m_restore_button->connect_clicked_signal([=] { on_restore_button_press(); });
  m_restore_button->hide();
  layout->addWidget(m_restore_button);
  m_close_button = create_button(":/Icons/close.svg", this);
  m_close_button->set_hover_color("#E63F44");
  m_close_button->connect_clicked_signal([=] { on_close_button_press(); });
  layout->addWidget(m_close_button);
  connect(window(), &QWidget::windowTitleChanged,
    [=] (auto& title) {on_window_title_change(title);});
}

void TitleBar::set_icon(const QImage& icon) {
  m_icon_button->set_icon(icon);
}

QLabel* TitleBar::get_title_label() const {
  return m_title_label;
}

void TitleBar::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ParentChange) {
    connect(window(), &QWidget::windowTitleChanged,
      [=] (auto& title) {on_window_title_change(title);});
  }
}

bool TitleBar::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::WindowDeactivate) {
      set_title_text_stylesheet(QColor("#A0A0A0"));
    } else if(event->type() == QEvent::WindowActivate) {
      set_title_text_stylesheet(QColor("#000000"));
    } else if(event->type() == QEvent::WindowStateChange) {
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

void TitleBar::on_window_title_change(const QString& title) {
  QFontMetrics metrics(m_title_label->font());
  auto shortened_text = metrics.elidedText(title,
    Qt::ElideRight, m_title_label->width());
  m_title_label->setText(shortened_text);
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

void TitleBar::set_title_text_stylesheet(const QColor& font_color) {
  m_title_label->setStyleSheet(QString(
    R"(color: %2;
       font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)).arg(font_color.name()));
}

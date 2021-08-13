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

  const auto& BUTTON_STYLE() {
    static auto style = [] {
      auto style = StyleSheet();
      style.get(Any()).set(Fill(QColor(0x333333)));
      style.get(!Focus()).set(Fill(QColor(0xD0D0D0)));
      return style;
    }();
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
      m_icon(std::move(icon)),
      m_icon_button(nullptr) {
  setObjectName("title_bar");
  setFixedHeight(scale_height(26));
  setStyleSheet("#title_bar { background-color: #F5F5F5; }");
  m_layout = new QHBoxLayout(this);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
  set_icon(m_icon);
  m_title_label = new QLabel("", this);
  m_title_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
  set_title_text_stylesheet(QColor("#000000"));
  m_layout->addWidget(m_title_label);
  m_layout->addSpacerItem(new QSpacerItem(
    scale_width(8), 10, QSizePolicy::Fixed, QSizePolicy::Expanding));
  m_minimize_button = create_button(":/Icons/minimize.svg", this);
  m_minimize_button->connect_clicked_signal([=] {
    on_minimize_button_press();
  });
  m_layout->addWidget(m_minimize_button);
  m_maximize_button = create_button(":/Icons/maximize.svg", this);
  m_maximize_button->connect_clicked_signal([=] {
    on_maximize_button_press();
  });
  m_layout->addWidget(m_maximize_button);
  m_restore_button = create_button(":/Icons/restore.svg", this);
  m_restore_button->connect_clicked_signal([=] { on_restore_button_press(); });
  m_restore_button->hide();
  m_layout->addWidget(m_restore_button);
  auto close_button_style = BUTTON_STYLE();
  close_button_style.get(Hover()).set(Fill(QColor(0xE63F44)));
  m_close_button = make_icon_button(
    imageFromSvg(":/Icons/close.svg", BUTTON_SIZE()), parent);
  m_close_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_close_button->setFixedSize(BUTTON_SIZE());
  m_close_button->connect_clicked_signal([=] { on_close_button_press(); });
  set_style(*m_close_button, std::move(close_button_style));
  m_layout->addWidget(m_close_button);
  connect_window_signals();
}

void TitleBar::set_icon(const QImage& icon) {
  m_icon = icon;
  set_icon(icon, "#333333");
}

void TitleBar::set_icon(const QImage& icon, const QColor& hover_color) {
  auto icon_button_style = BUTTON_STYLE();
  icon_button_style.get(Hover()).
    set(Fill(hover_color)).
    set(BackgroundColor(QColor(0, 0, 0, 0)));
  delete m_icon_button;
  m_icon_button = make_icon_button(icon, this);
  m_icon_button->setFixedSize(scale(26, 26));
  m_icon_button->setFocusPolicy(Qt::NoFocus);
  set_style(*m_icon_button, std::move(icon_button_style));
  m_layout->insertWidget(0, m_icon_button);
}

QLabel* TitleBar::get_title_label() const {
  return m_title_label;
}

void TitleBar::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ParentChange) {
    connect_window_signals();
  }
}

bool TitleBar::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::WindowDeactivate) {
      set_title_text_stylesheet(QColor(0xA0A0A0));
      set_icon(m_icon, "#D0D0D0");
    } else if(event->type() == QEvent::WindowActivate) {
      set_title_text_stylesheet(QColor(0x0u));
      set_icon(m_icon, "#333333");
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

void TitleBar::connect_window_signals() {
  connect(window(), &QWidget::windowTitleChanged,
    [=] (auto& title) {on_window_title_change(title);});
  window()->installEventFilter(this);
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
  m_title_label->setStyleSheet(QString(R"(
    QLabel {
      color: %2;
      font-family: Roboto;
      font-size: %1px;
    })").arg(scale_height(12)).arg(font_color.name()));
}

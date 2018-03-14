#include "spire/login/login_window.hpp"
#include <QHBoxLayout>
#include <QImage>
#include <QMovie>
#include <QPainter>
#include <QSize>
#include <QtSvg/QSvgRenderer>
#include <QVBoxLayout>
#include "spire/login/chroma_hash_widget.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/flat_button.hpp"
#include "spire/ui/icon_button.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

login_window::login_window(const std::string& version, QWidget* parent)
    : QWidget(parent),
      m_is_dragging(false) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setFixedSize(scale(384, 346));
  setStyleSheet("background-color: #4B23A0;");
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  auto title_bar_layout = new QHBoxLayout();
  title_bar_layout->setMargin(0);
  title_bar_layout->setSpacing(0);
  title_bar_layout->addStretch(1);
  m_exit_button = new icon_button(":/icons/close-purple.svg",
    ":/icons/close-red.svg", scale_width(32), scale_height(26),
    QRectF(scale_width(11), scale_height(8), scale_width(10), scale_height(10)),
    this);
  m_exit_button->installEventFilter(this);
  m_exit_button->connect_clicked_signal([&] {window()->close();});
  m_exit_button->setFixedSize(scale(32, 26));
  m_exit_button->setStyleSheet(":hover { background-color: #401D8B; }");
  title_bar_layout->addWidget(m_exit_button);
  layout->addLayout(title_bar_layout);
  layout->addStretch(30);
  auto logo_layout = new QHBoxLayout();
  logo_layout->setMargin(0);
  logo_layout->setSpacing(0);
  logo_layout->addStretch(1);
  m_logo_widget = new QLabel(parent);
  m_logo_widget->setFixedSize(scale(134, 50));
  auto logo = new QMovie(":/icons/logo.gif");
  logo->setScaledSize(m_logo_widget->size());
  m_logo_widget->setMovie(logo);
  logo->start();
  logo_layout->addWidget(m_logo_widget);
  logo_layout->addStretch(1);
  layout->addLayout(logo_layout);
  layout->addStretch(23);
  m_status_label = new QLabel(this);
  m_status_label->setStyleSheet(QString(
    R"(color: #FAEB96;
       font-family: Roboto;
       font-size: %1px;
       qproperty-alignment: AlignCenter;)").arg(scale_height(12)));
  layout->addWidget(m_status_label);
  layout->addStretch(20);
  auto username_layout = new QHBoxLayout();
  username_layout->setMargin(0);
  username_layout->setSpacing(0);
  username_layout->addStretch(1);
  m_username_line_edit = new QLineEdit(this);
  connect(m_username_line_edit, &QLineEdit::textEdited,
    [&] {on_input_updated();});
  m_username_line_edit->installEventFilter(this);
  m_username_line_edit->setPlaceholderText(tr("Username"));
  m_username_line_edit->setFixedSize(scale(280, 30));
  m_username_line_edit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       font-family: Roboto;
       font-size: %2px;
       padding-left: %1px;)")
    .arg(scale_width(10)).arg(scale_height(14)));
  username_layout->addWidget(m_username_line_edit);
  username_layout->addStretch(1);
  layout->addLayout(username_layout);
  layout->addStretch(15);
  auto password_layout = new QHBoxLayout();
  password_layout->setMargin(0);
  password_layout->setSpacing(0);
  password_layout->addStretch(1);
  m_password_line_edit = new QLineEdit(this);
  connect(m_password_line_edit, &QLineEdit::textEdited,
    [&] {on_input_updated();});
  connect(m_password_line_edit, &QLineEdit::textEdited,
    [&] {on_password_updated();});
  m_password_line_edit->installEventFilter(this);
  m_password_line_edit->setEchoMode(QLineEdit::Password);
  m_password_line_edit->setPlaceholderText(tr("Password"));
  m_password_line_edit->setFixedSize(scale(246, 30));
  m_password_line_edit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       font-family: Roboto;
       font-size: %2px;
       padding-left: %1px;)")
    .arg(scale_width(10)).arg(scale_height(14)));
  password_layout->addWidget(m_password_line_edit);
  auto ch_outer_widget = new QWidget(this);
  ch_outer_widget->setContentsMargins(scale_width(2), scale_height(2),
    scale_width(2), scale_height(2));
  ch_outer_widget->setFixedSize(scale(34, 30));
  ch_outer_widget->setStyleSheet("background-color: white;");
  auto ch_layout = new QHBoxLayout(ch_outer_widget);
  ch_layout->setMargin(0);
  m_chroma_hash_widget = new chroma_hash_widget(this);
  ch_layout->addWidget(m_chroma_hash_widget);
  password_layout->addWidget(ch_outer_widget);
  password_layout->addStretch(1);
  layout->addLayout(password_layout);
  layout->addStretch(30);
  auto button_layout = new QHBoxLayout();
  button_layout->setMargin(0);
  button_layout->setSpacing(0);
  button_layout->addStretch(52);
  auto build_label = new QLabel(QString(tr("Build ")) + version.c_str(), this);
  build_label->setStyleSheet(QString(
    R"(color: white;
       font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)));
  build_label->setFixedSize(scale(160, 30));
  button_layout->addWidget(build_label);
  m_sign_in_button = new flat_button(tr("Sign In"), this);
  m_sign_in_button->connect_clicked_signal([=] {try_login();});
  m_sign_in_button->installEventFilter(this);
  m_sign_in_button->setFixedSize(scale(120, 30));
  disable_button();
  button_layout->addWidget(m_sign_in_button);
  button_layout->addStretch(52);
  layout->addLayout(button_layout);
  layout->addStretch(48);
  setTabOrder(m_username_line_edit, m_password_line_edit);
  setTabOrder(m_password_line_edit, m_sign_in_button);
  set_state(state::NONE);
}

void login_window::set_state(state s) {
  switch(s) {
    case state::NONE: {
      reset_all();
      break;
    }
    case state::LOGGING_IN: {
      m_username_line_edit->setEnabled(false);
      m_password_line_edit->setEnabled(false);
      m_status_label->setText("");
      m_sign_in_button->set_text(tr("Cancel"));
      m_logo_widget->movie()->start();
      break;
    }
    case state::CANCELLING: {
      reset_all();
      s = state::NONE;
      break;
    }
    case state::INCORRECT_CREDENTIALS: {
      m_status_label->setText(tr("Incorrect username or password."));
      reset_visuals();
      break;
    }
    case state::SERVER_UNAVAILABLE: {
      m_status_label->setText(tr("Server is unavailable."));
      reset_visuals();
      break;
    }
  }
  m_state = s;
}

connection login_window::connect_login_signal(
    const login_signal::slot_type& slot) const {
  return m_login_signal.connect(slot);
}

connection login_window::connect_cancel_signal(
    const cancel_signal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

bool login_window::eventFilter(QObject* receiver, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    if(m_username_line_edit == receiver) {
      m_username_line_edit->setPlaceholderText("");
    }
    if(m_password_line_edit == receiver) {
      m_password_line_edit->setPlaceholderText("");
    }
    if(m_sign_in_button == receiver) {
      if(!m_username_line_edit->text().isEmpty()) {
        button_focused();
      }
    }
  } else if(event->type() == QEvent::FocusOut) {
    if(m_username_line_edit == receiver) {
      m_username_line_edit->setPlaceholderText(tr("Username"));
    }
    if(m_password_line_edit == receiver) {
      m_password_line_edit->setPlaceholderText(tr("Password"));
    }
    if(m_sign_in_button == receiver) {
      if(!m_username_line_edit->text().isEmpty()) {
        enable_button();
      } else {
        disable_button();
      }
    }
  }
  return QWidget::eventFilter(receiver, event);
}

void login_window::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    window()->close();
  } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(!m_username_line_edit->hasFocus()) {
      if(!m_username_line_edit->text().isEmpty()) {
        try_login();
      }
    }
  } else if(m_password_line_edit->hasFocus()) {
    return;
  } else if(!m_username_line_edit->hasFocus() &&
      m_username_line_edit->text().isEmpty()) {
    m_username_line_edit->setText(event->text());
    m_username_line_edit->setFocus();
  }
}

void login_window::mouseMoveEvent(QMouseEvent* event) {
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

void login_window::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_pos = event->globalPos();
}

void login_window::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void login_window::reset_all() {
  m_status_label->setText("");
  reset_visuals();
}

void login_window::reset_visuals() {
  m_username_line_edit->setEnabled(true);
  m_password_line_edit->setEnabled(true);
  m_sign_in_button->setFocus();
  m_sign_in_button->set_text(tr("Sign In"));
  m_logo_widget->movie()->stop();
  m_logo_widget->movie()->jumpToFrame(0);
}

void login_window::try_login() {
  if(m_state != state::LOGGING_IN) {
    if(m_username_line_edit->text().isEmpty()) {
      set_state(state::INCORRECT_CREDENTIALS);
    } else {
      m_login_signal(m_username_line_edit->text().toStdString(),
        m_password_line_edit->text().toStdString());
      set_state(state::LOGGING_IN);
    }
  } else {
    m_cancel_signal();
    set_state(state::CANCELLING);
  }
}

void login_window::enable_button() {
  m_sign_in_button->set_clickable(true);
  m_sign_in_button->setStyleSheet(QString(
    R"(QLabel {
         background-color: #684BC7;
         color: #E2E0FF;
         font-family: Roboto;
         font-size: %1px;
         font-weight: bold;
         qproperty-alignment: AlignCenter;
       }

       :hover {
         background-color: #8D78EC;
       })").arg(scale_height(14)));
}

void login_window::disable_button() {
  m_sign_in_button->set_clickable(false);
  m_sign_in_button->setStyleSheet(QString(
    R"(background-color: #4B23A0;
       color: #8D78EC;
       font-family: Roboto;
       font-size: %1px;
       font-weight: bold;
       qproperty-alignment: AlignCenter;)").arg(scale_height(14)));
}

void login_window::button_focused() {
  m_sign_in_button->setStyleSheet(
    m_sign_in_button->styleSheet() +
    R"(QLabel {
         border: 1px solid #8D78EC;
       })");
}

void login_window::on_input_updated() {
  if(!m_username_line_edit->text().isEmpty()) {
    enable_button();
  } else {
    disable_button();
  }
}

void login_window::on_password_updated() {
  m_chroma_hash_widget->set_text(m_password_line_edit->text());
}

#include "spire/login/login_window.hpp"
#include <QHBoxLayout>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QtSvg/QSvgRenderer>
#include <QVBoxlayout>
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

login_window::login_window(const std::string& version, QWidget* parent)
    : QWidget(parent),
      m_is_dragging(false) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  setFixedSize(scale(384, 346));
  setFocus();
  setStyleSheet("background-color: #4B23A0");
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  auto title_bar_layout = new QHBoxLayout();
  title_bar_layout->setMargin(0);
  title_bar_layout->setSpacing(0);
  title_bar_layout->addStretch(1);
  auto renderer = new QSvgRenderer(QString(":/icons/close-purple.svg"), this);
  auto default_icon = QImage(scale(32, 26), QImage::Format_ARGB32);
  QPainter p1(&default_icon);
  auto draw_rect = QRectF(scale_width(11), scale_height(8), scale_width(10),
    scale_height(10));
  default_icon.fill(QColor(0, 0, 0, 0));
  renderer->render(&p1, draw_rect);
  renderer->load(QString(":/icons/close-red.svg"));
  auto hover_icon = QImage(scale(32, 26), QImage::Format_ARGB32);
  QPainter p2(&hover_icon);
  hover_icon.fill(QColor(0, 0, 0, 0));
  renderer->render(&p2, draw_rect);
  m_exit_button = new icon_button(default_icon, hover_icon, this);
  m_exit_button->connect_clicked_signal([&] {close();});
  m_exit_button->setFixedSize(scale(32, 26));
  m_exit_button->setStyleSheet(":hover { background-color: #401D8B; }");
  title_bar_layout->addWidget(m_exit_button);
  layout->addLayout(title_bar_layout);
  layout->addStretch(30);
  auto logo_layout = new QHBoxLayout();
  logo_layout->setMargin(0);
  logo_layout->setSpacing(0);
  logo_layout->addStretch(1);
  m_logo_widget = new QSvgWidget(":/icons/logo.svg", this);
  m_logo_widget->setFixedSize(scale(134, 50));
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
  m_username_lineedit = new QLineEdit(this);
  connect(m_username_lineedit, &QLineEdit::textEdited, [&] { inputs_updated(); });
  m_username_lineedit->installEventFilter(this);
  m_username_lineedit->setPlaceholderText(tr("Username"));
  m_username_lineedit->setFixedHeight(scale_height(30));
  m_username_lineedit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       font-family: Roboto;
       font-size: %3px;
       margin: 0px %1px 0px %1px;
       padding-left: %2px;)")
    .arg(scale_width(52)).arg(scale_width(10)).arg(scale_height(14)));
  layout->addWidget(m_username_lineedit);
  layout->addStretch(15);
  m_password_lineedit = new QLineEdit(this);
  connect(m_password_lineedit, &QLineEdit::textEdited, [&] { inputs_updated(); });
  m_password_lineedit->installEventFilter(this);
  m_password_lineedit->setEchoMode(QLineEdit::Password);
  m_password_lineedit->setPlaceholderText(tr("Password"));
  m_password_lineedit->setFixedHeight(scale_height(30));
  m_password_lineedit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       font-family: Roboto;
       font-size: %3px;
       margin: 0px %1px 0px %1px;
       padding-left: %2px;)")
    .arg(scale_width(52)).arg(scale_width(10)).arg(scale_height(14)));
  layout->addWidget(m_password_lineedit);
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
  m_sign_in_button->connect_clicked_signal([=] {on_button_click();});
  m_sign_in_button->setFixedSize(scale(120, 30));
  disable_button();
  button_layout->addWidget(m_sign_in_button);
  button_layout->addStretch(52);
  layout->addLayout(button_layout);
  layout->addStretch(48);
  set_state(state::NONE);
  setFocus();
}

void login_window::set_state(state state) {
  switch(state) {
    case state::NONE: {
      reset_widget();
      m_state = state;
      break;
    }
    case state::LOGGING_IN: {
      m_status_label->setText("");
      m_sign_in_button->set_text("Cancel");
      m_logo_widget->load(QString(":/icons/login-preloader.svg"));
      m_state = state;
      break;
    }
    case state::CANCELLING: {
      reset_widget();
      state = state::NONE;
      break;
    }
    case state::INCORRECT_CREDENTIALS: {
      m_status_label->setText("Incorrect username or password.");
      reset_visuals();
      break;
    }
    case state::SERVER_UNAVAILABLE: {
      m_status_label->setText("Server is unavailable.");
      reset_visuals();
      break;
    }
  }
  m_state = state;
}

connection login_window::connect_login_signal(
    const login_signal::slot_type& slot) const {
  return m_login_signal.connect(slot);
}

connection login_window::connect_cancel_signal(
    const cancel_signal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

bool login_window::eventFilter(QObject* object, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    if(m_username_lineedit == object) {
      m_username_lineedit->setPlaceholderText("");
    }
    if(m_password_lineedit == object) {
      m_password_lineedit->setPlaceholderText("");
    }
  } else if(event->type() == QEvent::FocusOut) {
    if(m_username_lineedit == object) {
      m_username_lineedit->setPlaceholderText(tr("Username"));
    }
    if(m_password_lineedit == object) {
      m_password_lineedit->setPlaceholderText(tr("Password"));
    }
  }
  return QWidget::eventFilter(object, event);
}

void login_window::mouseMoveEvent(QMouseEvent* event) {
  if(!m_is_dragging) {
    return;
  }
  auto delta = event->globalPos();
  delta -= m_last_pos;
  auto window_pos = pos();
  window_pos += delta;
  m_last_pos = event->globalPos();
  move(window_pos);
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

void login_window::reset_widget() {
  m_status_label->setText("");
  reset_visuals();
}

void login_window::reset_visuals() {
  m_sign_in_button->set_text("Sign In");
  m_logo_widget->load(QString(":/icons/logo.svg"));
  setFocus();
}

void login_window::on_button_click() {
  if(m_state != state::LOGGING_IN) {
    if (m_username_lineedit->text() == "" || m_password_lineedit->text() == "") {
      set_state(state::INCORRECT_CREDENTIALS);
    } else {
      m_login_signal(m_username_lineedit->text().toStdString(),
        m_password_lineedit->text().toStdString());
      set_state(state::LOGGING_IN);
    }
  } else {
    m_cancel_signal();
    set_state(state::CANCELLING);
  }
}

void login_window::inputs_updated() {
  if(m_username_lineedit->text() != "") {
    enable_button();
  } else {
    disable_button();
  }
}

void login_window::enable_button() {
  m_sign_in_button->set_clickable(true);
  m_sign_in_button->setStyleSheet(QString(
    R"(QLabel {
         background-color: #684BC7;
         border: 1px solid #8D78EC;
         color: white;
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
       border: 1px solid #8D78EC;
       color: #8D78EC;
       font-family: Roboto;
       font-size: %1px;
       font-weight: bold;
       qproperty-alignment: AlignCenter;)").arg(scale_height(14)));
}

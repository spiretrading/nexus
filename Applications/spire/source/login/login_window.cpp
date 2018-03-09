#include "spire/login/login_window.hpp"
#include <QDebug>
#include <QHBoxLayout>
#include <QImage>
#include <QMovie>
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
  m_exit_button->installEventFilter(this);
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
  m_logo_widget = new QLabel(this);
  m_logo_widget->setStyleSheet("background-color: red");
  m_logo_widget->setFixedSize(scale(134, 50));
  auto logo = new QMovie(":/icons/animated-logo.gif");
  logo->setScaledSize(m_logo_widget->size());
  qDebug() << "Is valid: " << logo->isValid();
  //logo->setFileName(QString("://icons/animated-logo.gif"));
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
  m_username_lineedit = new QLineEdit(this);
  connect(m_username_lineedit, &QLineEdit::textEdited, [&] { inputs_updated(); });
  m_username_lineedit->installEventFilter(this);
  m_username_lineedit->setPlaceholderText(tr("Username"));
  m_username_lineedit->setFixedSize(scale(280, 30));
  m_username_lineedit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       font-family: Roboto;
       font-size: %2px;
       padding-left: %1px;)")
    .arg(scale_width(10)).arg(scale_height(14)));
  username_layout->addWidget(m_username_lineedit);
  username_layout->addStretch(1);
  layout->addLayout(username_layout);
  layout->addStretch(15);
  auto password_layout = new QHBoxLayout();
  password_layout->setMargin(0);
  password_layout->setSpacing(0);
  password_layout->addStretch(1);
  m_password_lineedit = new QLineEdit(this);
  connect(m_password_lineedit, &QLineEdit::textEdited, [&] { inputs_updated(); });
  connect(m_password_lineedit, &QLineEdit::textEdited, [&] {
    password_input_changed(); });
  m_password_lineedit->installEventFilter(this);
  m_password_lineedit->setEchoMode(QLineEdit::Password);
  m_password_lineedit->setPlaceholderText(tr("Password"));
  m_password_lineedit->setFixedSize(scale(246, 30));
  m_password_lineedit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       font-family: Roboto;
       font-size: %2px;
       padding-left: %1px;)")
    .arg(scale_width(10)).arg(scale_height(14)));
  password_layout->addWidget(m_password_lineedit);
  auto ch_outer_widget = new QWidget(this);
  ch_outer_widget->setContentsMargins(scale_width(2), scale_height(2),
    scale_width(2), scale_height(2));
  ch_outer_widget->setFixedSize(scale(34, 30));
  ch_outer_widget->setStyleSheet("background-color: white;");
  auto ch_layout = new QHBoxLayout(ch_outer_widget);
  ch_layout->setMargin(0);
  ch_layout->addWidget(ch_outer_widget);
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
  m_sign_in_button->connect_clicked_signal([=] {on_button_click();});
  m_sign_in_button->installEventFilter(this);
  m_sign_in_button->setFixedSize(scale(120, 30));
  disable_button();
  button_layout->addWidget(m_sign_in_button);
  button_layout->addStretch(52);
  layout->addLayout(button_layout);
  layout->addStretch(48);
  setTabOrder(m_username_lineedit, m_password_lineedit);
  setTabOrder(m_password_lineedit, m_sign_in_button);
  set_state(state::NONE);
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
      m_logo_widget->movie()->start();
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
    if(m_sign_in_button == object) {
      if(m_username_lineedit->text() != "") {
        button_focused();
      }
    }
  } else if(event->type() == QEvent::FocusOut) {
    if(m_username_lineedit == object) {
      m_username_lineedit->setPlaceholderText(tr("Username"));
    }
    if(m_password_lineedit == object) {
      m_password_lineedit->setPlaceholderText(tr("Password"));
    }
    if(m_sign_in_button == object) {
      if(m_username_lineedit->text() != "") {
        enable_button();
      } else {
        disable_button();
      }
    }
  } else if(event->type() == QEvent::MouseMove) {
    if(m_sign_in_button == object && m_username_lineedit->text() != "") {
      if(!rect().contains(static_cast<QMouseEvent*>(event)->
          localPos().toPoint())) {
        disable_button_hover();
      }
    } else if(m_exit_button == object) {
      if(!rect().contains(static_cast<QMouseEvent*>(event)->
          localPos().toPoint())) {
        m_exit_button->
      }
    }
  } else if(event->type() == QEvent::Enter) {
    if(m_sign_in_button == object && m_username_lineedit->text() != "") {
      enable_button();
    }
  }
  return QWidget::eventFilter(object, event);
}

void login_window::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    close();
  } else if(m_password_lineedit->hasFocus()) {
    return;
  } else if(!m_username_lineedit->hasFocus()) {
    m_username_lineedit->setText(event->text());
    m_username_lineedit->setFocus();
  }
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
  m_logo_widget->movie()->stop();
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

void login_window::password_input_changed() {
  m_chroma_hash_widget->set_text(m_password_lineedit->text());
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
       border: 1px solid #684BC7;
       color: #8D78EC;
       font-family: Roboto;
       font-size: %1px;
       font-weight: bold;
       qproperty-alignment: AlignCenter;)").arg(scale_height(14)));
}

void login_window::disable_button_hover() {
  m_sign_in_button->setStyleSheet(QString(
    R"(QLabel {
         background-color: #684BC7;
         color: #E2E0FF;
         font-family: Roboto;
         font-size: %1px;
         font-weight: bold;
         qproperty-alignment: AlignCenter;
       })").arg(scale_height(14)));
}

void login_window::button_focused() {
  m_sign_in_button->setStyleSheet(
    m_sign_in_button->styleSheet() + "QLabel { border: 1px solid #8D78EC; } ");
}

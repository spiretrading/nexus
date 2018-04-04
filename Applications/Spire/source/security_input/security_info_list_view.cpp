#include "spire/security_input/security_info_list_view.hpp"
#include <QEvent>
#include <QFocusEvent>
#include <QVBoxLayout>
#include "Nexus/Definitions/Country.hpp"
#include "spire/security_input/security_info_widget.hpp"
#include "spire/spire/dimensions.hpp"





#include <QDebug>
#include <QCoreApplication>





using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace spire;

security_info_list_view::security_info_list_view(QWidget* key_widget,
    QWidget* parent)
    : QScrollArea(parent),
      m_key_widget(key_widget),
      m_current_index(-1),
      m_hover_index(-1) {
  setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setFixedWidth(scale_width(180));
  setMaximumHeight(scale_height(200));
  setWidgetResizable(true);
  setObjectName("security_info_list_view_scrollbar");
  setFrameShape(QFrame::NoFrame);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setStyleSheet(QString(R"(
    #security_info_list_view_scrollbar {
      background-color: #FFFFFF;
      border-bottom: 1px solid #A0A0A0;
      border-left: 1px solid #A0A0A0;
      border-right: 1px solid #A0A0A0;
      border-top: none;
    }
    
    QScrollBar {
      background-color: #FFFFFF;
      border: none;
      width: %1px;
    }

    QScrollBar::handle:vertical {
      background-color: #EBEBEB;
    }

    QScrollBar::sub-line:vertical {
      border: none;
      background: none;
    }

    QScrollBar::add-line:vertical {
      border: none;
      background: none;
    })").arg(scale_height(12)));
  m_list_widget = new QWidget(this);
  auto layout = new QVBoxLayout(m_list_widget);
  layout->setMargin(0);
  layout->setSpacing(0);
  m_list_widget->setStyleSheet("background-color: #FFFFFF;");
  setWidget(m_list_widget);
  m_key_widget->installEventFilter(this);
}

void security_info_list_view::set_list(const std::vector<SecurityInfo>& list) {
  QLayoutItem* item;
  while((item = m_list_widget->layout()->takeAt(0)) != nullptr) {
    delete item->widget();
    delete item;
  }
  for(auto& security : list) {
    auto icon_path = QString(":/icons/%1.png").arg(
      security.m_security.GetCountry());
    auto security_widget = new security_info_widget(security, icon_path, this);
    security_widget->connect_commit_signal(
      [=] (const Security& s) { commit(s); });
    security_widget->connect_hovered_signal(
      [=] (QWidget* w) { update_hover_index(w); });
    m_list_widget->layout()->addWidget(security_widget);
  }
  if(list.size() > 5) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  } else {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }
  resize(widget()->width(), list.size() * scale_height(40));
  m_current_index = -1;
}

connection security_info_list_view::connect_clicked_signal(
    const selected_signal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

connection security_info_list_view::connect_highlighted_signal(
    const selected_signal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

bool security_info_list_view::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_key_widget) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Down) {
        highlight_next_item();
        return true;
      } else if(e->key() == Qt::Key_Up) {
        highlight_previous_item();
        return true;
      }
    }
  }
  return false;
}

void security_info_list_view::leaveEvent(QEvent* event) {
  m_current_index = -1;
  m_hover_index = -1;
}

void security_info_list_view::highlight_next_item() {
  if(m_list_widget->layout()->itemAt(m_current_index + 1) != nullptr) {
    if(m_list_widget->layout()->itemAt(m_current_index) != nullptr) {
      //m_list_widget->layout()->itemAt(m_current_index)->widget()->clearFocus();
      QCoreApplication::postEvent(
        m_list_widget->layout()->itemAt(m_current_index)->widget(),
        new QFocusEvent(QEvent::FocusOut));
    }
    //m_list_widget->layout()->itemAt(++m_current_index)->widget()->setFocus();
    QCoreApplication::postEvent(
      m_list_widget->layout()->itemAt(++m_current_index)->widget(),
      new QFocusEvent(QEvent::FocusIn));
    auto security = static_cast<security_info_widget*>(
      m_list_widget->layout()->itemAt(m_current_index)->widget())->
        get_security();
    m_highlighted_signal(security);
    if(m_current_index > M_MAX_VISIBLE_ITEMS - 1) {
      ensureWidgetVisible(
        m_list_widget->layout()->itemAt(m_current_index)->widget(), 0, 0);
    }
  }
}

void security_info_list_view::highlight_previous_item() {
  if(m_list_widget->layout()->itemAt(m_current_index - 1) != nullptr) {
    //m_list_widget->layout()->itemAt(m_current_index)->widget()->clearFocus();
    QCoreApplication::postEvent(
      m_list_widget->layout()->itemAt(m_current_index)->widget(),
      new QFocusEvent(QEvent::FocusOut));
    //m_list_widget->layout()->itemAt(++m_current_index)->widget()->setFocus();
    QCoreApplication::postEvent(
      m_list_widget->layout()->itemAt(--m_current_index)->widget(),
      new QFocusEvent(QEvent::FocusIn));
    auto security = static_cast<security_info_widget*>(
      m_list_widget->layout()->itemAt(m_current_index)->widget())->
        get_security();
    m_highlighted_signal(security);
    if(m_current_index < M_MAX_VISIBLE_ITEMS - 1) {
      ensureWidgetVisible(
        m_list_widget->layout()->itemAt(m_current_index)->widget(), 0, 0);
    }
  }
}

void security_info_list_view::commit(
    const Nexus::Security& security) {
  m_commit_signal(security);
}

void security_info_list_view::update_hover_index(QWidget* widget) {
  m_hover_index = m_list_widget->layout()->indexOf(widget);
  if(m_current_index != -1) {
    QCoreApplication::postEvent(
      m_list_widget->layout()->itemAt(m_current_index)->widget(),
      new QFocusEvent(QEvent::FocusOut));
  }
  m_current_index = m_hover_index;
}

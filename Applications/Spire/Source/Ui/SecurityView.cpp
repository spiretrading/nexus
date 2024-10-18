#include "Spire/Ui/SecurityView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SecurityDialog.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

SecurityView::SecurityView(std::shared_ptr<ComboBox::QueryModel> securities,
  QWidget& body, QWidget* parent)
  : SecurityView(std::move(securities), std::make_shared<LocalSecurityModel>(),
      body, parent) {}

SecurityView::SecurityView(std::shared_ptr<ComboBox::QueryModel> securities,
    std::shared_ptr<CurrentModel> current, QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_security_dialog(std::move(securities), this),
      m_current(std::move(current)),
      m_body(&body) {
  setFocusPolicy(Qt::StrongFocus);
  auto prompt = make_label(tr("Enter a ticker symbol."));
  update_style(*prompt, [] (auto& style) {
    style.get(ReadOnly() && Disabled()).
      set(TextAlign(Qt::AlignCenter)).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  m_layers = new QStackedWidget();
  m_layers->addWidget(prompt);
  m_layers->addWidget(m_body);
  enclose(*this, *m_layers);
  m_current_connection = m_current->connect_update_signal(
    std::bind_front(&SecurityView::on_current, this));
  m_security_dialog.connect_submit_signal(
    std::bind_front(&SecurityView::on_submit, this));
  if(m_current->get() != Security()) {
    on_current(m_current->get());
  }
}

const std::shared_ptr<ComboBox::QueryModel>&
    SecurityView::get_securities() const {
  return m_security_dialog.get_securities();
}

const std::shared_ptr<SecurityView::CurrentModel>&
    SecurityView::get_current() const {
  return m_current;
}

const QWidget& SecurityView::get_body() const {
  return *m_body;
}

QWidget& SecurityView::get_body() {
  return *m_body;
}

SecurityView::State SecurityView::save_state() const {
  return State(m_securities);
}

void SecurityView::restore(const State& state) {
  m_securities = state.m_securities;
  if(auto top = m_securities.get_top()) {
    if(*top != m_current->get()) {
      auto blocker = shared_connection_block(m_current_connection);
      m_current->set(*top);
      if(m_layers->currentWidget() != m_body) {
        m_layers->setCurrentWidget(m_body);
      }
    }
  }
}

void SecurityView::keyPressEvent(QKeyEvent* event) {
  if(auto text = event->text();
      text.size() == 1 && (text[0].isLetterOrNumber() || text[0] == '_')) {
    m_security_dialog.show();
    QApplication::sendEvent(find_focus_proxy(m_security_dialog), event);
  } else if(event->key() == Qt::Key_PageUp) {
    if(auto security = m_securities.rotate_top()) {
      m_current->set(*security);
    }
  } else if(event->key() == Qt::Key_PageDown) {
    if(auto security = m_securities.rotate_bottom()) {
      m_current->set(*security);
    }
  } else {
    QWidget::keyPressEvent(event);
  }
}

void SecurityView::on_current(const Security& security) {
  if(security == Security()) {
    return;
  }
  m_security_dialog.hide();
  m_securities.add(security);
  if(m_layers->currentWidget() != m_body) {
    m_layers->setCurrentWidget(m_body);
  }
}

void SecurityView::on_submit(const Security& security) {
  m_current->set(security);
}

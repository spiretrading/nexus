#include "Spire/Ui/SecurityView.hpp"
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/SecurityDialog.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

SecurityView::SecurityView(std::shared_ptr<SecurityInfoQueryModel> securities,
  QWidget& body, QWidget* parent)
  : SecurityView(std::move(securities), std::make_shared<LocalSecurityModel>(),
      body, parent) {}

SecurityView::SecurityView(std::shared_ptr<SecurityInfoQueryModel> securities,
    std::shared_ptr<CurrentModel> current, QWidget& body, QWidget* parent)
    : QWidget(parent),
      m_security_dialog(std::move(securities), this),
      m_current(std::move(current)),
      m_body(&body),
      m_current_index(-1) {
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
  m_security_dialog.connect_submit_signal(
    std::bind_front(&SecurityView::on_submit, this));
}

const std::shared_ptr<SecurityInfoQueryModel>&
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

void SecurityView::keyPressEvent(QKeyEvent* event) {
  if(event->modifiers() &
      (Qt::ControlModifier | Qt::AltModifier  | Qt::MetaModifier)) {
    QWidget::keyPressEvent(event);
    return;
  }
  if(auto text = event->text();
      text.size() == 1 && (text[0].isLetterOrNumber() || text[0] == '_')) {
    m_security_dialog.show();
    QApplication::sendEvent(find_focus_proxy(m_security_dialog), event);
  } else if(event->key() == Qt::Key_PageUp && !m_securities.empty()) {
    m_current_index =
      (m_securities.size() + m_current_index - 1) % m_securities.size();
    m_current->set(m_securities[m_current_index]);
  } else if(event->key() == Qt::Key_PageDown && !m_securities.empty()) {
    m_current_index = (m_current_index + 1) % m_securities.size();
    m_current->set(m_securities[m_current_index]);
  }
  QWidget::keyPressEvent(event);
}

void SecurityView::on_submit(const Security& security) {
  if(auto i = std::find(m_securities.begin(), m_securities.end(), security);
      i != m_securities.end()) {
    if(std::distance(m_securities.begin(), i) <= m_current_index) {
      --m_current_index;
    }
    m_securities.erase(i);
  }
  ++m_current_index;
  m_securities.insert(m_securities.begin() + m_current_index, security);
  m_current->set(security);
  m_security_dialog.hide();
  if(!m_securities.empty() && m_layers->currentWidget() != m_body) {
    m_layers->setCurrentWidget(m_body);
  }
}

#include "Spire/SecurityInput/SecurityInputBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/SecurityInput/SecurityInfoListView.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityInputBox::SecurityInputBox(Ref<SecurityInputModel> model,
    QWidget* parent)
    : SecurityInputBox(Ref(model), "", parent) {}

SecurityInputBox::SecurityInputBox(Ref<SecurityInputModel> model,
    const QString& initial_text, QWidget* parent)
    : QWidget(parent) {
  parent->installEventFilter(this);
  setObjectName("SecurityInputBox");
  setStyleSheet(QString(R"(
    #SecurityInputBox {
      border: %1px solid #C8C8C8;
    }
    :hover {
      border: %1px solid #4b23A0;
    })").arg(scale_width(1)));
  auto layout = new QHBoxLayout(this);
  layout->setMargin(scale_width(1));
  layout->setSpacing(0);
  m_security_line_edit = new SecurityInputLineEdit(initial_text, model, true,
    this);
  m_security_line_edit->connect_commit_signal([=] (auto& s) { on_commit(s); });
  m_security_line_edit->installEventFilter(this);
  layout->addWidget(m_security_line_edit);
  setFocusProxy(m_security_line_edit);
}

connection SecurityInputBox::connect_commit_signal(
    const CommitSignal::slot_type& slot) const {
  return m_commit_signal.connect(slot);
}

bool SecurityInputBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_security_line_edit) {
    if(event->type() == QEvent::FocusIn) {
      setStyleSheet(QString(R"(
        #SecurityInputBox {
          border: %1px solid #4b23A0;
        }
        #SecurityInputBox:hover {
          border: %1px solid #4b23A0;
        })").arg(scale_width(1)));
    } else if(event->type() == QEvent::FocusOut) {
      setStyleSheet(QString(R"(
        #SecurityInputBox {
          border: %1px solid #C8C8C8;
        }
        #SecurityInputBox:hover {
          border: %1px solid #4b23A0;
        })").arg(scale_width(1)));
    }
  }
  return QWidget::eventFilter(watched, event);
}

void SecurityInputBox::on_commit(const Security& security) {
  m_commit_signal(security);
}

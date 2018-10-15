#include "Spire/UI/ValueLabel.hpp"

using namespace Beam;
using namespace std;
using namespace Spire;
using namespace Spire::UI;

ValueLabel::ValueLabel(QWidget* parent, Qt::WindowFlags flags)
    : QLabel(parent, flags) {}

ValueLabel::ValueLabel(const string& label, Ref<UserProfile> userProfile) {
  Initialize(label, Ref(userProfile));
}

void ValueLabel::Initialize(const string& label,
    Ref<UserProfile> userProfile) {
  m_label = label;
  m_converter.Initialize(Ref(userProfile));
  QSizePolicy sizePolicy(QSizePolicy::Maximum, sizePolicy().verticalPolicy());
  sizePolicy.setHorizontalStretch(1);
  setSizePolicy(sizePolicy);
  QString text;
  if(!m_label.empty()) {
    text = "<span style=\"font-weight: bold;\">" +
      QString::fromStdString(m_label) + ":</span> ";
  }
  text += "N/A";
  setText(text);
}

void ValueLabel::AdjustSize(const QVariant& value) {
  QFontMetrics metrics(font());
  setFixedWidth(metrics.width(QString::fromStdString(m_label) + ": " +
    m_converter->displayText(value, QLocale())));
}

void ValueLabel::SetValue(const QVariant& value) {
  QString text;
  if(!m_label.empty()) {
    text = "<span style=\"font-weight: bold;\">" +
      QString::fromStdString(m_label) + ":</span> ";
  }
  text += m_converter->displayText(value, QLocale());
  setText(text);
}

void ValueLabel::Reset() {
  QString text;
  if(!m_label.empty()) {
    text = "<span style=\"font-weight: bold;\">" +
      QString::fromStdString(m_label) + ":</span> ";
  }
  text += "N/A";
  setText(text);
}

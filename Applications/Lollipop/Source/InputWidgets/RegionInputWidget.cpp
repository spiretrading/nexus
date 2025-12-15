#include "Spire/InputWidgets/RegionInputWidget.hpp"
#include <QBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include "Spire/InputWidgets/RegionInputDialog.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

RegionInputWidget::RegionInputWidget(
  Ref<UserProfile> userProfile, QWidget* parent)
  : RegionInputWidget(Region(), Ref(userProfile), parent) {}

RegionInputWidget::RegionInputWidget(
    Region region, Ref<UserProfile> userProfile, QWidget* parent)
    : m_userProfile(userProfile.get()),
      m_isReadOnly(false) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  m_lineEdit = new QLineEdit(this);
  m_lineEdit->installEventFilter(this);
  m_lineEdit->setReadOnly(true);
  layout->addWidget(m_lineEdit);
  SetRegion(region);
}

const Region& RegionInputWidget::GetRegion() const {
  return m_region;
}

void RegionInputWidget::SetRegion(const Region& region) {
  m_region = region;
  m_lineEdit->clear();
  auto text = QString();
  if(m_region.is_global()) {
    text = "*";
  } else {
    auto countries = std::vector(
      m_region.get_countries().begin(), m_region.get_countries().end());
    std::sort(countries.begin(), countries.end(), [&] (auto left, auto right) {
      auto left_code =
        m_userProfile->GetCountryDatabase().from(left).m_two_letter_code;
      auto right_code =
        m_userProfile->GetCountryDatabase().from(right).m_two_letter_code;
      return left_code < right_code;
    });
    for(auto& country : countries) {
      auto code =
        m_userProfile->GetCountryDatabase().from(country).m_two_letter_code;
      if(!text.isEmpty()) {
        text += ", ";
      }
      text += code.get_data();
    }
    auto venues =
      std::vector(m_region.get_venues().begin(), m_region.get_venues().end());
    std::sort(venues.begin(), venues.end(), [&] (auto left, auto right) {
      auto& left_name =
        m_userProfile->GetVenueDatabase().from(left).m_display_name;
      auto& right_name =
        m_userProfile->GetVenueDatabase().from(right).m_display_name;
      return left_name < right_name;
    });
    for(auto& venue : venues) {
      auto& name = m_userProfile->GetVenueDatabase().from(venue).m_display_name;
      if(!text.isEmpty()) {
        text += ", ";
      }
      text += QString::fromStdString(name);
    }
    auto securities = std::vector(
      m_region.get_securities().begin(), m_region.get_securities().end());
    std::sort(securities.begin(), securities.end(),
      [&] (const auto& left, const auto& right) {
        return displayText(left) < displayText(right);
      });
    for(auto& security : securities) {
      auto ticker = displayText(security);
      if(!text.isEmpty()) {
        text += ", ";
      }
      text += ticker;
    }
  }
  m_lineEdit->setText(text);
  m_updatedSignal(m_region);
}

void RegionInputWidget::SetReadOnly(bool value) {
  m_isReadOnly = value;
}

connection RegionInputWidget::ConnectUpdatedSignal(
    const UpdatedSignal::slot_type& slot) const {
  return m_updatedSignal.connect(slot);
}

bool RegionInputWidget::eventFilter(QObject* receiver, QEvent* event) {
  if(receiver == m_lineEdit) {
    if(event->type() == QEvent::MouseButtonDblClick) {
      mouseDoubleClickEvent(static_cast<QMouseEvent*>(event));
      return true;
    } else if(event->type() == QEvent::KeyPress) {
      keyPressEvent(static_cast<QKeyEvent*>(event));
      return true;
    }
  }
  return QWidget::eventFilter(receiver, event);
}

void RegionInputWidget::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab ||
      event->text().isEmpty()) {
    QWidget::keyPressEvent(event);
    return;
  }
  if(m_isReadOnly) {
    return;
  }
}

void RegionInputWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  if(m_isReadOnly) {
    return;
  }
  auto dialog = RegionInputDialog(m_region, Ref(*m_userProfile), this);
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  SetRegion(dialog.GetRegion());
}

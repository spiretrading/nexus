#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorVenueSelectionWidget.hpp"
#include <QCheckBox>
#include <QGridLayout>
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const int VENUES_PER_COLUMN = 6;
}

OrderImbalanceIndicatorVenueSelectionWidget::
    OrderImbalanceIndicatorVenueSelectionWidget(QWidget* parent)
    : QWidget(parent) {}

OrderImbalanceIndicatorVenueSelectionWidget::
    OrderImbalanceIndicatorVenueSelectionWidget(
    const VenueDatabase& venueDatabase,
    Ref<OrderImbalanceIndicatorModel> model, QWidget* parent)
    : QWidget(parent) {
  Initialize(venueDatabase, Ref(model));
}

void OrderImbalanceIndicatorVenueSelectionWidget::Initialize(
    const VenueDatabase& venueDatabase,
    Ref<OrderImbalanceIndicatorModel> model) {
  m_model = model.get();
  QGridLayout* layout = new QGridLayout();
  setLayout(layout);
  int count = 0;
  for(auto& venue : venueDatabase.get_entries()) {
    int column = count / VENUES_PER_COLUMN;
    int row = count % VENUES_PER_COLUMN;
    std::unique_ptr<QCheckBox> checkBox = std::make_unique<QCheckBox>(
      QString::fromStdString(venue.m_display_name));
    if(m_model->GetProperties().IsFiltered(venue.m_venue)) {
      checkBox->setCheckState(Qt::Unchecked);
    } else {
      checkBox->setCheckState(Qt::Checked);
    }
    m_venues[checkBox.get()] = venue.m_venue;
    layout->addWidget(checkBox.get(), row, column, 1, 1);
    connect(checkBox.get(), &QCheckBox::stateChanged, this,
      &OrderImbalanceIndicatorVenueSelectionWidget::OnCheckBoxStateChanged);
    m_checkBoxes.push_back(std::move(checkBox));
    ++count;
  }
}

unique_ptr<WindowSettings> OrderImbalanceIndicatorVenueSelectionWidget::
    GetWindowSettings() const {
  return std::make_unique<
    OrderImbalanceIndicatorVenueSelectionWidgetWindowSettings>(*this);
}

void OrderImbalanceIndicatorVenueSelectionWidget::OnCheckBoxStateChanged(
    int state) {
  QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
  Venue venue = m_venues.at(checkBox);
  if(state == Qt::Checked) {
    m_model->SetVenueFilter(venue, false);
  } else {
    m_model->SetVenueFilter(venue, true);
  }
}

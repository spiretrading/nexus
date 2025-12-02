#include "Spire/InputWidgets/RegionInputDialog.hpp"
#include <fstream>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include "Spire/InputWidgets/RegionItemInputDialog.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "ui_ValueListInputDialog.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

namespace {
  QVariant Parse(const std::string& line, UserProfile& userProfile) {
    if(auto country =
        parse_country_code(line, userProfile.GetCountryDatabase())) {
      return QVariant::fromValue(country);
    } else if(auto venue = parse_venue(line, userProfile.GetVenueDatabase())) {
      return QVariant::fromValue(venue);
    } else if(auto security =
        parse_security(line, userProfile.GetVenueDatabase())) {
      return QVariant::fromValue(security);
    } else if(line == "*") {
      return QVariant::fromValue(Region::GLOBAL);
    }
    throw std::runtime_error("Unknown region item.");
  }
}

RegionInputDialog::RegionInputDialog(
  Ref<UserProfile> userProfile, QWidget* parent)
  : RegionInputDialog(Region(), Ref(userProfile), parent) {}

RegionInputDialog::RegionInputDialog(
    Region region, Ref<UserProfile> userProfile, QWidget* parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui_ValueListInputDialog>()),
      m_userProfile(userProfile.get()),
      m_region(std::move(region)) {
  m_ui->setupUi(this);
  setWindowTitle(tr("Region Editor"));
  connect(m_ui->m_addToolButton, &QToolButton::clicked, this,
    &RegionInputDialog::OnAddItemAction);
  connect(m_ui->m_loadToolButton, &QToolButton::clicked, this,
    &RegionInputDialog::OnLoadFileAction);
  connect(m_ui->m_deleteToolButton, &QToolButton::clicked, this,
    &RegionInputDialog::OnDeleteItemAction);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &RegionInputDialog::OnAccept);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &RegionInputDialog::reject);
  connect(m_ui->m_valueListWidget, &QTableWidget::cellActivated, this,
    &RegionInputDialog::OnCellActivated);
  auto itemDelegate = new CustomVariantItemDelegate(Ref(*m_userProfile));
  m_ui->m_valueListWidget->setItemDelegate(itemDelegate);
  m_ui->m_valueListWidget->installEventFilter(this);
  auto metrics = QFontMetrics(m_ui->m_valueListWidget->font());
  m_ui->m_valueListWidget->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  if(m_region.is_global()) {
    Append(QVariant::fromValue(Region::GLOBAL));
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
      Append(QVariant::fromValue(country));
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
      Append(QVariant::fromValue(venue));
    }
    auto securities = std::vector(
      m_region.get_securities().begin(), m_region.get_securities().end());
    std::sort(securities.begin(), securities.end(),
      [&] (const auto& left, const auto& right) {
        return displayText(left) < displayText(right);
      });
    for(auto& security : securities) {
      Append(QVariant::fromValue(security));
    }
  }
}

RegionInputDialog::~RegionInputDialog() = default;

const Region& RegionInputDialog::GetRegion() const {
  return m_region;
}

void RegionInputDialog::ActivateRow(int row, QKeyEvent* event) {
  auto item = m_ui->m_valueListWidget->item(row, 0);
  auto region = item->data(Qt::DisplayRole);
  auto text = [&] {
    if(event) {
      return event->text().trimmed();
    }
    return QString();
  }();
  auto initialValue = [&] () -> variant<std::string, QVariant> {
    if(text.isEmpty()) {
      return region;
    }
    return text.toStdString();
  }();
  ShowRegionItemInputDialog(
    initialValue, Ref(*m_userProfile), m_ui->m_valueListWidget,
    [=] (auto value) {
      if(!value) {
        return;
      }
      item->setData(Qt::DisplayRole, *value);
    });
}

void RegionInputDialog::Append(const QVariant& item) {
  auto widgetItem = new QTableWidgetItem();
  widgetItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
  auto row = m_ui->m_valueListWidget->rowCount();
  m_ui->m_valueListWidget->setRowCount(row + 1);
  m_ui->m_valueListWidget->setItem(row, 0, widgetItem);
  widgetItem->setData(Qt::DisplayRole, item);
}

void RegionInputDialog::OnAccept() {
  m_region = Region();
  for(auto i = 0; i < m_ui->m_valueListWidget->rowCount(); ++i) {
    auto value = m_ui->m_valueListWidget->item(i, 0)->data(Qt::DisplayRole);
    if(value.isValid()) {
      if(value.canConvert<CountryCode>()) {
        m_region += value.value<CountryCode>();
      } else if(value.canConvert<Venue>()) {
        m_region += value.value<Venue>();
      } else if(value.canConvert<Security>()) {
        m_region += value.value<Security>();
      } else if(value.canConvert<Region>()) {
        m_region += value.value<Region>();
      }
    }
  }
  accept();
}

void RegionInputDialog::OnAddItemAction() {
  auto item = new QTableWidgetItem();
  item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
  auto row = m_ui->m_valueListWidget->rowCount();
  m_ui->m_valueListWidget->setRowCount(row + 1);
  m_ui->m_valueListWidget->setItem(row, 0, item);
}

void RegionInputDialog::OnLoadFileAction() {
  auto path = QFileDialog::getOpenFileName(
    this, "Load Values", "", "Comma Separated Values (*.csv)");
  if(path.isNull()) {
    return;
  }
  auto file = std::ifstream(path.toStdString());
  auto line = std::string();
  auto lineCount = 0;
  auto parsedValues = std::vector<QVariant>();
  while(std::getline(file, line)) {
    ++lineCount;
    try {
      auto parsedValue = Parse(line, *m_userProfile);
      parsedValues.push_back(parsedValue);
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        "File contents on line " + QString::number(lineCount) +
        " are invalid: " + QString::fromStdString(e.what()));
      return;
    }
  }
  for(auto& parsedValue : parsedValues) {
    Append(parsedValue);
  }
}

void RegionInputDialog::OnDeleteItemAction() {
  for(auto selectedItem : m_ui->m_valueListWidget->selectedItems()) {
    m_ui->m_valueListWidget->removeRow(selectedItem->row());
  }
}

void RegionInputDialog::OnCellActivated(int row, int column) {
  ActivateRow(row, nullptr);
}

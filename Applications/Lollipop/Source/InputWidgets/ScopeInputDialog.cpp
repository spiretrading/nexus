#include "Spire/InputWidgets/ScopeInputDialog.hpp"
#include <fstream>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include "Spire/InputWidgets/ScopeItemInputDialog.hpp"
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
    } else if(auto ticker =
        parse_ticker(line, userProfile.GetVenueDatabase())) {
      return QVariant::fromValue(ticker);
    } else if(line == "*") {
      return QVariant::fromValue(Scope::GLOBAL);
    }
    throw std::runtime_error("Unknown scope.");
  }
}

ScopeInputDialog::ScopeInputDialog(
  Ref<UserProfile> userProfile, QWidget* parent)
  : ScopeInputDialog(Scope(), Ref(userProfile), parent) {}

ScopeInputDialog::ScopeInputDialog(
    Scope scope, Ref<UserProfile> userProfile, QWidget* parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui_ValueListInputDialog>()),
      m_userProfile(userProfile.get()),
      m_scope(std::move(scope)) {
  m_ui->setupUi(this);
  setWindowTitle(tr("Scope Editor"));
  connect(m_ui->m_addToolButton, &QToolButton::clicked, this,
    &ScopeInputDialog::OnAddItemAction);
  connect(m_ui->m_loadToolButton, &QToolButton::clicked, this,
    &ScopeInputDialog::OnLoadFileAction);
  connect(m_ui->m_deleteToolButton, &QToolButton::clicked, this,
    &ScopeInputDialog::OnDeleteItemAction);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &ScopeInputDialog::OnAccept);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &ScopeInputDialog::reject);
  connect(m_ui->m_valueListWidget, &QTableWidget::cellActivated, this,
    &ScopeInputDialog::OnCellActivated);
  auto itemDelegate = new CustomVariantItemDelegate(Ref(*m_userProfile));
  m_ui->m_valueListWidget->setItemDelegate(itemDelegate);
  m_ui->m_valueListWidget->installEventFilter(this);
  auto metrics = QFontMetrics(m_ui->m_valueListWidget->font());
  m_ui->m_valueListWidget->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  if(m_scope.is_global()) {
    Append(QVariant::fromValue(Scope::GLOBAL));
  } else {
    auto countries = std::vector(
      m_scope.get_countries().begin(), m_scope.get_countries().end());
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
      std::vector(m_scope.get_venues().begin(), m_scope.get_venues().end());
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
    auto tickers = std::vector(
      m_scope.get_tickers().begin(), m_scope.get_tickers().end());
    std::sort(tickers.begin(), tickers.end(),
      [&] (const auto& left, const auto& right) {
        return displayText(left) < displayText(right);
      });
    for(auto& ticker : tickers) {
      Append(QVariant::fromValue(ticker));
    }
  }
}

ScopeInputDialog::~ScopeInputDialog() = default;

const Scope& ScopeInputDialog::GetScope() const {
  return m_scope;
}

void ScopeInputDialog::ActivateRow(int row, QKeyEvent* event) {
  auto item = m_ui->m_valueListWidget->item(row, 0);
  auto scope = item->data(Qt::DisplayRole);
  auto text = [&] {
    if(event) {
      return event->text().trimmed();
    }
    return QString();
  }();
  auto initialValue = [&] () -> variant<std::string, QVariant> {
    if(text.isEmpty()) {
      return scope;
    }
    return text.toStdString();
  }();
  ShowScopeItemInputDialog(
    initialValue, Ref(*m_userProfile), m_ui->m_valueListWidget,
    [=] (auto value) {
      if(!value) {
        return;
      }
      item->setData(Qt::DisplayRole, *value);
    });
}

void ScopeInputDialog::Append(const QVariant& item) {
  auto widgetItem = new QTableWidgetItem();
  widgetItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
  auto row = m_ui->m_valueListWidget->rowCount();
  m_ui->m_valueListWidget->setRowCount(row + 1);
  m_ui->m_valueListWidget->setItem(row, 0, widgetItem);
  widgetItem->setData(Qt::DisplayRole, item);
}

void ScopeInputDialog::OnAccept() {
  m_scope = Scope();
  for(auto i = 0; i < m_ui->m_valueListWidget->rowCount(); ++i) {
    auto value = m_ui->m_valueListWidget->item(i, 0)->data(Qt::DisplayRole);
    if(value.isValid()) {
      if(value.canConvert<CountryCode>()) {
        m_scope += value.value<CountryCode>();
      } else if(value.canConvert<Venue>()) {
        m_scope += value.value<Venue>();
      } else if(value.canConvert<Ticker>()) {
        m_scope += value.value<Ticker>();
      } else if(value.canConvert<Scope>()) {
        m_scope += value.value<Scope>();
      }
    }
  }
  accept();
}

void ScopeInputDialog::OnAddItemAction() {
  auto item = new QTableWidgetItem();
  item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
  auto row = m_ui->m_valueListWidget->rowCount();
  m_ui->m_valueListWidget->setRowCount(row + 1);
  m_ui->m_valueListWidget->setItem(row, 0, item);
}

void ScopeInputDialog::OnLoadFileAction() {
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

void ScopeInputDialog::OnDeleteItemAction() {
  for(auto selectedItem : m_ui->m_valueListWidget->selectedItems()) {
    m_ui->m_valueListWidget->removeRow(selectedItem->row());
  }
}

void ScopeInputDialog::OnCellActivated(int row, int column) {
  ActivateRow(row, nullptr);
}

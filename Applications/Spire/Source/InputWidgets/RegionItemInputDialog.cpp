#include "Spire/InputWidgets/RegionItemInputDialog.hpp"
#include <QHeaderView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTableView>
#include <boost/variant/get.hpp>
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_SecurityInputDialog.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  QVariant Parse(const std::string& line, UserProfile& userProfile) {
    if(auto country = parse_country_code(line)) {
      return QVariant::fromValue(country);
    } else if(auto venue = parse_venue(line)) {
      return QVariant::fromValue(venue);
    } else if(auto security = parse_security(line)) {
      return QVariant::fromValue(security);
    } else if(line == "*") {
      return QVariant::fromValue(Region::GLOBAL);
    }
    return QVariant();
  }
}

RegionItemInputDialog::RegionItemInputDialog(
  Ref<UserProfile> userProfile, QWidget* parent)
  : RegionItemInputDialog(QVariant(), Ref(userProfile), parent) {}

RegionItemInputDialog::RegionItemInputDialog(
  QVariant item, Ref<UserProfile> userProfile, QWidget* parent)
  : RegionItemInputDialog(CustomVariantItemDelegate(
      Ref(*m_userProfile)).displayText(item, QLocale()).toStdString(),
      Ref(userProfile), parent) {}

RegionItemInputDialog::RegionItemInputDialog(
    const std::string& text, Ref<UserProfile> userProfile, QWidget* parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui_SecurityInputDialog>()),
      m_userProfile(userProfile.get()) {
  m_ui->setupUi(this);
  setWindowTitle("Region");
  m_ui->m_label->setText(tr("Region"));
  m_ui->m_input->setText(QString::fromStdString(text));
  if(parent) {
    parent->installEventFilter(this);
  }
  m_model = std::make_unique<RegionQueryModel>(Ref(*m_userProfile));
  m_completer = new RegionCompleter(m_model.get(), this);
  m_completer->setWrapAround(false);
  m_completerPopup = new QTableView();
  m_completerPopup->setShowGrid(false);
  m_completerPopup->verticalHeader()->hide();
  m_completerPopup->horizontalHeader()->hide();
  m_completerPopup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto metrics = QFontMetrics(m_completerPopup->font());
  m_completerPopup->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  m_completerPopup->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_completerPopup->setSelectionMode(QAbstractItemView::SingleSelection);
  m_completer->setPopup(m_completerPopup);
  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_completer->setCompletionRole(Qt::DisplayRole);
  m_ui->m_input->setCompleter(m_completer);
  m_completerPopup->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  m_completerPopup->horizontalHeader()->stretchLastSection();
  connect(m_ui->m_input, &QLineEdit::textChanged, this,
    &RegionItemInputDialog::OnInputEdited);
  connect(m_ui->m_input, &QLineEdit::returnPressed, this,
    &RegionItemInputDialog::accept);
  connect(m_completer->model(), &QAbstractItemModel::rowsInserted, this,
    &RegionItemInputDialog::OnRowsAddedRemoved);
  connect(m_completer->model(), &QAbstractItemModel::rowsRemoved, this,
    &RegionItemInputDialog::OnRowsAddedRemoved);
  connect(m_completer->model(),&QAbstractItemModel::dataChanged, this,
    &RegionItemInputDialog::OnDataChanged);
}

RegionItemInputDialog::~RegionItemInputDialog() = default;

QVariant RegionItemInputDialog::GetItem() const {
  return Parse(m_ui->m_input->text().toUpper().toStdString(), *m_userProfile);
}

QLineEdit& RegionItemInputDialog::GetSymbolInput() {
  return *m_ui->m_input;
}

bool RegionItemInputDialog::eventFilter(QObject* receiver, QEvent* event) {
  if(isVisible() && event->type() == QEvent::KeyPress) {
    auto forwardEvent = QKeyEvent(static_cast<QKeyEvent&>(*event));
    QCoreApplication::sendEvent(m_ui->m_input, &forwardEvent);
    return true;
  }
  return QDialog::eventFilter(receiver, event);
}

void RegionItemInputDialog::AdjustCompleterSize() {
  m_completerPopup->resizeColumnsToContents();
  auto width = 2 + m_completerPopup->verticalHeader()->width();
  for(auto i = 0; i < RegionQueryModel::COLUMN_COUNT; ++i) {
    width += 4 + m_completerPopup->columnWidth(i);
  }
  width = std::max(width, m_ui->m_input->width());
  m_completerPopup->setMinimumWidth(width);
  m_completerPopup->setMaximumWidth(width);
  auto height = 4 + m_completerPopup->horizontalHeader()->height();
  for(auto i = 0; i < std::min(7, m_model->rowCount(QModelIndex())); ++i) {
    height += m_completerPopup->rowHeight(i);
  }
  m_completerPopup->setMinimumHeight(height);
  m_completerPopup->setMaximumHeight(height);
}

void RegionItemInputDialog::OnInputEdited(const QString& text) {
  m_model->Search(text.toStdString());
}

void RegionItemInputDialog::OnRowsAddedRemoved(
    const QModelIndex& parent, int start, int end) {
  m_completer->complete();
  AdjustCompleterSize();
}

void RegionItemInputDialog::OnDataChanged(
    const QModelIndex& topLeft, const QModelIndex& bottomRight) {
  m_completer->complete();
  AdjustCompleterSize();
}

void Spire::ShowRegionItemInputDialog(
    const variant<std::string, QVariant>& item, Ref<UserProfile> userProfile,
    QWidget* parent, std::function<void (optional<QVariant> item)> onResult) {
  auto dialog = [&] {
    if(auto text = get<std::string>(&item)) {
      return new RegionItemInputDialog(*text, Ref(userProfile), parent);
    }
    return new RegionItemInputDialog(
      get<QVariant>(item), Ref(userProfile), parent);
  }();
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  QObject::connect(dialog, &RegionItemInputDialog::finished, parent,
    [=] (auto result) {
      if(result == QDialog::Rejected) {
        onResult(none);
      } else {
        onResult(dialog->GetItem());
      }
    });
  dialog->show();
}

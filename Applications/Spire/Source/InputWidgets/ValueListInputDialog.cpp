#include "Spire/InputWidgets/ValueListInputDialog.hpp"
#include <Beam/Utilities/InstantiateTemplate.hpp>
#include <boost/optional/optional.hpp>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include "Nexus/Definitions/Market.hpp"
#include "Spire/InputWidgets/SecurityInputDialog.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "ui_ValueListInputDialog.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  struct ValueToVariantConverter {
    template<typename T>
    static void Template(vector<ValueListInputDialog::Value>& values,
        const QVariant& value) {
      values.push_back(value.value<T>());
    }

    using SupportedTypes = boost::mpl::list<Security>;
  };

  struct VariantToValueConverter {
    template<typename T>
    static QVariant Template(const ValueListInputDialog::Value& value) {
      return QVariant::fromValue(boost::get<T>(value));
    }

    using SupportedTypes = boost::mpl::list<Security>;
  };

  struct ParseLine {
    template<typename T>
    static ValueListInputDialog::Value Template(const string& line,
        const UserProfile& userProfile) {
      return {};
    }

    template<>
    static ValueListInputDialog::Value Template<Security>(const string& line,
        const UserProfile& userProfile) {
      auto security = ParseSecurity(line, userProfile.GetMarketDatabase());
      if(security == Security{}) {
        BOOST_THROW_EXCEPTION(runtime_error{"Invalid symbol specified."});
      }
      return security;
    }

    using SupportedTypes = boost::mpl::list<Security>;
  };
}

ValueListInputDialog::ValueListInputDialog(RefType<UserProfile> userProfile,
    const std::type_info& type, QWidget* parent, Qt::WindowFlags flags)
    : QDialog{parent, flags},
      m_ui{std::make_unique<Ui_ValueListInputDialog>()},
      m_userProfile{userProfile.Get()},
      m_type{&type} {
  m_ui->setupUi(this);
  connect(m_ui->m_addToolButton, &QToolButton::clicked, this,
    &ValueListInputDialog::OnAddItemAction);
  connect(m_ui->m_loadToolButton, &QToolButton::clicked, this,
    &ValueListInputDialog::OnLoadFileAction);
  connect(m_ui->m_deleteToolButton, &QToolButton::clicked, this,
    &ValueListInputDialog::OnDeleteItemAction);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &ValueListInputDialog::OnAccept);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &ValueListInputDialog::reject);
  connect(m_ui->m_valueListWidget, &QTableWidget::cellActivated, this,
    &ValueListInputDialog::OnCellActivated);
  auto itemDelegate = new CustomVariantItemDelegate{Ref(*m_userProfile)};
  m_ui->m_valueListWidget->setItemDelegate(itemDelegate);
  m_ui->m_valueListWidget->installEventFilter(this);
  QFontMetrics metrics{m_ui->m_valueListWidget->font()};
  m_ui->m_valueListWidget->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
}

ValueListInputDialog::ValueListInputDialog(RefType<UserProfile> userProfile,
    const std::type_info& type, const std::vector<Value>& values,
    QWidget* parent, Qt::WindowFlags flags)
    : ValueListInputDialog{Ref(userProfile), type, parent, flags} {
  for(auto& value : values) {
    AppendItem(value);
  }
}

ValueListInputDialog::~ValueListInputDialog() {}

const vector<ValueListInputDialog::Value>&
    ValueListInputDialog::GetValues() const {
  return m_values;
}

bool ValueListInputDialog::eventFilter(QObject* receiver, QEvent* event) {
  if(receiver == m_ui->m_valueListWidget) {
    if(event->type() == QEvent::KeyPress) {
      auto& keyEvent = *static_cast<QKeyEvent*>(event);
      if(keyEvent.key() == Qt::Key_Delete) {
        OnDeleteItemAction();
        return true;
      } else {
        auto currentRow = m_ui->m_valueListWidget->currentRow();
        if(currentRow != -1) {
          auto text = keyEvent.text().trimmed();
          if(keyEvent.key() != Qt::Key_Tab &&
              keyEvent.key() != Qt::Key_Escape &&
              keyEvent.key() != Qt::Key_Backtab && !text.isEmpty()) {
            ActivateRow(currentRow, &keyEvent);
            return true;
          }
        }
      }
    }
  }
  return QDialog::eventFilter(receiver, event);
}

void ValueListInputDialog::ActivateRow(int row, QKeyEvent* event) {
  if(*m_type == typeid(Security)) {
    auto item = m_ui->m_valueListWidget->item(row, 0);
    auto security = item->data(Qt::DisplayRole).value<Security>();
    optional<SecurityInputDialog> dialog;
    auto text =
      [&] {
        if(event == nullptr) {
          return QString{};
        }
        return event->text().trimmed();
      }();
    if(text.isEmpty()) {
      dialog.emplace(Ref(*m_userProfile), security);
    } else {
      dialog.emplace(Ref(*m_userProfile), text.toStdString());
    }
    dialog->GetSymbolInput().selectAll();
    if(dialog->exec() == QDialog::Rejected) {
      return;
    }
    auto newValue = dialog->GetSecurity(true);
    if(newValue == Security{}) {
      return;
    }
    item->setData(Qt::DisplayRole, QVariant::fromValue(newValue));
  }
}

void ValueListInputDialog::AppendItem(const Value& value) {
  auto item = new QTableWidgetItem{};
  item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
  auto row = m_ui->m_valueListWidget->rowCount();
  m_ui->m_valueListWidget->setRowCount(row + 1);
  m_ui->m_valueListWidget->setItem(row, 0, item);
  item->setData(Qt::DisplayRole,
    Instantiate<VariantToValueConverter>(*m_type)(value));
}

void ValueListInputDialog::OnAccept() {
  m_values.clear();
  for(auto i = 0; i < m_ui->m_valueListWidget->rowCount(); ++i) {
    auto value = m_ui->m_valueListWidget->item(i, 0)->data(Qt::DisplayRole);
    if(value.isValid()) {
      Instantiate<ValueToVariantConverter>(*m_type)(m_values, value);
    }
  }
  accept();
}

void ValueListInputDialog::OnAddItemAction() {
  auto item = new QTableWidgetItem{};
  item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
  auto row = m_ui->m_valueListWidget->rowCount();
  m_ui->m_valueListWidget->setRowCount(row + 1);
  m_ui->m_valueListWidget->setItem(row, 0, item);
}

void ValueListInputDialog::OnLoadFileAction() {
  auto path = QFileDialog::getOpenFileName(this, "Load Values", "",
    "Comma Separated Values (*.csv)");
  if(path.isNull()) {
    return;
  }
  ifstream file{path.toStdString()};
  string line;
  auto lineCount = 0;
  vector<Value> parsedValues;
  while(getline(file, line)) {
    ++lineCount;
    try {
      auto parsedValue = Instantiate<ParseLine>(*m_type)(line, *m_userProfile);
      parsedValues.push_back(parsedValue);
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        "File contents on line " +
        QString::number(lineCount) + " are invalid: " +
        QString::fromStdString(e.what()));
      return;
    }
  }
  for(auto& parsedValue : parsedValues) {
    AppendItem(parsedValue);
  }
}

void ValueListInputDialog::OnDeleteItemAction() {
  for(auto selectedItem : m_ui->m_valueListWidget->selectedItems()) {
    m_ui->m_valueListWidget->removeRow(selectedItem->row());
  }
}

void ValueListInputDialog::OnCellActivated(int row, int column) {
  ActivateRow(row, nullptr);
}

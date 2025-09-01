#include "Spire/AccountViewer/ComplianceRuleEntryWidget.hpp"
#include "Spire/AccountViewer/ComplianceModel.hpp"
#include "Spire/AccountViewer/ComplianceValueWidget.hpp"
#include "Spire/UI/ReadOnlyCheckBox.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_ComplianceRuleEntryWidget.h"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

ComplianceRuleEntryWidget::ComplianceRuleEntryWidget(
    Ref<UserProfile> userProfile, bool isReadOnly,
    const ComplianceRuleEntry& entry, std::shared_ptr<ComplianceModel> model,
    QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_ui{std::make_unique<Ui_ComplianceRuleEntryWidget>()},
      m_userProfile{userProfile.Get()},
      m_hasModifications{false},
      m_isReadOnly{isReadOnly},
      m_idUpdated{false},
      m_entry{entry},
      m_model{std::move(model)} {
  m_ui->setupUi(this);
  m_ui->m_parametersWidget->hide();
  if(m_entry.get_schema().get_name() == PerAccountComplianceRule::GetName()) {
    for(auto& parameter : m_entry.get_schema().get_parameters()) {
      if(parameter.m_name == "name") {
        m_ruleName = boost::get<string>(parameter.m_value);
        break;
      }
    }
  } else {
    m_ruleName = entry.get_schema().get_name();
  }
  m_ui->m_ruleLabel->setText(QString::fromStdString(m_ruleName));
  if(m_entry.get_directory_entry().m_type == DirectoryEntry::Type::DIRECTORY) {
    m_ui->m_stateComboBox->addItem(tr("Active Per Account"));
    m_ui->m_stateComboBox->addItem(tr("Active Consolidated"));
    m_ui->m_stateComboBox->addItem(tr("Passive Per Account"));
    m_ui->m_stateComboBox->addItem(tr("Passive Consolidated"));
    m_ui->m_stateComboBox->addItem(tr("Disabled"));
    if(entry.get_state() == ComplianceRuleEntry::State::ACTIVE) {
      if(m_entry.get_schema().get_name() ==
          PerAccountComplianceRule::GetName()) {
        m_ui->m_stateComboBox->setCurrentIndex(0);
      } else {
        m_ui->m_stateComboBox->setCurrentIndex(1);
      }
    } else if(entry.get_state() == ComplianceRuleEntry::State::PASSIVE) {
      if(m_entry.get_schema().get_name() ==
          PerAccountComplianceRule::GetName()) {
        m_ui->m_stateComboBox->setCurrentIndex(2);
      } else {
        m_ui->m_stateComboBox->setCurrentIndex(3);
      }
    } else {
      m_ui->m_stateComboBox->setCurrentIndex(4);
    }
  } else {
    m_ui->m_stateComboBox->addItem(tr("Active"));
    m_ui->m_stateComboBox->addItem(tr("Passive"));
    m_ui->m_stateComboBox->addItem(tr("Disabled"));
    if(entry.get_state() == ComplianceRuleEntry::State::ACTIVE) {
      m_ui->m_stateComboBox->setCurrentIndex(0);
    } else if(entry.get_state() == ComplianceRuleEntry::State::PASSIVE) {
      m_ui->m_stateComboBox->setCurrentIndex(1);
    } else {
      m_ui->m_stateComboBox->setCurrentIndex(2);
    }
  }
  m_ui->m_stateComboBox->setEnabled(!m_isReadOnly);
  m_ui->m_ruleLabel->setCheckable(!m_isReadOnly);
  m_connections.AddConnection(
    m_model->ConnectComplianceRuleEntryIdUpdatedSignal(
    std::bind(&ComplianceRuleEntryWidget::OnEntryIdChanged, this,
    std::placeholders::_1, std::placeholders::_2)));
  m_connections.AddConnection(m_ui->m_expandButton->ConnectExpandedSignal(
    std::bind(&ComplianceRuleEntryWidget::OnTableExpanded, this)));
  m_connections.AddConnection(m_ui->m_expandButton->ConnectCollapsedSignal(
    std::bind(&ComplianceRuleEntryWidget::OnTableCollapsed, this)));
  SetupParameters();
}

ComplianceRuleEntryWidget::~ComplianceRuleEntryWidget() {}

const ComplianceRuleEntry& ComplianceRuleEntryWidget::GetEntry() const {
  return m_entry;
}

bool ComplianceRuleEntryWidget::IsSelected() const {
  return m_ui->m_ruleLabel->isChecked();
}

void ComplianceRuleEntryWidget::Commit() {
  auto state =
    [&] {
      if(m_entry.get_directory_entry().m_type ==
          DirectoryEntry::Type::DIRECTORY) {
        if(m_ui->m_stateComboBox->currentIndex() == 0 ||
            m_ui->m_stateComboBox->currentIndex() == 1) {
          return ComplianceRuleEntry::State::ACTIVE;
        } else if(m_ui->m_stateComboBox->currentIndex() == 2 ||
            m_ui->m_stateComboBox->currentIndex() == 3) {
          return ComplianceRuleEntry::State::PASSIVE;
        } else {
          return ComplianceRuleEntry::State::DISABLED;
        }
      } else {
        if(m_ui->m_stateComboBox->currentIndex() == 0) {
          return ComplianceRuleEntry::State::ACTIVE;
        } else if(m_ui->m_stateComboBox->currentIndex() == 1) {
          return ComplianceRuleEntry::State::PASSIVE;
        } else {
          return ComplianceRuleEntry::State::DISABLED;
        }
      }
    }();
  auto isConsolidated =
    [&] {
      if(m_entry.get_directory_entry().m_type ==
          DirectoryEntry::Type::DIRECTORY) {
        if(m_ui->m_stateComboBox->currentIndex() == 1 ||
            m_ui->m_stateComboBox->currentIndex() == 3 ||
            m_ui->m_stateComboBox->currentIndex() == 4) {
          return true;
        } else {
          return false;
        }
      } else {
        return true;
      }
    }();
  auto hasUpdate = false;
  if(state != m_entry.get_state()) {
    hasUpdate = true;
  }
  vector<ComplianceParameter> parameters;
  for(auto i = 0; i < m_ui->m_parametersLayout->rowCount(); ++i) {
    auto& name = m_parameterNames[i];
    auto& widget = *m_ui->m_parametersLayout->itemAt(
      i, QFormLayout::FieldRole)->widget();
    auto value = GetComplianceValue(widget);
    parameters.emplace_back(name, value);
  }
  if(isConsolidated) {
    ComplianceRuleSchema schema{m_ruleName, std::move(parameters)};
    if(m_entry.get_schema().get_name() == PerAccountComplianceRule::GetName() ||
        schema.get_parameters() != m_entry.get_schema().get_parameters()) {
      hasUpdate = true;
    }
    m_entry = ComplianceRuleEntry{m_entry.get_id(),
      m_entry.get_directory_entry(), state, std::move(schema)};
  } else {
    for(auto& parameter : parameters) {
      parameter.m_name = "\\" + parameter.m_name;
    }
    parameters.emplace_back("name", m_ruleName);
    ComplianceRuleSchema schema{PerAccountComplianceRule::GetName(),
      std::move(parameters)};
    if(m_entry.get_schema().get_name() != PerAccountComplianceRule::GetName() ||
        schema.get_parameters() != m_entry.get_schema().get_parameters()) {
      hasUpdate = true;
    }
    m_entry = ComplianceRuleEntry{m_entry.get_id(),
      m_entry.get_directory_entry(), state, std::move(schema)};
  }
  if(hasUpdate) {
    m_model->Update(m_entry);
  }
}

void ComplianceRuleEntryWidget::SetupParameters() {
  boost::optional<ComplianceRuleSchema> schema;
  for(auto& modelSchema : m_model->GetSchemas()) {
    if(modelSchema.get_name() == m_ruleName) {
      schema = modelSchema;
      break;
    }
  }
  if(!schema.is_initialized()) {
    return;
  }
  for(auto& parameter : m_entry.get_schema().get_parameters()) {
    if(m_entry.get_schema().get_name() == PerAccountComplianceRule::GetName() &&
        parameter.m_name == "name") {
      continue;
    }
    auto name =
      [&] () -> string {
        if(m_entry.get_schema().get_name() ==
            PerAccountComplianceRule::GetName()) {
          return parameter.m_name.substr(1);
        }
        return parameter.m_name;
      }();
    boost::optional<ComplianceValue> parameterValue;
    for(auto& schemaParameter : schema->get_parameters()) {
      if(schemaParameter.m_name == name) {
        parameterValue = schemaParameter.m_value;
        break;
      }
    }
    if(!parameterValue.is_initialized()) {
      continue;
    }
    m_parameterNames.push_back(name);
    auto parameterWidget = MakeComplianceValueWidget(*parameterValue,
      m_isReadOnly, Ref(*m_userProfile));
    SetComplianceValue(*parameterWidget, parameter.m_value);
    m_ui->m_parametersLayout->addRow(QString::fromStdString(name + ": "),
      parameterWidget);
  }
}

void ComplianceRuleEntryWidget::OnTableExpanded() {
  m_ui->m_parametersWidget->show();
}

void ComplianceRuleEntryWidget::OnTableCollapsed() {
  m_ui->m_parametersWidget->hide();
}

void ComplianceRuleEntryWidget::OnEntryIdChanged(
    ComplianceRuleEntry::Id previousId, ComplianceRuleEntry::Id newId) {
  if(!m_idUpdated && previousId == m_entry.get_id()) {
    m_idUpdated = true;
    m_entry = ComplianceRuleEntry(newId, m_entry.get_directory_entry(),
      m_entry.get_state(), m_entry.get_schema());
  }
}

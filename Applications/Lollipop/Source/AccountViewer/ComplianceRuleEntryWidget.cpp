#include "Spire/AccountViewer/ComplianceRuleEntryWidget.hpp"
#include "Nexus/Compliance/MapComplianceRule.hpp"
#include "Spire/AccountViewer/ComplianceModel.hpp"
#include "Spire/AccountViewer/ComplianceValueWidget.hpp"
#include "Spire/UI/ReadOnlyCheckBox.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_ComplianceRuleEntryWidget.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  ComplianceRuleSchema Flatten(const ComplianceRuleSchema& schema) {
    auto wrappedName = optional<std::string>();
    auto arguments = optional<std::vector<ComplianceValue>>();
    for(auto& parameter : schema.get_parameters()) {
      if(parameter.m_name == "name") {
        if(auto value = get<std::string>(&parameter.m_value)) {
          wrappedName = *value;
        }
      } else if(parameter.m_name == "arguments") {
        if(auto value = get<std::vector<ComplianceValue>>(&parameter.m_value)) {
          arguments = *value;
        }
      }
    }
    if(!wrappedName || !arguments) {
      return schema;
    }
    auto innerParameters = std::vector<ComplianceParameter>();
    for(auto& argument : *arguments) {
      auto parameters = get<std::vector<ComplianceValue>>(&argument);
      if(!parameters || parameters->size() != 2) {
        return schema;
      }
      auto name = get<std::string>(&(*parameters)[0]);
      if(!name) {
        return schema;
      }
      innerParameters.push_back(ComplianceParameter(*name, (*parameters)[1]));
    }
    auto inner = Flatten(ComplianceRuleSchema(
      std::move(*wrappedName), std::move(innerParameters)));
    auto resultParameters = std::vector<ComplianceParameter>();
    for(auto& parameter : schema.get_parameters()) {
      if(parameter.m_name == "name" || parameter.m_name == "arguments") {
        continue;
      }
      resultParameters.push_back(parameter);
    }
    for(auto& parameter : inner.get_parameters()) {
      resultParameters.push_back(parameter);
    }
    return ComplianceRuleSchema(inner.get_name(), std::move(resultParameters));
  }

  ComplianceRuleEntry Flatten(const ComplianceRuleEntry& entry) {
    return ComplianceRuleEntry(entry.get_id(), entry.get_directory_entry(),
      entry.get_state(), Flatten(entry.get_schema()));
  }

  ComplianceRuleSchema Renest(const ComplianceRuleSchema& flattened,
      const ComplianceRuleSchema& fullyWrapped) {
    auto wrappedName = optional<std::string>();
    auto arguments = optional<std::vector<ComplianceValue>>();
    auto parameter_names = std::vector<std::string>();
    for(auto& parameter : fullyWrapped.get_parameters()) {
      if(parameter.m_name == "name") {
        if(auto value = get<std::string>(&parameter.m_value)) {
          wrappedName = *value;
        }
      } else if(parameter.m_name == "arguments") {
        if(auto value = get<std::vector<ComplianceValue>>(&parameter.m_value)) {
          arguments = *value;
        }
      } else {
        parameter_names.push_back(parameter.m_name);
      }
    }
    if(!wrappedName || !arguments ||
        flattened.get_parameters().size() < parameter_names.size()) {
      return flattened;
    }
    auto outerParameters = std::vector<ComplianceParameter>();
    for(auto i = std::size_t(0); i < parameter_names.size(); ++i) {
      for(auto& flattened_parameter : flattened.get_parameters()) {
        if(flattened_parameter.m_name == parameter_names[i]) {
          outerParameters.emplace_back(flattened_parameter);
          break;
        }
      }
      if(outerParameters.size() != i + 1) {
        return flattened;
      }
    }
    auto innerParameters = std::vector<ComplianceParameter>();
    for(auto& flattened_parameter : flattened.get_parameters()) {
      auto i = std::find(parameter_names.begin(), parameter_names.end(),
        flattened_parameter.m_name);
      if(i == parameter_names.end()) {
        innerParameters.push_back(flattened_parameter);
      }
    }
    auto innerSchema = Renest(ComplianceRuleSchema(
      flattened.get_name(), std::move(innerParameters)), unwrap(fullyWrapped));
    return wrap(
      fullyWrapped.get_name(), std::move(outerParameters), innerSchema);
  }
}

ComplianceRuleEntryWidget::ComplianceRuleEntryWidget(
    Ref<UserProfile> userProfile, bool isReadOnly,
    const ComplianceRuleEntry& entry, std::shared_ptr<ComplianceModel> model,
    QWidget* parent, Qt::WindowFlags flags)
    : QWidget{parent, flags},
      m_ui{std::make_unique<Ui_ComplianceRuleEntryWidget>()},
      m_userProfile{userProfile.get()},
      m_hasModifications{false},
      m_isReadOnly{isReadOnly},
      m_idUpdated{false},
      m_entry{entry},
      m_flattenedEntry{Flatten(entry)},
      m_model{std::move(model)} {
  m_ui->setupUi(this);
  m_ui->m_parametersWidget->hide();
  m_ui->m_ruleLabel->setText(
    QString::fromStdString(m_flattenedEntry.get_schema().get_name()));
  if(m_entry.get_directory_entry().m_type == DirectoryEntry::Type::DIRECTORY) {
    m_ui->m_stateComboBox->addItem(tr("Active Per Account"));
    m_ui->m_stateComboBox->addItem(tr("Active Consolidated"));
    m_ui->m_stateComboBox->addItem(tr("Passive Per Account"));
    m_ui->m_stateComboBox->addItem(tr("Passive Consolidated"));
    m_ui->m_stateComboBox->addItem(tr("Disabled"));
    if(entry.get_state() == ComplianceRuleEntry::State::ACTIVE) {
      if(m_entry.get_schema().get_name() == PER_ACCOUNT_RULE_NAME) {
        m_ui->m_stateComboBox->setCurrentIndex(0);
      } else {
        m_ui->m_stateComboBox->setCurrentIndex(1);
      }
    } else if(entry.get_state() == ComplianceRuleEntry::State::PASSIVE) {
      if(m_entry.get_schema().get_name() == PER_ACCOUNT_RULE_NAME) {
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
  m_connections.add(
    m_model->ConnectComplianceRuleEntryIdUpdatedSignal(
      std::bind_front(&ComplianceRuleEntryWidget::OnEntryIdChanged, this)));
  m_connections.add(m_ui->m_expandButton->ConnectExpandedSignal(
    std::bind_front(&ComplianceRuleEntryWidget::OnTableExpanded, this)));
  m_connections.add(m_ui->m_expandButton->ConnectCollapsedSignal(
    std::bind_front(&ComplianceRuleEntryWidget::OnTableCollapsed, this)));
  SetupParameters();
}

const ComplianceRuleEntry& ComplianceRuleEntryWidget::GetEntry() const {
  return m_entry;
}

bool ComplianceRuleEntryWidget::IsSelected() const {
  return m_ui->m_ruleLabel->isChecked();
}

void ComplianceRuleEntryWidget::Commit() {
  auto state = [&] {
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
  auto isConsolidated = [&] {
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
  auto hasUpdate = state != m_entry.get_state();
  auto parameters = std::vector<ComplianceParameter>();
  for(auto i = 0; i < m_ui->m_parametersLayout->rowCount(); ++i) {
    auto& name = m_flattenedEntry.get_schema().get_parameters()[i].m_name;
    auto& widget =
      *m_ui->m_parametersLayout->itemAt(i, QFormLayout::FieldRole)->widget();
    auto value = GetComplianceValue(widget);
    parameters.push_back(ComplianceParameter(name, std::move(value)));
  }
  if(parameters != m_flattenedEntry.get_schema().get_parameters()) {
    hasUpdate = true;
    m_flattenedEntry = ComplianceRuleEntry(m_flattenedEntry.get_id(),
      m_flattenedEntry.get_directory_entry(), m_flattenedEntry.get_state(),
      ComplianceRuleSchema(
        m_flattenedEntry.get_schema().get_name(), std::move(parameters)));
  }
  auto schema = [&] {
    if(isConsolidated) {
      if(m_entry.get_schema().get_name() == PER_ACCOUNT_RULE_NAME) {
        hasUpdate = true;
        return Renest(
          m_flattenedEntry.get_schema(), unwrap(m_entry.get_schema()));
      }
      return Renest(m_flattenedEntry.get_schema(), m_entry.get_schema());
    }
    if(m_entry.get_schema().get_name() == PER_ACCOUNT_RULE_NAME) {
      return Renest(m_flattenedEntry.get_schema(), m_entry.get_schema());
    }
    hasUpdate = true;
    return make_per_account_compliance_rule_schema(
      Renest(m_flattenedEntry.get_schema(), m_entry.get_schema()));
  }();
  m_entry = ComplianceRuleEntry(
    m_entry.get_id(), m_entry.get_directory_entry(), state, std::move(schema));
  m_flattenedEntry = Flatten(m_entry);
  if(hasUpdate) {
    m_model->Update(m_entry);
  }
}

void ComplianceRuleEntryWidget::SetupParameters() {
  for(auto& parameter : m_flattenedEntry.get_schema().get_parameters()) {
    auto parameterWidget = MakeComplianceValueWidget(parameter.m_value,
      m_isReadOnly, Ref(*m_userProfile));
    SetComplianceValue(*parameterWidget, parameter.m_value);
    m_ui->m_parametersLayout->addRow(
      QString::fromStdString(parameter.m_name + ": "), parameterWidget);
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
    m_flattenedEntry = Flatten(m_entry);
  }
}

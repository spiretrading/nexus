#ifndef SPIRE_COMPLIANCERULEENTRYWIDGET_HPP
#define SPIRE_COMPLIANCERULEENTRYWIDGET_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <QWidget>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_ComplianceRuleEntryWidget;

namespace Spire {

  /*! \class ComplianceRuleEntryWidget
      \brief Displays a single ComplianceRuleEntry.
   */
  class ComplianceRuleEntryWidget : public QWidget {
    public:

      //! Constructs a ComplianceRuleEntryWidget.
      /*!
        \param userProfile The user's profile.
        \param isReadOnly Whether the entitlement is strictly for display
               purposes.
        \param complianceRuleEntry The ComplianceRuleEntry to display.
        \param model The model the entry belongs to.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ComplianceRuleEntryWidget(Beam::Ref<UserProfile> userProfile,
        bool isReadOnly, const Nexus::Compliance::ComplianceRuleEntry& entry,
        std::shared_ptr<ComplianceModel> model, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ComplianceRuleEntryWidget();

      //! Returns the ComplianceRuleEntry represented by this widget.
      const Nexus::Compliance::ComplianceRuleEntry& GetEntry() const;

      //! Returns <code>true</code> iff this widget's checkbox has been
      //! selected.
      bool IsSelected() const;

      //! Commits changes to the model.
      void Commit();

    private:
      std::unique_ptr<Ui_ComplianceRuleEntryWidget> m_ui;
      UserProfile* m_userProfile;
      bool m_hasModifications;
      bool m_isReadOnly;
      bool m_idUpdated;
      Nexus::Compliance::ComplianceRuleEntry m_entry;
      std::string m_ruleName;
      std::vector<std::string> m_parameterNames;
      std::shared_ptr<ComplianceModel> m_model;
      Beam::SignalHandling::ConnectionGroup m_connections;

      void SetupParameters();
      void OnTableExpanded();
      void OnTableCollapsed();
      void OnEntryIdChanged(Nexus::Compliance::ComplianceRuleId previousId,
        Nexus::Compliance::ComplianceRuleId newId);
  };
}

#endif

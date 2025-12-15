#ifndef SPIRE_COMPLIANCERULEENTRYWIDGET_HPP
#define SPIRE_COMPLIANCERULEENTRYWIDGET_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <QWidget>
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"

class Ui_ComplianceRuleEntryWidget;

namespace Spire {
  class UserProfile;

  /** Displays a single ComplianceRuleEntry. */
  class ComplianceRuleEntryWidget : public QWidget {
    public:

      /**
       * Constructs a ComplianceRuleEntryWidget.
       * @param userProfile The user's profile.
       * @param isReadOnly Whether the entitlement is strictly for display
       *        purposes.
       * @param complianceRuleEntry The ComplianceRuleEntry to display.
       * @param model The model the entry belongs to.
       * @param parent The parent widget.
       * @param flags Qt flags passed to the parent widget.
       */
      ComplianceRuleEntryWidget(Beam::Ref<UserProfile> userProfile,
        bool isReadOnly, const Nexus::ComplianceRuleEntry& entry,
        std::shared_ptr<ComplianceModel> model, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      ~ComplianceRuleEntryWidget() override = default;

      /** Returns the ComplianceRuleEntry represented by this widget. */
      const Nexus::ComplianceRuleEntry& GetEntry() const;

      /**
       * Returns <code>true</code> iff this widget's checkbox has been
       * selected.
       */
      bool IsSelected() const;

      /** Commits changes to the model. */
      void Commit();

    private:
      std::unique_ptr<Ui_ComplianceRuleEntryWidget> m_ui;
      UserProfile* m_userProfile;
      bool m_hasModifications;
      bool m_isReadOnly;
      bool m_idUpdated;
      Nexus::ComplianceRuleEntry m_entry;
      Nexus::ComplianceRuleEntry m_flattenedEntry;
      std::shared_ptr<ComplianceModel> m_model;
      Beam::ConnectionGroup m_connections;

      void SetupParameters();
      void OnTableExpanded();
      void OnTableCollapsed();
      void OnEntryIdChanged(Nexus::ComplianceRuleEntry::Id previousId,
        Nexus::ComplianceRuleEntry::Id newId);
  };
}

#endif

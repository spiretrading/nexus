#ifndef SPIRE_COMPLIANCEWIDGET_HPP
#define SPIRE_COMPLIANCEWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Nexus/Compliance/Compliance.hpp"
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_ComplianceWidget;

namespace Spire {

  /*! \class ComplianceWidget
      \brief Shows an account's compliance rules.
   */
  class ComplianceWidget : public QWidget {
    public:

      //! Constructs a ComplianceWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ComplianceWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ComplianceWidget();

      //! Sets the user's profile.
      /*!
        \param userProfile The user's profile.
        \param isReadOnly <code>true</code> iff the model is being shown
               strictly for display purposes.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile, bool isReadOnly);

      //! Returns the model being displayed.
      const ComplianceModel& GetModel() const;

      //! Returns the model being displayed.
      ComplianceModel& GetModel();

      //! Sets the model that this widget displays.
      /*!
        \param model The model to display.
      */
      void SetModel(const std::shared_ptr<ComplianceModel>& model);

      //! Commits changes to the model.
      void Commit();

    private:
      std::unique_ptr<Ui_ComplianceWidget> m_ui;
      bool m_isReadOnly;
      UserProfile* m_userProfile;
      std::shared_ptr<ComplianceModel> m_model;
      boost::signals2::scoped_connection m_entryAddedConnection;
      boost::signals2::scoped_connection m_entryRemovedConnection;

      void OnNewRuleActivated(int index);
      void OnEntryAdded(const Nexus::Compliance::ComplianceRuleEntry& entry);
      void OnEntryRemoved(const Nexus::Compliance::ComplianceRuleEntry& entry);
      void OnDeleteRules();
  };
}

#endif

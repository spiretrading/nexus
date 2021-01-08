#ifndef SPIRE_GROUPDIRECTORYVIEWWIDGET_HPP
#define SPIRE_GROUPDIRECTORYVIEWWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_GroupDirectoryViewWidget;

namespace Spire {

  /*! \class GroupDirectoryViewWidget
      \brief Displays trading related information about a DirectoryEntry.
   */
  class GroupDirectoryViewWidget : public QWidget {
    public:

      //! Constructs a GroupDirectoryViewWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      GroupDirectoryViewWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~GroupDirectoryViewWidget();

      //! Initializes the GroupDirectoryViewWidget.
      /*!
        \param userProfile The user's profile.
        \param isReadOnly <code>true</code> iff the models being shown are
               strictly for display purposes.
        \param directoryEntry The DirectoryEntry being displayed.
        \param complianceModel The ComplianceModel to display.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile, bool isReadOnly,
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry,
        std::shared_ptr<ComplianceModel> complianceModel);

      //! Removes the tab displaying the profit and loss.
      void RemoveActivityReportTab();

      //! Commits all displayed info into their respective models.
      void Commit();

      //! Reloads all models.
      void Reload();

    private:
      std::unique_ptr<Ui_GroupDirectoryViewWidget> m_ui;
  };
}

#endif

#ifndef SPIRE_GROUPDIRECTORYITEMWIDGET_HPP
#define SPIRE_GROUPDIRECTORYITEMWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <QWidget>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/UI/UI.hpp"

class Ui_GroupDirectoryItemWidget;

namespace Spire {

  /*! \class GroupDirectoryItemWidget
      \brief Shows the main widget used to display a DirectoryItem for a group.
   */
  class GroupDirectoryItemWidget : public QWidget {
    public:

      //! Constructs a GroupDirectoryItemWidget.
      /*!
        \param userProfile The user's profile.
        \param directoryEntry The DirectoryEntry to display.
        \param complianceModel The model used to display compliance rules.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      GroupDirectoryItemWidget(Beam::Ref<UserProfile> userProfile,
        const Beam::DirectoryEntry& directoryEntry,
        std::shared_ptr<ComplianceModel> complianceModel,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~GroupDirectoryItemWidget();

      //! Removes the tab displaying the profit and loss.
      void RemoveActivityReportTab();

    private:
      std::unique_ptr<Ui_GroupDirectoryItemWidget> m_ui;

      void OnApply();
      void OnReload();
  };
}

#endif

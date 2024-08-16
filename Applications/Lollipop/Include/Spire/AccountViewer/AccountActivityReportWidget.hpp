#ifndef SPIRE_ACCOUNTACTIVITYREPORTWIDGET_HPP
#define SPIRE_ACCOUNTACTIVITYREPORTWIDGET_HPP
#include <optional>
#include <QWidget>
#include <Beam/Queues/SequencePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Blotter/ProfitAndLossModel.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_AccountActivityReportWidget;

namespace Spire {

  /*! \class AccountActivityReportWidget
      \brief Shows an account's activity report.
   */
  class AccountActivityReportWidget : public QWidget {
    public:

      //! Constructs an AccountActivityReportWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      AccountActivityReportWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~AccountActivityReportWidget();

      //! Initializes this widget.
      /*!
        \param userProfile The user's profile.
        \param account The account to display.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        const Beam::ServiceLocator::DirectoryEntry& account);

    private:
      struct ReportModel {
        ProfitAndLossModel m_profitAndLossModel;
        SpirePortfolioController m_portfolioController;

        ReportModel(Beam::Ref<UserProfile> userProfile,
          Beam::ScopedQueueReader<const Nexus::OrderExecutionService::Order*>
          orders);
      };
      std::unique_ptr<Ui_AccountActivityReportWidget> m_ui;
      UserProfile* m_userProfile;
      Beam::ServiceLocator::DirectoryEntry m_account;
      std::optional<ReportModel> m_model;

      void OnUpdate(bool checked);
  };
}

#endif

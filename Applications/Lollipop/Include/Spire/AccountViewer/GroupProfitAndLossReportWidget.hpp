#ifndef SPIRE_GROUPPROFITANDLOSSREPORTWIDGET_HPP
#define SPIRE_GROUPPROFITANDLOSSREPORTWIDGET_HPP
#include <optional>
#include <vector>
#include <QWidget>
#include <Beam/Queues/ScopedQueueReader.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/AccountViewer/AccountViewer.hpp"
#include "Spire/Blotter/ProfitAndLossModel.hpp"
#include "Spire/UI/UI.hpp"

class Ui_GroupProfitAndLossReportWidget;

namespace Spire {

  /*! \class GroupProfitAndLossReportWidget
      \brief Shows a group's profit and loss.
   */
  class GroupProfitAndLossReportWidget : public QWidget {
    public:

      //! Constructs a GroupProfitAndLossReportWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      GroupProfitAndLossReportWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~GroupProfitAndLossReportWidget();

      //! Initializes this widget.
      /*!
        \param userProfile The user's profile.
        \param group The group to display.
      */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        const Beam::DirectoryEntry& group);

    private:
      struct ReportModel {
        ProfitAndLossModel m_profitAndLossModel;
        ProfitAndLossModel::PortfolioController m_portfolioController;

        ReportModel(Beam::Ref<UserProfile> userProfile,
          Beam::ScopedQueueReader<std::shared_ptr<Nexus::Order>> orders);
      };
      std::unique_ptr<Ui_GroupProfitAndLossReportWidget> m_ui;
      UserProfile* m_userProfile;
      Beam::DirectoryEntry m_group;
      std::optional<ReportModel> m_totalsModel;
      std::vector<std::unique_ptr<ReportModel>> m_groupModels;

      void OnUpdate(bool checked);
  };
}

#endif

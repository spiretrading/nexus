#ifndef SPIRE_PORTFOLIOVIEWERWINDOW_HPP
#define SPIRE_PORTFOLIOVIEWERWINDOW_HPP
#include <QFrame>
#include "Spire/PortfolioViewer/PortfolioViewer.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerModel.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/WindowSettings.hpp"

class QStatusBar;
class Ui_PortfolioViewerWindow;

namespace Spire {

  /*! \class PortfolioViewerWindow
      \brief Displays the portfolio's viewable by a manager's account.
   */
  class PortfolioViewerWindow : public QFrame, public UI::PersistentWindow {
    public:

      //! Constructs a PortfolioViewerWindow.
      /*!
        \param userProfile The user's profile.
        \param properties The properties used to display the portfolio.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      PortfolioViewerWindow(Beam::Ref<UserProfile> userProfile,
        const PortfolioViewerProperties& properties, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~PortfolioViewerWindow();

      //! Sets the properties used to display the portfolio.
      /*!
        \param properties The properties used to display the portfolio.
      */
      void SetProperties(const PortfolioViewerProperties& properties);

      virtual std::unique_ptr<UI::WindowSettings> GetWindowSettings() const;

    protected:
      virtual void showEvent(QShowEvent* event);
      virtual void closeEvent(QCloseEvent* event);

    private:
      friend class PortfolioViewerWindowSettings;
      std::unique_ptr<Ui_PortfolioViewerWindow> m_ui;
      UserProfile* m_userProfile;
      mutable PortfolioViewerProperties m_properties;
      QStatusBar* m_statusBar;
      UI::ValueLabel* m_totalProfitAndLossLabel;
      UI::ValueLabel* m_unrealizedProfitAndLossLabel;
      UI::ValueLabel* m_realizedProfitAndLossLabel;
      UI::ValueLabel* m_feesLabel;
      UI::ValueLabel* m_volumeLabel;
      UI::ValueLabel* m_tradesLabel;
      std::unique_ptr<PortfolioViewerModel> m_viewerModel;
      std::unique_ptr<UI::CustomVariantSortFilterProxyModel> m_proxyViewerModel;
      std::unique_ptr<PortfolioSelectionModel> m_selectionModel;
      boost::signals2::scoped_connection m_totalsUpdatedConnection;

      void OnTotalsUpdated(const PortfolioViewerModel::TotalEntry& totals);
      void OnContextMenu(const QPoint& position);
  };
}

#endif

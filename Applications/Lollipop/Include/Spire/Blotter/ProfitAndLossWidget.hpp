#ifndef SPIRE_PROFITANDLOSSWIDGET_HPP
#define SPIRE_PROFITANDLOSSWIDGET_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/Spire.hpp"

class Ui_ProfitAndLossWidget;

namespace Spire {

  /*! \class ProfitAndLossWidget
      \brief Displays a blotter's profit/loss.
   */
  class ProfitAndLossWidget : public QWidget {
    public:

      //! Constructs a ProfitAndLossWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ProfitAndLossWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ProfitAndLossWidget();

      //! Sets the BlotterModel this widget is representing.
      /*!
        \param userProfile The user's profile.
        \param model The model to represent.
      */
      void SetModel(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<ProfitAndLossModel> model);

    private:
      std::unique_ptr<Ui_ProfitAndLossWidget> m_ui;
      UserProfile* m_userProfile;
      boost::signals2::scoped_connection m_modelAddedConnection;
      boost::signals2::scoped_connection m_modelRemovedConnection;

      void OnModelAdded(ProfitAndLossEntryModel& model);
      void OnModelRemoved(ProfitAndLossEntryModel& model);
  };
}

#endif

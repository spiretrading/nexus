#ifndef SPIRE_PROFITANDLOSSENTRY_HPP
#define SPIRE_PROFITANDLOSSENTRY_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /*! \class ProfitAndLossEntry
      \brief Displays the profit and loss for a single currency.
   */
  class ProfitAndLossEntry : public QWidget {
    public:

      //! Constructs a ProfitAndLossEntry.
      /*!
        \param userProfile The user's profile.
        \param model The profit and loss model to represent.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ProfitAndLossEntry(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<ProfitAndLossEntryModel> model,
        QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ProfitAndLossEntry();

      //! Returns the model being displayed.
      const ProfitAndLossEntryModel& GetModel() const;

      //! Returns the model being displayed.
      ProfitAndLossEntryModel& GetModel();

    private:
      ProfitAndLossEntryModel* m_model;
      ProfitAndLossHeader* m_header;
      ProfitAndLossTable* m_table;
      boost::signals2::scoped_connection m_expandedConnection;
      boost::signals2::scoped_connection m_collapsedConnection;

      void OnTableExpanded();
      void OnTableCollapsed();
  };
}

#endif

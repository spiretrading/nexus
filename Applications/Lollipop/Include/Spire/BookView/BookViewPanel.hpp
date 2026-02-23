#ifndef SPIRE_BOOK_VIEW_PANEL_HPP
#define SPIRE_BOOK_VIEW_PANEL_HPP
#include <optional>
#include <QMetaObject>
#include <QWidget>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UI.hpp"

class QModelIndex;
class QTableView;
class Ui_BookViewPanel;

namespace Spire {

  /** Displays one Side of a Ticker's book. */
  class BookViewPanel : public QWidget {
    public:

      /**
       * Constructs a BookViewPanel.
       * @param parent The parent widget.
       * @param flags The flags passed to the <i>parent</i> widget.
       */
      explicit BookViewPanel(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /**
       * Initializes this BookViewPanel.
       * @param userProfile The user's profile.
       * @param properties The BookViewProperties used to display this panel.
       * @param side The Side to display.
       */
      void Initialize(Beam::Ref<UserProfile> userProfile,
        const BookViewProperties& properties, Nexus::Side side);

      /** Returns the best Quote currently displayed. */
      const Nexus::Quote& GetBestQuote() const;

      /** Returns the table displaying the Quotes. */
      QTableView& GetQuoteList();

      /** Sets the BookViewProperties used to display this panel. */
      void SetProperties(const BookViewProperties& properties);

      /**
       * Sets the Ticker to display.
       * @param ticker The Ticker to display.
       */
      void DisplayTicker(const Nexus::Ticker& ticker);

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      friend class BookViewWindowSettings;
      std::unique_ptr<Ui_BookViewPanel> m_ui;
      UserProfile* m_userProfile;
      std::optional<UI::CustomVariantItemDelegate> m_itemDelegate;
      BookViewProperties m_properties;
      Nexus::Ticker m_ticker;
      Nexus::Side m_side;
      Nexus::Quote m_bestQuote;
      std::unique_ptr<BookViewModel> m_model;
      int m_topRow;
      int m_currentRow;
      QMetaObject::Connection m_rowsAboutToBeModifiedConnection;
      QMetaObject::Connection m_rowsModifiedConnection;
      QMetaObject::Connection m_rowsAboutToBeRemovedConnection;
      QMetaObject::Connection m_rowsRemovedConnection;
      std::optional<EventHandler> m_eventHandler;

      void ConnectModel();
      void DisconnectModel();
      void OnBbo(const Nexus::Ticker& ticker, const Nexus::BboQuote& bbo);
      void OnRowsAboutToBeModified(
        const QModelIndex& parent, int start, int end);
      void OnRowsModified(const QModelIndex& parent, int start, int end);
  };
}

#endif

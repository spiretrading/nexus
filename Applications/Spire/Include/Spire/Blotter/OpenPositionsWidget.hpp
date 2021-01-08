#ifndef SPIRE_OPENPOSITIONSWIDGET_HPP
#define SPIRE_OPENPOSITIONSWIDGET_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <QMetaObject>
#include <QWidget>
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/UI.hpp"

class QItemSelection;
class Ui_OpenPositionsWidget;

namespace Spire {

  /*! \class OpenPositionsWidget
      \brief Displays a blotter's open positions.
   */
  class OpenPositionsWidget : public QWidget {
    public:

      /*! \struct UIState
          \brief Stores the user interface state of this widget.
       */
      struct UIState {

        //! The geometry of the open positions table header.
        QByteArray m_tableGeometry;

        //! The state of the open positions table header.
        QByteArray m_tableState;

        template<typename Shuttler>
        void Shuttle(Shuttler& shuttle, unsigned int version);
      };

      //! Constructs a OpenPositionsWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      OpenPositionsWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      ~OpenPositionsWidget() = default;

      //! Returns the current UI state.
      UIState GetUIState() const;

      //! Sets the UI state.
      /*!
        \param state The UIState to set.
      */
      void SetUIState(const UIState& state);

      //! Sets the BlotterModel this widget is representing.
      /*!
        \param userProfile The user's profile.
        \param model The model to represent.
      */
      void SetModel(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<BlotterModel> model);

    private:
      std::unique_ptr<Ui_OpenPositionsWidget> m_ui;
      UserProfile* m_userProfile;
      BlotterModel* m_model;
      std::unique_ptr<UI::CustomVariantSortFilterProxyModel> m_proxyModel;
      QMetaObject::Connection m_selectionChangedConnection;

      void OnContextMenu(const QPoint& position);
      void OnFlattenAll();
      void OnFlattenSelection();
      void OnSelectionChanged(const QItemSelection& selected,
        const QItemSelection& deselected);
  };

  template<typename Shuttler>
  void OpenPositionsWidget::UIState::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("table_geometry", m_tableGeometry);
    shuttle.Shuttle("table_state", m_tableState);
  }
}

#endif

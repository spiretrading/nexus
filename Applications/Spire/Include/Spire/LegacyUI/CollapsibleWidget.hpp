#ifndef SPIRE_COLLAPSIBLEWIDGET_HPP
#define SPIRE_COLLAPSIBLEWIDGET_HPP
#include <boost/signals2/connection.hpp>
#include <QWidget>
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"

class QLineEdit;
class QBoxLayout;

namespace Spire {
namespace LegacyUI {

  /*! \class CollapsibleWidget
      \brief Displays a widget within a collapsible group with a specified
             header.
   */
  class CollapsibleWidget : public QWidget, public LegacyUI::PersistentWindow {
    public:

      //! Constructs a CollapsibleWidget.
      /*!
        \param parent The parent widget.
      */
      CollapsibleWidget(QWidget* parent = nullptr);

      //! Constructs a CollapsibleWidget.
      /*!
        \param label The header's label.
        \param widget The widget to display.
        \param expanded Whether the initial state of this group is expanded.
        \param parent The parent widget.
      */
      CollapsibleWidget(const QString& label, QWidget* widget, bool expanded,
        QWidget* parent = nullptr);

      //! Initializes this widget.
      /*!
        \param label The header's label.
        \param widget The widget to display.
        \param expanded Whether the initial state of this group is expanded.
      */
      void Initialize(const QString& label, QWidget* widget, bool expanded);

      //! Returns <code>true</code> iff this widget is expanded.
      bool IsExpanded() const;

      //! Sets the expanded/collapsed state of this widget.
      /*!
        \param expanded The expand state to set.
        \param adjustSize Whether the size of the parent widget should be
               adjusted.
      */
      void SetExpanded(bool expanded, bool adjustSize);

      //! Sets the label to display in the header.
      /*!
        \param label The label to display in the header.
      */
      void SetLabel(const QString& label);

      //! Sets the widget to display.
      /*!
        \param widget The widget to display.
      */
      void SetWidget(QWidget* widget);

      virtual std::unique_ptr<WindowSettings> GetWindowSettings() const;

    private:
      friend class CollapsibleWidgetWindowSettings;
      ExpandButton* m_expandButton;
      QLineEdit* m_header;
      QBoxLayout* m_layout;
      QWidget* m_widget;
      boost::signals2::scoped_connection m_expandedConnection;
      boost::signals2::scoped_connection m_collapsedConnection;

      void Expand(bool adjustSize);
      void Collapse(bool adjustSize);
      void SetupLayouts(bool expanded);
      void OnCollapse();
      void OnExpand();
  };
}
}

#endif

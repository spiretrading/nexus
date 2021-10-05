#ifndef SPIRE_NAVIGATION_VIEW_HPP
#define SPIRE_NAVIGATION_VIEW_HPP
#include <any>
#include <QStackedWidget>
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Represents a component which holds the NavigationList and associated page
   * components.
   */
  class NavigationView : public QWidget {
    public:

      /**
       * A ValueModel over an integer to represent the current of the
       * NavigationView.
       */
      using CurrentModel = ValueModel<int>;

      /**
       * A LocalValueModel over an integer to represent the local
       * current of the NavigationView.
       */
      using LocalCurrentModel = LocalValueModel<int>;

      /**
       * Constructs a NavigationView using a LocalCurrentModel.
       * @param parent The parent widget.
       */
      explicit NavigationView(QWidget* parent = nullptr);

      /**
       * Constructs a NavigationView.
       * @param current_model The current value's model.
       * @param parent The parent widget.
       */
      explicit NavigationView(std::shared_ptr<CurrentModel> current_model,
        QWidget* parent = nullptr);

      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current_model() const;

      /**
       * Appends a tab with a page and label.
       * @param page The component displayed in the page area.
       * @param tab_label The text value associated with the page.
       */
      void add_tab(QWidget& page, const QString& tab_label);

      /**
       * Inserts a tab with a page and label at the specified index.
       * @param index The index to insert the tab at.
       * @param page The component displayed in the page area.
       * @param tab_label The text value associated with the page.
       * @throws <code>std::out_of_range</code> -
       *         <code>index < 0 or index > get_tab_count()</code>.
       */
      void insert_tab(int index, QWidget& page, const QString& tab_label);

      /** Gets the number of tabs in the NavigationView. */
      int get_tab_count() const;

      /**
       * Gets the label of the tab at the specified index.
       * @param index The index to get.
       * @return The label of the tab at the specified index.
       * @throws <code>std::out_of_range</code> -
       *         <code>index < 0 or index >= get_tab_count()</code>.
       */
      QString get_tab_label(int index) const;

      /**
       * Gets the tab page at the specified index.
       * @param index The index to get.
       * @return The tab page at the specified index.
       * @throws <code>std::out_of_range</code> -
       *         <code>index < 0 or index >= get_tab_count()</code>.
       */
      QWidget& get_page(int index) const;

      /**
       * Returns <code>true</code> iff the tab at the specified index is enabled.
       * @throws <code>std::out_of_range</code> -
       *         <code>index < 0 or index >= get_tab_count()</code>.
       */
      bool is_tab_enabled(int index) const;

      /**
       * Sets whether the tab at the specified index is enabled.
       * @param index The index to set.
       * @param is_enabled True iff the tab should be enabled.
       * @throws <code>std::out_of_range</code> -
       *         <code>index < 0 or index >= get_tab_count()</code>.
       */
      void set_tab_enabled(int index, bool is_enabled);

    private:
      class SelectLine;
      class Tab;
      class LabelContainer;
      class NavigationTab;
      class Separator;
      std::shared_ptr<CurrentModel> m_current_model;
      ListView* m_navigation_list;
      QStackedWidget* m_stacked_widget;
      AssociativeValueModel<QString> m_associative_model;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(int index);
      void on_list_submit(const std::any& submission);
      void on_list_current(const boost::optional<int>& current);
      void on_associative_value_current(int index, bool value);
  };
}

#endif

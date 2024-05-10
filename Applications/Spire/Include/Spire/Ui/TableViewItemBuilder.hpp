#ifndef SPIRE_TABLE_VIEW_ITEM_BUILDER_HPP
#define SPIRE_TABLE_VIEW_ITEM_BUILDER_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <QWidget>
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Defines the requirements of a type compatible with the
   * TableViewItemBuilder.
   */
  template<typename T>
  concept TableViewItemBuilderConcept = requires(T builder) {
    { builder.mount(std::shared_ptr<TableModel>(), int(), int()) };
    { builder.unmount(static_cast<QWidget*>(nullptr), int(), int()) };
  };

  /**
   * Builder class used to build QWidget's used by a TableView to represent the
   * elements of a TableModel.
   */
  class TableViewItemBuilder {
    public:

      /**
       * Constructs a TableViewItemBuilder that calls a function to build a new
       * QWidget for every element, and calls delete to unmount widgets.
       * @param builder The function to call for every element of the table.
       */
      TableViewItemBuilder(std::function<
        QWidget* (const std::shared_ptr<TableModel>&, int, int)> builder);

      template<TableViewItemBuilderConcept B>
      TableViewItemBuilder(B&& builder) requires
        !std::is_same_v<std::remove_cvref_t<B>, TableViewItemBuilder>;

      TableViewItemBuilder(const TableViewItemBuilder&) = default;

      TableViewItemBuilder(TableViewItemBuilder&&) = default;

      /**
       * Returns the QWidget to represent a table element.
       * @param table The table containing the element to represent.
       * @param row The row of the element to represent.
       * @param column The column of the element to represent.
       * @return The QWidget to use to represent the element.
       */
      QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column);

      /**
       * Releases a previously mounted QWidget.
       * @param widget The QWidget to release.
       * @param row The row of the element the <i>widget</i> represented.
       * @param column The column of the element the <i>widget</i> represented.
       */
      void unmount(QWidget* widget, int row, int column);

    private:
      struct VirtualTableViewItemBuilder {
        virtual ~VirtualTableViewItemBuilder() = default;
        virtual QWidget* mount(
          const std::shared_ptr<TableModel>& table, int row, int column) = 0;
        virtual void unmount(QWidget* widget, int row, int column) = 0;
      };
      template<typename B>
      struct WrappedTableViewItemBuilder final : VirtualTableViewItemBuilder {
        using Builder = B;
        Beam::GetOptionalLocalPtr<Builder> m_builder;

        template<typename... Args>
        WrappedTableViewItemBuilder(Args&&... args);
        QWidget* mount(const std::shared_ptr<TableModel>& table, int row,
          int column) override;
        void unmount(QWidget* widget, int row, int column) override;
      };
      std::shared_ptr<VirtualTableViewItemBuilder> m_builder;
  };

  /**
   * Implements a TableViewItemBuilder that mounts using an std::function and
   * calls delete to unmount.
   */
  class FunctionTableViewItemBuilder {
    public:

      /**
       * Constructs a FunctionTableViewItemBuilder.
       * @param builder The function used to mount QWidgets.
       */
      FunctionTableViewItemBuilder(
        std::function<QWidget* (const std::shared_ptr<TableModel>&, int, int)>
          builder);

      QWidget* mount(
        const std::shared_ptr<TableModel>& table, int row, int column);

      void unmount(QWidget* widget, int row, int column);

    private:
      std::function<QWidget* (const std::shared_ptr<TableModel>&, int, int)>
        m_builder;
  };

  template<TableViewItemBuilderConcept B>
  TableViewItemBuilder::TableViewItemBuilder(B&& builder) requires
    !std::is_same_v<std::remove_cvref_t<B>, TableViewItemBuilder>
    : m_builder(std::make_shared<WrappedTableViewItemBuilder<
        std::remove_cvref_t<B>>>(std::forward<B>(builder))) {}
}

#endif

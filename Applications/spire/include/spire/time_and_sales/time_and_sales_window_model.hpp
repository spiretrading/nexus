#ifndef SPIRE_TIME_AND_SALES_WINDOW_MODEL_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_MODEL_HPP
#include <QAbstractItemModel>
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"

namespace spire {

  //! Implements the table model for displaying time and sales in a list view.
  class time_and_sales_window_model : public QAbstractTableModel {
    public:

      //! Constructs a time and sales window model.
      /*!
        \param model The base model.
        \param properties The display properties to use.
      */
      time_and_sales_window_model(std::shared_ptr<time_and_sales_model> model,
        const time_and_sales_properties& properties);

      //! Returns the security being displayed.
      const Nexus::Security& get_security() const;

      //! Sets the display properties.
      void set_properties(const time_and_sales_properties& properties);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

    private:
      std::shared_ptr<time_and_sales_model> m_model;
      time_and_sales_properties m_properties;
      std::vector<time_and_sales_model::entry> m_entries;

      void update_data(const time_and_sales_model::entry& e);
  };
}

#endif

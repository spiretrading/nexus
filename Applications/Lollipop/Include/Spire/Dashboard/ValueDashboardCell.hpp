#ifndef SPIRE_VALUEDASHBOARDCELL_HPP
#define SPIRE_VALUEDASHBOARDCELL_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"

namespace Spire {

  /*! \class ValueDashboardCell
      \brief A DashboardCell whose value is programmatically set.
   */
  class ValueDashboardCell : public DashboardCell {
    public:

      //! Constructs a ValueDashboardCell.
      ValueDashboardCell();

      //! Sets the size of the buffer.
      /*!
        \param size The size of the buffer.
      */
      void SetBufferSize(int size);

      //! Sets the value.
      /*!
        \param value The new value to represent.
      */
      void SetValue(const Value& value);

      virtual const boost::circular_buffer<Value>& GetValues() const;

      virtual boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_function_type& slot) const;

    private:
      boost::circular_buffer<Value> m_values;
      mutable UpdateSignal m_updateSignal;
  };
}

#endif

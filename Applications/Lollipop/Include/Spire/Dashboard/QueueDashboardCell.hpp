#ifndef SPIRE_QUEUEDASHBOARDCELL_HPP
#define SPIRE_QUEUEDASHBOARDCELL_HPP
#include <Beam/Queues/ConverterQueueReader.hpp>
#include <Beam/Queues/QueueReader.hpp>
#include <Beam/Utilities/Casts.hpp>
#include <QTimer>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"

namespace Spire {

  /*! \class QueueDashboardCell
      \brief A DashboardCell whose values update via a Queue.
   */
  class QueueDashboardCell : public DashboardCell {
    public:

      //! Constructs a QueueDashboardCell.
      /*!
        \param queue The queue providing the updated values.
      */
      QueueDashboardCell(std::shared_ptr<Beam::QueueReader<Value>> queue);

      //! Constructs a QueueDashboardCell.
      /*!
        \param queue The queue providing the updated values.
      */
      template<typename T>
      QueueDashboardCell(std::shared_ptr<Beam::QueueReader<T>> queue);

      //! Sets the size of the buffer.
      /*!
        \param size The size of the buffer.
      */
      void SetBufferSize(int size);

      virtual const boost::circular_buffer<Value>& GetValues() const;

      virtual boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_function_type& slot) const;

    private:
      std::shared_ptr<Beam::QueueReader<Value>> m_queue;
      boost::circular_buffer<Value> m_values;
      mutable UpdateSignal m_updateSignal;
      QTimer m_updateTimer;

      void OnUpdateTimer();
  };

  template<typename T>
  QueueDashboardCell::QueueDashboardCell(
    std::shared_ptr<Beam::QueueReader<T>> queue)
    : QueueDashboardCell(Beam::convert(queue, [] (auto&& value) {
        return static_cast<Value>(std::forward<decltype(value)>(value));
      })) {}
}

#endif

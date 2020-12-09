#ifndef SPIRE_REAL_SPIN_BOX_MODEL_HPP
#define SPIRE_REAL_SPIN_BOX_MODEL_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <QLocale>
#include "Spire/Ui/SpinBoxModel.hpp"

namespace Spire {

  //! Represents a RealSpinBox's data model.
  class RealSpinBoxModel {
    public:
      
      //! The precision of the model's floating point type.
      static constexpr auto PRECISION = 15;

      //! Numeric type used by the model.
      using Real = boost::multiprecision::cpp_dec_float<PRECISION>;

      virtual ~RealSpinBoxModel() = default;

      //! Returns an increment value.
      /*
        \param modifiers The modifiers to get the increment value of.
      */
      virtual Real get_increment(
        Qt::KeyboardModifiers modifiers) const = 0;

      //! Returns the initial value.
      virtual Real get_initial() const = 0;

      //! Returns the minimum acceptable value.
      virtual Real get_minimum() const = 0;

      //! Returns the maximum acceptable value.
      virtual Real get_maximum() const = 0;
  };

  //! Converts an integer to a Real.
  /*!
    \param value The integer to convert.
  */
  RealSpinBoxModel::Real to_real(std::int64_t value);

  //! Converts a double to a Real.
  /*!
    \param value The double to convert.
  */
  RealSpinBoxModel::Real to_real(double value);

  //! Converts a Money to a Real.
  /*!
    \param value The Money to convert.
  */
  RealSpinBoxModel::Real to_real(Nexus::Money value);

  //! Converts a Quantity to a Real.
  /*!
    \param value The Quantity to convert.
  */
  RealSpinBoxModel::Real to_real(Nexus::Quantity value);

  //! Wraps a SpinBoxModel to provide a generic interface.
  template<typename T>
  class RealSpinBoxAdapterModel : public RealSpinBoxModel {
    public:

      //! The type stored in the model.
      using Type = T;

      //! Constructs a RealSpinBoxModelAdapter.
      /*
        \param model The source model.
      */
      RealSpinBoxAdapterModel(std::shared_ptr<SpinBoxModel<Type>> model);

      RealSpinBox::Real get_increment(
        Qt::KeyboardModifiers modifiers) const override;

      RealSpinBox::Real get_initial() const override;

      RealSpinBox::Real get_minimum() const override;

      RealSpinBox::Real get_maximum() const override;

    private:
      std::shared_ptr<SpinBoxModel<Type>> m_model;
  };

  template<typename T>
  RealSpinBoxAdapterModel<T>::RealSpinBoxAdapterModel(
    std::shared_ptr<SpinBoxModel<Type>> model)
    : m_model(std::move(model)) {}

  template<typename T>
  RealSpinBoxModel::Real RealSpinBoxAdapterModel<T>::get_increment(
      Qt::KeyboardModifiers modifiers) const {
    return to_real(m_model->get_increment(modifiers));
  }

  template<typename T>
  RealSpinBoxModel::Real RealSpinBoxAdapterModel<T>::get_initial() const {
    return to_real(m_model->get_initial());
  }

  template<typename T>
  RealSpinBoxModel::Real RealSpinBoxAdapterModel<T>::get_minimum() const {
    return to_real(m_model->get_minimum());
  }

  template<typename T>
  RealSpinBoxModel::Real RealSpinBoxAdapterModel<T>::get_maximum() const {
    return to_real(m_model->get_maximum());
  }
}

#endif

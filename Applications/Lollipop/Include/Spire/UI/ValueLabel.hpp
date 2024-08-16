#ifndef SPIRE_VALUELABEL_HPP
#define SPIRE_VALUELABEL_HPP
#include <optional>
#include <string>
#include <Beam/Pointers/Ref.hpp>
#include <QLabel>
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class ValueLabel
      \brief Extends a QLabel to display a formatted label and a value.
   */
  class ValueLabel : public QLabel {
    public:

      //! Constructs a ValueLabel.
      /*!
        \param parent The parent widget.
        \param flags The flags passed to the <i>parent</i> widget.
      */
      ValueLabel(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      //! Constructs a ValueLabel.
      /*!
        \param label The label to display.
        \param userProfile The user's profile.
      */
      ValueLabel(const std::string& label,
        Beam::Ref<UserProfile> userProfile);

      //! Initializes this ValueLabel.
      /*!
        \param label The label to display.
        \param userProfile The user's profile.
      */
      void Initialize(const std::string& label,
        Beam::Ref<UserProfile> userProfile);

      //! Adjusts the size of this label to fit a value.
      /*!
        \param value The value to adjust the size to.
      */
      void AdjustSize(const QVariant& value);

      //! Sets the value to display.
      /*!
        \param value The value to display.
      */
      void SetValue(const QVariant& value);

      //! Resets the displayed value.
      void Reset();

    private:
      std::string m_label;
      std::optional<CustomVariantItemDelegate> m_converter;
  };
}
}

#endif

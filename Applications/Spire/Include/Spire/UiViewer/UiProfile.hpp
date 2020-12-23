#ifndef SPIRE_UI_PROFILE_HPP
#define SPIRE_UI_PROFILE_HPP
#include <functional>
#include <memory>
#include <vector>
#include <QWidget>
#include "Spire/UiViewer/UiProperty.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  //! Stores the information needed to display a single widget in the UI Viewer.
  class UiProfile {
    public:

      //! Constructs the profile for a widget.
      /*!
        \param name The name of the widget.
        \param properties The widget's properties.
        \param factory The function used to construct the widget.
      */
      UiProfile(QString name,
        std::vector<std::shared_ptr<UiProperty>> properties,
        std::function<QWidget* (const UiProfile&)>);

      UiProfile(UiProfile&&) = default;

      //! Returns the name of the widget.
      const QString& get_name() const;

      //! Returns the widget's properties.
      const std::vector<std::shared_ptr<UiProperty>>& get_properties() const;

      //! Returns the widget to display.
      QWidget* get_widget() const;

      //! Resets the widget.
      QWidget* reset();

      UiProfile& operator =(UiProfile&&) = default;

    private:
      QString m_name;
      std::vector<std::shared_ptr<UiProperty>> m_properties;
      std::function<QWidget* (const UiProfile&)> m_factory;
      QWidget* m_widget;

      UiProfile(const UiProfile&) = delete;
      UiProfile& operator =(const UiProfile&) = delete;
  };
}

#endif

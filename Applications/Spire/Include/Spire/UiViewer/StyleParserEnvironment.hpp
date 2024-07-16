#ifndef SPIRE_STYLE_PARSER_ENVIRONMENT_HPP
#define SPIRE_STYLE_PARSER_ENVIRONMENT_HPP

namespace Spire {

  /** Registers all selectors used in UiViewer. */
  void register_selectors();

  /** Registers all property and function converters used in UiViewer. */
  void register_property_converters();
}

#endif

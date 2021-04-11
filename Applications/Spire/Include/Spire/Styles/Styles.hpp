#ifndef SPIRE_STYLES_HPP
#define SPIRE_STYLES_HPP

namespace Spire::Styles {
  class AncestorSelector;
  class AndSelector;
  class Any;
  template<typename T, typename G> class BasicProperty;
  class Block;
  class ChildSelector;
  template<typename... T> class CompositeProperty;
  class DescendantSelector;
  class DisambiguateSelector;
  template<typename T> class Expression;
  class IsASelector;
  class NotSelector;
  class OrSelector;
  class ParentSelector;
  class Property;
  class PseudoElement;
  template<typename T, typename G> class PseudoElementSelector;
  class Rule;
  class Selector;
  class SelectorRegistry;
  class SiblingSelector;
  template<typename T, typename G> class StateSelector;
  class StyleSheet;
  class Stylist;
}

#endif

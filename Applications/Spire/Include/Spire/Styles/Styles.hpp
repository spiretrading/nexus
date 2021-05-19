#ifndef SPIRE_STYLES_HPP
#define SPIRE_STYLES_HPP

namespace Spire::Styles {
  class AncestorSelector;
  class AndSelector;
  class Any;
  template<typename T, typename G> class BasicProperty;
  class Block;
  class ChildSelector;
  template<typename T, typename G> class ComponentId;
  class ComponentSelector;
  template<typename... T> class CompositeProperty;
  class DescendantSelector;
  template<typename T> struct Evaluation;
  template<typename T> class Expression;
  class FlipSelector;
  class IsASelector;
  class NotSelector;
  class OrSelector;
  class ParentSelector;
  class PathSelector;
  class Property;
  class PropertyMatchSelector;
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

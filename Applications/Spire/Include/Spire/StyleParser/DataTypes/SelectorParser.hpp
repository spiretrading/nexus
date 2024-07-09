#ifndef SPIRE_SELECTOR_PARSER_HPP
#define SPIRE_SELECTOR_PARSER_HPP
#include "Spire/StyleParser/DataTypes/TokenParser.hpp"
#include "Spire/Styles/Selector.hpp"

namespace Spire {

  /** Represents an abstract selector parser. */
  class SelectorParser {
    public:

      virtual ~SelectorParser() = default;

      /**
       * Parses the value to a Selector.
       * @throws <code>std::runtime_error</code> iff the value can't be
       *         interpreted.
       */
      virtual Styles::Selector parse() = 0;
  };

  /** Represents a selector parser for the AnySelector. */
  class AnySelectorParser : public SelectorParser {
    public:

      Styles::Selector parse() override;
  };

  /** Represents a selector parser for the IsASelector. */
  class TypeSelectorParser : public SelectorParser {
    public:

      /** Constructs the TypeSelectorParser. */
      explicit TypeSelectorParser(Identifier identifier);

      Styles::Selector parse() override;

    private:
      Identifier m_identifier;
  };

  /** Represents a selector parser for the StateSelector. */
  class StateSelectorParser : public SelectorParser {
    public:

      /** Constructs the StateSelectorParser. */
      explicit StateSelectorParser(Identifier identifier);

      Styles::Selector parse() override;

    private:
      Identifier m_identifier;
  };

  /** Represents a selector parser for the PseudoElement. */
  class PseudoElementParser : public SelectorParser {
    public:

      /** Constructs the PseudoElementParser. */
      explicit PseudoElementParser(Identifier identifier);

      Styles::Selector parse() override;

    private:
      Identifier m_identifier;
  };

  /** Represents a selector parser for the PropertyMatchSelector. */
  class PropertyMatchSelectorParser : public SelectorParser {
    public:

      /** Constructs the PropertyMatchSelectorParser. */
      PropertyMatchSelectorParser(Keyword property, Keyword value);

      Styles::Selector parse() override;

    private:
      Keyword m_property;
      Keyword m_value;
  };

  /** Represents a selector parser for the ChildSelector. */
  class ChildSelectorParser : public SelectorParser {
    public:

      /** Constructs the ChildSelectorParser. */
      ChildSelectorParser(std::shared_ptr<SelectorParser> base,
        std::shared_ptr<SelectorParser> child);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_base;
      std::shared_ptr<SelectorParser> m_child;
  };

  /** Represents a selector parser for the DescendantSelector. */
  class DescendantSelectorParser : public SelectorParser {
    public:

      /** Constructs the DescendantSelectorParser. */
      DescendantSelectorParser(std::shared_ptr<SelectorParser> base,
        std::shared_ptr<SelectorParser> descendant);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_base;
      std::shared_ptr<SelectorParser> m_descendant;
  };

  /** Represents a selector parser for the ParentSelector. */
  class ParentSelectorParser : public SelectorParser {
    public:

      /** Constructs the ParentSelectorParser. */
      explicit ParentSelectorParser(std::shared_ptr<SelectorParser> base,
        std::shared_ptr<SelectorParser> parent);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_base;
      std::shared_ptr<SelectorParser> m_parent;
  };

  /** Represents a selector parser for the AncestorSelector. */
  class AncestorSelectorParser : public SelectorParser {
    public:

      /** Constructs the AncestorSelectorParser. */
      AncestorSelectorParser(std::shared_ptr<SelectorParser> base,
        std::shared_ptr<SelectorParser> ancestor);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_base;
      std::shared_ptr<SelectorParser> m_ancestor;
  };

  /** Represents a selector parser for the SiblingSelector. */
  class SiblingSelectorParser : public SelectorParser {
    public:

      /** Constructs the SiblingSelectorParser. */
      SiblingSelectorParser(std::shared_ptr<SelectorParser> base,
        std::shared_ptr<SelectorParser> sibling);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_base;
      std::shared_ptr<SelectorParser> m_sibling;
  };

  /** Represents a selector parser for the AndSelector. */
  class AndSelectorParser : public SelectorParser {
    public:

      /** Constructs the AndSelectorParser. */
      AndSelectorParser(std::shared_ptr<SelectorParser> left,
        std::shared_ptr<SelectorParser> right);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_left;
      std::shared_ptr<SelectorParser> m_right;
  };

  /** Represents a selector parser for the OrSelector. */
  class OrSelectorParser : public SelectorParser {
    public:

      /** Constructs the OrSelectorParser. */
      OrSelectorParser(std::shared_ptr<SelectorParser> left,
        std::shared_ptr<SelectorParser> right);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_left;
      std::shared_ptr<SelectorParser> m_right;
  };

  /** Represents a selector parser for the NotSelector. */
  class NotSelectorParser : public SelectorParser {
    public:

      /** Constructs the NotSelectorParser. */
      explicit NotSelectorParser(std::shared_ptr<SelectorParser> operand);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_operand;
  };

  /** Represents a selector parser for the FlipSelector. */
  class FlipSelectorParser : public SelectorParser {
    public:

      /** Constructs the FlipSelectorParser. */
      explicit FlipSelectorParser(std::shared_ptr<SelectorParser> operand);

      Styles::Selector parse() override;

    private:
      std::shared_ptr<SelectorParser> m_operand;
  };

  /** Represents an abstract strategy for selector parsing. */
  class SelectorParseStrategy {
    public:

      virtual ~SelectorParseStrategy() = default;

      /** Parses the expression. */
      virtual std::shared_ptr<SelectorParser> parse_expression(
        TokenParser& token_parser) = 0;

      /** Parses the primary. */
      virtual std::shared_ptr<SelectorParser> parse_primary(
        TokenParser& token_parser) = 0;
  };

  /** Represents the default selector parsing strategy. */
  class DefaultSelectorParseStrategy : public SelectorParseStrategy {
    public:

      std::shared_ptr<SelectorParser> parse_expression(
        TokenParser& token_parser) override;
      std::shared_ptr<SelectorParser> parse_primary(
        TokenParser& token_parser) override;

    private:
      std::shared_ptr<SelectorParser> parse_or(TokenParser& token_parser);
      std::shared_ptr<SelectorParser> parse_and(TokenParser& token_parser);
      std::shared_ptr<SelectorParser> parse_hierarchy(
        TokenParser& token_parser);
      std::shared_ptr<SelectorParser> parse_property(TokenParser& token_parser);
      std::shared_ptr<SelectorParser> parse_colon_sign(
        TokenParser& token_parser, const Token& token);
  };

  /**
   * Registers the IsASelector for parsing.
   * @param name The selector name.
   * @param selector The IsASelector.
   */
  void register_type_selector(const std::string& name,
    const Styles::Selector& selector);

  /**
   * Registers the StateSelector for parsing.
   * @param name The selector name.
   * @param selector The StateSelector.
   */
  void register_state_selector(const std::string& name,
    const Styles::Selector& selector);

  /**
   * Registers the PseudoElement for parsing.
   * @param name The selector name.
   * @param selector The PseudoElement.
   */
  void register_pseudo_selector(const std::string& name,
    const Styles::Selector& selector);

  /**
   * Parses a selector based on the parsed tokens.
   * @param parser The token parser.
   * @param strategy The selector parse strategy.
   * @return A valid selector.
   * @throws <code>std::runtime_error</code> iff the token can't be parsed.
   */
  Styles::Selector parse_selector(TokenParser& token_parser,
    std::shared_ptr<SelectorParseStrategy> strategy);
}

#endif

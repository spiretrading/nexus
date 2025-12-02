#ifndef SPIRE_ORDER_FIELD_INFO_TIP_HPP
#define SPIRE_ORDER_FIELD_INFO_TIP_HPP
#include <string>
#include <vector>
#include <QWidget>

namespace Spire {

  /** Displays an InfoTip for a specific order field. */
  class OrderFieldInfoTip : public QWidget {
    public:

      /** Stores the details of a single order field. */
      struct Model {

        /** Stores meta data about a single argument and its description. */
        struct Argument {

          /** The argument's value. */
          std::string m_value;

          /** The argument's description. */
          std::string m_description;
        };

        /** Stores meta data about a single order field's tag. */
        struct Tag {

          /** The name of the tag. */
          std::string m_name;

          /** The tag's description. */
          std::string m_description;

          /** The list of permissible arguments. */
          std::vector<Argument> m_arguments;
        };

        /** The tag being displayed. */
        Tag m_tag;

        /** The list of prerequesits needed to enable the associated tag. */
        std::vector<Tag> m_prerequisites;
      };

      /**
       * Constructs an OrderFieldInfoTip.
       * @param model Stores all of the meta data about the order field to
       *        display.
       */
      explicit OrderFieldInfoTip(Model model, QWidget* parent = nullptr);

      QSize sizeHint() const override;
  };
}

#endif

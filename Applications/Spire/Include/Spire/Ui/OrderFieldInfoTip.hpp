#ifndef SPIRE_ORDER_FIELD_INFO_TIP_HPP
#define SPIRE_ORDER_FIELD_INFO_TIP_HPP
#include <QWidget>

namespace Spire {

  class OrderFieldInfoTip : public QWidget {
    public:
  
      struct Model {

        struct AllowedValue {
          std::string m_value;
          std::string m_description;
        };

        struct Tag {
          std::string m_name;
          std::string m_description;
          std::vector<AllowedValue> m_values;
        };

        Tag m_tag;
        std::vector<Tag> m_prerequisites;
      };

      OrderFieldInfoTip(Model model, QWidget* parent);
  };
}

#endif

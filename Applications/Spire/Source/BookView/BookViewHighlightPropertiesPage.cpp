#include "Spire/BookView/BookViewHighlightPropertiesPage.hpp"
#include "Spire/BookView/ArrayValueToListModel.hpp"
#include "Spire/BookView/MarketHighlightsTableView.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TableView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using OrderVisibility = BookViewHighlightProperties::OrderVisibility;
  using MarketHighlight = BookViewHighlightProperties::MarketHighlight;

  auto apply_header_label_style(int bottom_padding, StyleSheet& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(Font(font)).
      set(PaddingBottom(scale_height(bottom_padding)));
  }

  auto apply_description_label_style(StyleSheet& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(10));
    style.get(Any()).
      set(Font(font)).
      set(TextColor(QColor(0x808080))).
      set(PaddingBottom(scale_height(8)));
  }

  auto make_markets_title() {
    auto markets_header = make_label(QObject::tr("Markets"));
    markets_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*markets_header, std::bind_front(apply_header_label_style, 4));
    auto markets_description = make_label(
      QObject::tr("Set up highlights on orders from specific markets"));
    markets_description->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Fixed);
    update_style(*markets_description, apply_description_label_style);
    auto body = new QWidget();
    auto layout = make_vbox_layout(body);
    layout->addWidget(markets_header);
    layout->addWidget(markets_description);
    auto box = new Box(body);
    update_style(*box, [] (auto& style) {
      style.get(Any()).
        set(PaddingBottom(scale_height(8))).
        set(horizontal_padding(scale_width(8)));
    });
    return box;
  }

  auto setup_visibility_box() {
    static auto settings = [] {
      auto settings = EnumBox<OrderVisibility>::Settings(
        [] (const auto& value) {
          return to_text(value);
        },
        [] (const auto& value) {
          return make_label(to_text(value));
        });
      auto cases = std::make_shared<ArrayListModel<OrderVisibility>>();
      cases->push(OrderVisibility::HIDDEN);
      cases->push(OrderVisibility::VISIBLE);
      cases->push(OrderVisibility::HIGHLIGHTED);
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }

  auto make_visibility_slot(
      std::shared_ptr<ValueModel<OrderVisibility>> visibility) {
    auto label = make_label(QObject::tr("Visibility"));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(PaddingRight(scale_width(8)));
    });
    auto settings = setup_visibility_box();
    settings.m_current = std::move(visibility);
    auto visibility_box = new EnumBox<OrderVisibility>(std::move(settings));
    visibility_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    visibility_box->setFixedWidth(scale_width(120));
    auto body = new QWidget();
    auto layout = make_hbox_layout(body);
    layout->addWidget(label);
    layout->addWidget(visibility_box);
    auto box = new Box(body);
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(vertical_padding(scale_height(4)));
    });
    return box;
  }

  auto make_highlight_state_slot(const QString& name,
      std::shared_ptr<HighlightColorModel> current) {
    auto label = make_label(name);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(PaddingRight(scale_height(8)));
    });
    auto highlight_box = new HighlightBox(std::move(current));
    highlight_box->setFixedSize(scale(120, 19));
    auto layout = make_hbox_layout();
    layout->addWidget(label);
    layout->addWidget(highlight_box);
    return std::tuple(layout, highlight_box);
  }

  template<typename T, std::size_t N>
  class FixedSizeArrayValueToListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;
      using OperationSignal = ListModel<T>::OperationSignal;
      using UpdateOperation = typename ListModel<T>::UpdateOperation;
      using StartTransaction = typename ListModel<T>::StartTransaction;
      using EndTransaction = typename ListModel<T>::EndTransaction;

      explicit FixedSizeArrayValueToListModel(
        std::shared_ptr<ValueModel<std::array<Type, N>>> source)
        : m_source(std::move(source)) {}

      int get_size() const override {
        return static_cast<int>(m_source->get().size());
      }

      const Type& get(int index) const override {
        if(index < 0 || index >= get_size()) {
          throw std::out_of_range("The index is out of range.");
        }
        return m_source->get()[index];
      }

      QValidator::State set(int index, const Type& value) override {
        if(index < 0 || index >= get_size()) {
          throw std::out_of_range("The index is out of range.");
        }
        auto data = m_source->get();
        auto previous = data[index];
        data[index] = value;
        auto state = m_source->set(data);
        m_transaction.push(UpdateOperation(index, std::move(previous), value));
        return state;
      }

      connection connect_operation_signal(
          const typename OperationSignal::slot_type& slot) const override {
        return m_transaction.connect_operation_signal(slot);
      }

      void transact(const std::function<void()>& transaction) override {
        m_transaction.transact(transaction);
      }

    private:
      std::shared_ptr<ValueModel<std::array<Type, N>>> m_source;
      ListModelTransactionLog<Type> m_transaction;
  };

  using OrderHighlightStateListModel =
    FixedSizeArrayValueToListModel<HighlightColor,
      BookViewHighlightProperties::ORDER_HIGHLIGHT_STATE_COUNT>;
}

BookViewHighlightPropertiesPage::BookViewHighlightPropertiesPage(
    std::shared_ptr<HighlightPropertiesModel> current,
    MarketDatabase markets, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)) {
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  layout->addWidget(make_markets_title());
  auto table_view = make_market_highlights_table_view(
    std::make_shared<ArrayValueToListModel<MarketHighlight>>(
      make_field_value_model(m_current,
        &BookViewHighlightProperties::m_market_highlights)),
    std::move(markets));
  auto table_box = new Box(table_view);
  table_box->setFixedHeight(scale_height(208));
  update_style(*table_box, [] (auto& style) {
    style.get(Any()).
      set(BorderBottomSize(scale_height(1))).
      set(BorderBottomColor(QColor(0xE0E0E0)));
  });
  layout->addWidget(table_box);
  layout->addSpacing(scale_height(24));
  auto orders_header = make_label(QObject::tr("Orders"));
  orders_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  update_style(*orders_header, std::bind_front(apply_header_label_style, 8));
  auto orders_body = new QWidget();
  auto orders_layout = make_vbox_layout(orders_body);
  orders_layout->addWidget(orders_header);
  auto visibility = make_field_value_model(m_current,
    &BookViewHighlightProperties::m_order_visibility);
  visibility->connect_update_signal(std::bind_front(
    &BookViewHighlightPropertiesPage::on_visibility_update, this));
  orders_layout->addWidget(make_visibility_slot(visibility));
  auto order_highlights = std::make_shared<OrderHighlightStateListModel>(
    make_field_value_model(m_current,
      &BookViewHighlightProperties::m_order_highlights));
  for(auto i = 0; i < BookViewHighlightProperties::ORDER_HIGHLIGHT_STATE_COUNT;
      ++i) {
    auto [slot, highlight_box] =
      make_highlight_state_slot(to_text(
        static_cast<BookViewHighlightProperties::OrderHighlightState>(i)),
        std::make_shared<ListValueModel<HighlightColor>>(order_highlights, i));
    m_order_highlight_boxes[i] = highlight_box;
    link(*this, *highlight_box);
    orders_layout->addSpacing(scale_height(8));
    orders_layout->addLayout(slot);
  }
  auto orders_box = new Box(orders_body);
  update_style(*orders_box, [] (auto& style) {
    style.get(Any()).set(horizontal_padding(scale_width(8)));
  });
  layout->addWidget(orders_box);
  auto box = new Box(body);
  update_style(*box, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(vertical_padding(scale_height(18)));
  });
  enclose(*this, *box);
  on_visibility_update(visibility->get());
}

const std::shared_ptr<HighlightPropertiesModel>&
    BookViewHighlightPropertiesPage::get_current() const {
  return m_current;
}

void BookViewHighlightPropertiesPage::on_visibility_update(
    BookViewHighlightProperties::OrderVisibility visibility) {
  for(auto& box : m_order_highlight_boxes) {
    box->setEnabled(
      visibility == BookViewHighlightProperties::OrderVisibility::HIGHLIGHTED);
  }
}

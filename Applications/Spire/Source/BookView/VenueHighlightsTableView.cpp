#include "Spire/BookView/VenueHighlightsTableView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListToTableModel.hpp"
#include "Spire/Spire/SortedListModel.hpp"
#include "Spire/Spire/TableValueModel.hpp"
#include "Spire/Ui/DestinationListItem.hpp"
#include "Spire/Ui/EditableBox.hpp"
#include "Spire/Ui/EditableTableView.hpp"
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TableItem.hpp"
#include "Spire/Ui/VenueBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using VenueHighlightLevel =
    BookViewHighlightProperties::VenueHighlightLevel;
  using VenueHighlight = BookViewHighlightProperties::VenueHighlight;
  using EmptyVenue = StateSelector<void, struct EmptyVenueSeletorTag>;

  static const auto COLUMN_COUNT = 3;
  static const auto VENUE_COLUMN = 0;
  static const auto LEVEL_COLUMN = 1;
  static const auto COLOR_COLUMN = 2;

  const auto& DEFAULT_VENUE_HIGHLIGHTS() {
    static const auto default_venue_highlights = std::vector<HighlightColor>{
      {QColor(0xFFFFC4), QColor(0x834A2D)},
      {QColor(0xDDF9FF), QColor(0x003698)},
      {QColor(0xFFECFF), QColor(0x76008A)},
      {QColor(0xF4F4FF), QColor(0x4C00DA)},
      {QColor(0xFFE6C9), QColor(0xA00000)},
      {QColor(0xFEEE7F), QColor(0x630000)},
      {QColor(0x92CFE9), QColor(0x0000C6)},
      {QColor(0xF1A6F1), QColor(0x770088)},
      {QColor(0xC1BAFF), QColor(0x4F00D7)},
      {QColor(0xFFBF76), QColor(0xA00000)},
      {QColor(0x8A6729), QColor(0xFFFFFF)},
      {QColor(0x246FBC), QColor(0xFFFFFF)},
      {QColor(0xA937B2), QColor(0xFFFFFF)},
      {QColor(0x6F53C5), QColor(0xFFFFFF)},
      {QColor(0xAE4D1F), QColor(0xFFFFFF)},
      {QColor(0x4D2A00), QColor(0xFFFFFF)},
      {QColor(0x21148C), QColor(0xFFFFFF)},
      {QColor(0x5F006F), QColor(0xFFFFFF)},
      {QColor(0x361976), QColor(0xFFFFFF)},
      {QColor(0x680000), QColor(0xFFFFFF)}
    };
    return default_venue_highlights;
  }

  auto apply_table_view_style(StyleSheet& style) {
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(10));
    auto body_selector = Any() > is_a<TableBody>();
    auto item_selector = body_selector > Row() > is_a<TableItem>();
    auto input_box_selector = item_selector > Any() > Any();
    style.get(Any()).
      set(BorderTopSize(scale_height(1))).
      set(BorderTopColor(QColor(0xE0E0E0)));
    style.get(input_box_selector > Any() > ReadOnly() > is_a<TextBox>()).
      set(horizontal_padding(scale_width(8)));
    style.get(input_box_selector > is_a<HighlightBox>()).
      set(vertical_padding(scale_height(5))).
      set(horizontal_padding(scale_width(8)));
    style.get(input_box_selector > is_a<HighlightBox>() > is_a<TextBox>()).
      set(Font(font));
    style.get(item_selector > EmptyVenue() << Row() > is_a<TableItem>() >
      DeleteButton()).set(Visibility(Visibility::INVISIBLE));
  }

  AnyRef extract(VenueHighlight& highlight, int index) {
    if(index == VENUE_COLUMN) {
      return highlight.m_venue;
    } else if(index == LEVEL_COLUMN) {
      return highlight.m_level;
    } else {
      return highlight.m_color;
    }
  }

  auto setup_level_box() {
    static auto settings = [] {
      auto settings = EnumBox<VenueHighlightLevel>::Settings(
        [] (const auto& value) {
          return to_text(value);
        },
        [] (const auto& value) {
          return make_label(to_text(value));
        });
      auto cases = std::make_shared<ArrayListModel<VenueHighlightLevel>>();
      cases->push(VenueHighlightLevel::TOP);
      cases->push(VenueHighlightLevel::ALL);
      settings.m_cases = std::move(cases);
      return settings;
    }();
    return settings;
  }

  VenueBox* make_venue_box(std::shared_ptr<VenueModel> current,
      std::shared_ptr<ListModel<Venue>> available_venues,
      QWidget* parent = nullptr) {
    auto settings = VenueBox::Settings([=] (const auto& venue) {
      return QString::fromStdString(DEFAULT_VENUES.from(venue).m_display_name);
    },
    [=] (const auto& venue) {
      auto& entry = DEFAULT_VENUES.from(venue);
      auto destination_entry = DestinationDatabase::Entry();
      destination_entry.m_id = entry.m_display_name;
      destination_entry.m_description = entry.m_description;
      return new DestinationListItem(std::move(destination_entry));
    });
    settings.m_cases = std::move(available_venues);
    settings.m_current = std::move(current);
    return new VenueBox(std::move(settings), parent);
  }

  struct VenueHighlightListToTableModel : ListToTableModel<VenueHighlight> {
    std::shared_ptr<ListModel<VenueHighlight>> m_list;

    VenueHighlightListToTableModel(
      std::shared_ptr<ListModel<VenueHighlight>> list, int columns,
      Accessor accessor)
      : ListToTableModel(list, columns, accessor),
        m_list(std::move(list)) {}

    QValidator::State push(const VenueHighlight& value) {
      return m_list->push(value);
    }
  };

  struct ExpandableTableModel : TableModel {
    std::shared_ptr<VenueHighlightListToTableModel> m_source;
    std::shared_ptr<ArrayListModel<Venue>> m_venue_list;
    std::vector<std::shared_ptr<SortedListModel<Venue>>> m_available_venues;
    TableModelTransactionLog m_transaction;
    scoped_connection m_connection;

    ExpandableTableModel(std::shared_ptr<VenueHighlightListToTableModel> source)
        : m_source(std::move(source)),
          m_venue_list(std::make_shared<ArrayListModel<Venue>>()) {
      for(auto& venue : DEFAULT_VENUES.get_entries()) {
        m_venue_list->push(venue.m_venue);
      }
      for(auto i = 0; i < get_row_size(); ++i) {
        add_available_venues(i);
      }
      m_connection = m_source->connect_operation_signal(
        std::bind_front(&ExpandableTableModel::on_operation, this));
    }

    int get_row_size() const {
      return m_source->get_row_size() + 1;
    }

    int get_column_size() const {
      return COLUMN_COUNT;
    }

    AnyRef at(int row, int column) const {
      if(row < m_source->get_row_size()) {
        return m_source->at(row, column);
      } else if(row == m_source->get_row_size()) {
        if(column == VENUE_COLUMN) {
          static const auto NONE = Venue();
          return NONE;
        } else if(column == LEVEL_COLUMN) {
          static auto DEFAULT = VenueHighlightLevel::TOP;
          return DEFAULT;
        } else if(column == COLOR_COLUMN) {
          static auto DEFAULT =
            HighlightColor(Qt::transparent, Qt::transparent);
          return DEFAULT;
        }
      }
      throw std::out_of_range("The row or column is out of range.");
    }

    QValidator::State set(int row, int column, const std::any& value) {
      if(row < m_source->get_row_size()) {
        return m_source->set(row, column, value);
      } else if(row == m_source->get_row_size()) {
        if(column == LEVEL_COLUMN || column == COLOR_COLUMN) {
          return QValidator::Acceptable;
        }
        auto blocker = shared_connection_block(m_connection);
        add_available_venues(get_row_size());
        auto& highlights = DEFAULT_VENUE_HIGHLIGHTS();
        auto& highlight =
          highlights[m_source->get_row_size() % highlights.size()];
        m_source->push({std::any_cast<const Venue&>(value), highlight,
          BookViewHighlightProperties::VenueHighlightLevel::TOP});
        reduce_available_venues(row);
        m_transaction.transact([&] {
          m_transaction.push(UpdateOperation(row, column, Venue(), value));
          m_transaction.push(AddOperation(row + 1));
        });
        return QValidator::Acceptable;
      }
      throw std::out_of_range("The row is out of range.");
    }

    QValidator::State remove(int row) {
      if(row < m_source->get_row_size()) {
        return m_source->remove(row);
      } else if(row == m_source->get_row_size()) {
        return QValidator::Invalid;
      }
      throw std::out_of_range("The row is out of range.");
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const {
      return m_transaction.connect_operation_signal(slot);
    }

    void add_available_venues(int row) {
      auto available_venues = std::make_shared<SortedListModel<Venue>>(
        std::make_shared<ArrayListModel<Venue>>(),
        [=] (auto left, auto right) {
          return DEFAULT_VENUES.from(left).m_display_name <
            DEFAULT_VENUES.from(right).m_display_name;
        });
      for(auto i = 0; i < m_venue_list->get_size(); ++i) {
        available_venues->push(m_venue_list->get(i));
      }
      for(auto i = 0; i < get_row_size(); ++i) {
        if(i == row) {
          continue;
        }
        auto j = std::find(available_venues->begin(), available_venues->end(),
          get<Venue>(i, VENUE_COLUMN));
        if(j != available_venues->end()) {
          available_venues->remove(j);
        }
      }
      m_available_venues.insert(
        m_available_venues.begin() + row, std::move(available_venues));
    }

    void reduce_available_venues(int row, Venue venue) {
      if(!venue) {
        return;
      }
      for(auto i = 0; i < get_row_size(); ++i) {
        if(i == row) {
          continue;
        }
        auto& available_venues = *m_available_venues[i];
        auto j =
          std::find(available_venues.begin(), available_venues.end(), venue);
        if(j != available_venues.end()) {
          available_venues.remove(j);
        }
      }
    }

    void reduce_available_venues(int row) {
      reduce_available_venues(row, get<Venue>(row, VENUE_COLUMN));
    }

    void augment_available_venues(int row, Venue venue) {
      if(!venue) {
        return;
      }
      for(auto i = 0; i < get_row_size(); ++i) {
        if(i == row) {
          continue;
        }
        auto& available_venues = *m_available_venues[i];
        auto j =
          std::find(available_venues.begin(), available_venues.end(), venue);
        if(j == available_venues.end()) {
          available_venues.push(venue);
        }
      }
    }

    void augment_available_venues(int row) {
      augment_available_venues(row, get<Venue>(row, VENUE_COLUMN));
    }

    void on_operation(const TableModel::Operation& operation) {
      visit(operation,
        [&] (const TableModel::AddOperation& operation) {
          add_available_venues(operation.m_index);
          reduce_available_venues(operation.m_index);
        },
        [&] (const TableModel::PreRemoveOperation& operation) {
          augment_available_venues(operation.m_index);
        },
        [&] (const TableModel::RemoveOperation& operation) {
          m_available_venues.erase(
            m_available_venues.begin() + operation.m_index);
        },
        [&] (const TableModel::MoveOperation& operation) {
          auto i = m_available_venues.begin() + operation.m_source;
          auto source = std::move(*i);
          m_available_venues.erase(i);
          m_available_venues.insert(m_available_venues.begin() +
            operation.m_destination, std::move(source));
        },
        [&] (const TableModel::UpdateOperation& operation) {
          if(operation.m_column != VENUE_COLUMN) {
            return;
          }
          reduce_available_venues(operation.m_row);
          augment_available_venues(
            operation.m_row, std::any_cast<Venue>(operation.m_previous));
        });
      m_transaction.push(operation);
    }
  };

  struct ChildAddedObserver : QObject {
    TableBody* m_table_body;
    QWidget* m_added_widget;

    ChildAddedObserver(TableBody& table_body, QObject* parent = nullptr)
      : QObject(parent),
        m_table_body(&table_body),
        m_added_widget(nullptr) {}

    bool eventFilter(QObject* watched, QEvent* event) override {
      if(event->type() == QEvent::ChildAdded && watched == m_table_body) {
        auto& child_event = *static_cast<QChildEvent*>(event);
        if(child_event.added()) {
          if(auto child = child_event.child(); child->isWidgetType()) {
            m_added_widget = static_cast<QWidget*>(child);
            m_added_widget->installEventFilter(this);
          }
        }
      } else if(event->type() == QEvent::Show && watched == m_added_widget) {
        if(auto layout = m_added_widget->layout()) {
          if(layout->count() > 0) {
            layout->itemAt(0)->widget()->setFocusPolicy(Qt::NoFocus);
            m_added_widget->removeEventFilter(this);
            m_added_widget = nullptr;
          }
        }
      }
      return QObject::eventFilter(watched, event);
    }
  };

  template<typename T, typename M>
  struct InputBoxWrapper : QWidget {
    using InputBox = T;
    using Model = M;
    using SubmitSignal = InputBox::SubmitSignal;
    Model m_model;
    std::shared_ptr<VenueModel> m_venue;
    scoped_connection m_connection;

    InputBoxWrapper(Model model, std::shared_ptr<VenueModel> venue,
        QWidget* parent = nullptr)
        : m_model(std::move(model)),
          m_venue(std::move(venue)),
          QWidget(parent) {
      make_vbox_layout(this);
      if(m_venue->get()) {
        on_venue_update(m_venue->get());
      } else {
        m_connection = m_venue->connect_update_signal(
          std::bind_front(&InputBoxWrapper::on_venue_update, this));
      }
    }

    const decltype(std::declval<InputBox>().get_current())&
        get_current() const {
      if constexpr(std::is_same_v<InputBox, HighlightBox>) {
        return m_model;
      } else {
        return m_model.m_current;
      }
    }

    bool is_read_only() const {
      if(layout()->count() == 0) {
        return false;
      }
      return get_input_box()->is_read_only();
    }

    void set_read_only(bool is_read_only) {
      if(layout()->count() == 0) {
        return;
      }
      get_input_box()->set_read_only(is_read_only);
    }

    connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const {
      if(layout()->count() == 0) {
        return connection();
      }
      return get_input_box()->connect_submit_signal(slot);
    }

    InputBox* get_input_box() const {
      return static_cast<InputBox*>(layout()->itemAt(0)->widget());
    }

    void on_venue_update(Venue venue) {
      if(venue && layout()->count() == 0) {
        auto input_box = new InputBox(m_model);
        input_box->set_read_only(true);
        layout()->addWidget(input_box);
        setFocusProxy(input_box);
        setFocusPolicy(Qt::NoFocus);
      }
    }
  };

  auto make_item(const std::shared_ptr<ExpandableTableModel>& expandable_table,
      const std::shared_ptr<TableModel>& table, int row, int column) {
    if(column == VENUE_COLUMN) {
      auto venue_box = make_venue_box(
        make_table_value_model<Venue>(table, row, column),
        expandable_table->m_available_venues[row]);
      auto item = new EditableBox(*venue_box);
      if(row == table->get_row_size() - 1) {
        match(*item, EmptyVenue());
      }
      venue_box->get_current()->connect_update_signal(
        [=] (const auto& venue) {
          if(venue && is_match(*item, EmptyVenue())) {
            unmatch(*item, EmptyVenue());
          }
        });
      return item;
    } else if(column == LEVEL_COLUMN) {
      auto settings = setup_level_box();
      settings.m_current =
        make_table_value_model<VenueHighlightLevel>(table, row, column);
      return new EditableBox(*new InputBoxWrapper<EnumBox<VenueHighlightLevel>,
        EnumBox<VenueHighlightLevel>::Settings>(std::move(settings),
          make_table_value_model<Venue>(table, row, VENUE_COLUMN)));
    } else if(column == COLOR_COLUMN) {
      return new EditableBox(*new InputBoxWrapper<HighlightBox,
        std::shared_ptr<HighlightColorModel>>(
          make_table_value_model<HighlightColor>(table, row, column),
          make_table_value_model<Venue>(table, row, VENUE_COLUMN)));
    }
    throw std::runtime_error("Invalid column.");
  }
}

TableView* Spire::make_venue_highlights_table_view(
    std::shared_ptr<VenueHighlightListModel> venue_highlights,
    QWidget* parent) {
  auto table = std::make_shared<ExpandableTableModel>(
    std::make_shared<VenueHighlightListToTableModel>(
      venue_highlights, COLUMN_COUNT, &extract));
  auto table_builder = EditableTableViewBuilder(table);
  table_builder.add_header_item(QObject::tr("Venue"), QObject::tr("Vn"),
    TableHeaderItem::Order::NONE, TableFilter::Filter::NONE);
  table_builder.add_header_item(QObject::tr("Highlight"), QObject::tr("HL"),
    TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE);
  table_builder.add_header_item(QObject::tr("Color"), QObject::tr("Clr"),
    TableHeaderItem::Order::UNORDERED, TableFilter::Filter::NONE);
  table_builder.set_item_builder(std::bind_front(make_item, table));
  table_builder.set_comparator([=] (const AnyRef& left, int left_row,
      const AnyRef& right, int right_row, int column) {
    if(left_row == table->get_row_size() - 1) {
      return false;
    }
    return DEFAULT_VENUES.from(any_cast<Venue>(left)).m_display_name <
      DEFAULT_VENUES.from(any_cast<Venue>(right)).m_display_name;
  });
  auto table_view = table_builder.make();
  table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  table_view->get_header().get_widths()->set(0, scale_width(26));
  table_view->get_header().get_widths()->set(1, scale_width(92));
  table_view->get_header().get_widths()->set(2, scale_width(92));
  table_view->get_header().get_widths()->set(3, scale_width(138));
  update_style(*table_view, apply_table_view_style);
  auto filter = new ChildAddedObserver(table_view->get_body(), table_view);
  table_view->get_body().installEventFilter(filter);
  return table_view;
}

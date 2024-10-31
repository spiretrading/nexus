#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include <QEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Spire/ListValueModel.hpp"
#include "Spire/Ui/ColorBox.hpp"
#include "Spire/Ui/ColorConversion.hpp"
#include "Spire/Ui/FontBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using ColorBand = StateSelector<void, struct ColorBandTag>;
  using ScrollBoxOverflow = StateSelector<void, struct ScrollBoxOverflowTag>;

  enum class FillType {
    GRADIENT,
    SOLID
  };

  auto to_text(FillType type) {
    if(type == FillType::GRADIENT) {
      static const auto value = QObject::tr("Gradient");
      return value;
    } else {
      static const auto value = QObject::tr("Solid");
      return value;
    }
  }

  auto make_header_label(const QString& name) {
    auto label = make_label(name);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*label, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(12));
      style.get(Any()).
        set(Font(font)).
        set(PaddingBottom(scale_height(8)));
    });
    return label;
  }

  auto make_levels_slot(std::shared_ptr<OptionalIntegerModel> model) {
    auto label = make_label(QObject::tr("Levels"));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(PaddingRight(scale_height(8)));
    });
    auto levels_box = new IntegerBox(std::move(model));
    levels_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    levels_box->setFixedWidth(6 * scale_width(12));
    auto body = new QWidget();
    auto layout = make_hbox_layout(body);
    layout->addWidget(label);
    layout->addWidget(levels_box);
    auto box = new Box(body);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(vertical_padding(scale_height(4)));
    });
    return box;
  }

  auto get_text_color(const QColor& background_color) {
    if(std::abs(apca(QColor(Qt::black), background_color)) >
      std::abs(apca(QColor(Qt::white), background_color))) {
      return QColor(Qt::black);
    }
    return QColor(Qt::white);
  }

  template<typename T>
  class ArrayValueToListModel : public ListModel<T> {
    public:
      using Type = typename ListModel<T>::Type;

      using OperationSignal = ListModel<T>::OperationSignal;

      using AddOperation = typename ListModel<T>::AddOperation;

      using PreRemoveOperation = typename ListModel<T>::PreRemoveOperation;

      using RemoveOperation = typename ListModel<T>::RemoveOperation;

      using UpdateOperation = typename ListModel<T>::UpdateOperation;

      using StartTransaction = typename ListModel<T>::StartTransaction;

      using EndTransaction = typename ListModel<T>::EndTransaction;

      ArrayValueToListModel(
        std::shared_ptr<ValueModel<std::vector<Type>>> source)
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

      QValidator::State insert(const Type& value, int index) override {
        if(index < 0 || index > get_size()) {
          throw std::out_of_range("The index is out of range.");
        }
        auto data = m_source->get();
        data.insert(std::next(data.begin(), index), value);
        auto state = m_source->set(data);
        m_transaction.push(AddOperation(index));
        return state;
      }

      QValidator::State remove(int index) override {
        if(index < 0 || index >= get_size()) {
          throw std::out_of_range("The index is out of range.");
        }
        auto data = m_source->get();
        m_transaction.push(PreRemoveOperation(index));
        data.erase(std::next(data.begin(), index));
        auto state = m_source->set(data);
        m_transaction.push(RemoveOperation(index));
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
      std::shared_ptr<ValueModel<std::vector<Type>>> m_source;
      ListModelTransactionLog<Type> m_transaction;
  };
}

struct PriceLevelModel {
  std::shared_ptr<ListModel<QColor>> m_color_scheme;
  std::shared_ptr<LocalOptionalIntegerModel> m_levels;
  std::shared_ptr<AssociativeValueModel<FillType>> m_fill_type;
  std::shared_ptr<ArrayListModel<QColor>> m_colors;
  scoped_connection m_scheme_connection;
  scoped_connection m_levels_connection;
  scoped_connection m_type_connection;
  scoped_connection m_color_connection;

  PriceLevelModel(std::shared_ptr<ListModel<QColor>> color_scheme)
      : m_color_scheme(std::move(color_scheme)),
        m_levels(std::make_shared<LocalOptionalIntegerModel>(
          m_color_scheme->get_size())),
        m_fill_type(std::make_shared<AssociativeValueModel<FillType>>(
          FillType::GRADIENT)),
        m_colors(std::make_shared<ArrayListModel<QColor>>()) {
    m_levels->set_minimum(1);
    m_levels->set_maximum(99);
    m_scheme_connection = m_color_scheme->connect_operation_signal(
      std::bind_front(&PriceLevelModel::on_color_scheme_operation, this));
    m_levels_connection = m_levels->connect_update_signal(
      std::bind_front(&PriceLevelModel::on_levels_update, this));
    m_type_connection = m_fill_type->connect_update_signal(
      std::bind_front(&PriceLevelModel::on_type_update, this));
    m_color_connection = m_colors->connect_operation_signal(
      std::bind_front(&PriceLevelModel::on_color_operation, this));
    on_type_update(m_fill_type->get());
  }

  void scale(int levels) {
    auto colors = scale_oklch(m_color_scheme->get(0),
      m_color_scheme->get(m_color_scheme->get_size() - 1), levels);
    m_color_scheme->transact([&] {
      auto index = 0;
      while(index < std::ssize(colors)) {
        if(index < m_color_scheme->get_size()) {
          if(m_color_scheme->get(index).name() != colors[index].name()) {
            m_color_scheme->set(index, colors[index]);
          }
        } else {
          m_color_scheme->insert(colors[index], index);
        }
        ++index;
      }
      while(m_color_scheme->get_size() > index) {
        m_color_scheme->remove(m_color_scheme->get_size() - 1);
      }
    });
  }

  void on_levels_update(const optional<int> levels) {
    if(!levels) {
      return;
    }
    if(m_fill_type->get() == FillType::GRADIENT) {
      scale(*levels);
    } else {
      m_color_scheme->transact([&] {
        if(m_color_scheme->get_size() < *levels) {
          auto color = m_color_scheme->get(m_color_scheme->get_size() - 1);
          for(auto i = m_color_scheme->get_size(); i < *levels; ++i) {
            m_color_scheme->insert(color, i);
          }
        } else {
          while(m_color_scheme->get_size() > *levels) {
            m_color_scheme->remove(m_color_scheme->get_size() - 1);
          }
        }
      });
    }
  }

  void on_type_update(FillType type) {
    if(type == FillType::GRADIENT) {
      m_colors->transact([&] {
        if(m_colors->get_size() == 0) {
          auto size = std::min(m_color_scheme->get_size(), 2);
          for(auto i = 0; i < size; ++i) {
            if(i == 1) {
              m_colors->insert(
                m_color_scheme->get(m_color_scheme->get_size() - 1), i);
            } else {
              m_colors->insert(m_color_scheme->get(i), i);
            }
          }
        } else if(m_colors->get_size() > 2) {
          while(m_colors->get_size() > 2) {
            m_colors->remove(1);
          }
          scale(*m_levels->get());
        }
      });
    } else {
      auto current_blocker = shared_connection_block(m_color_connection);
      for(auto i = 0; i < m_colors->get_size(); ++i) {
        m_colors->set(i, m_color_scheme->get(i));
      }
      for(auto i = m_colors->get_size(); i < m_color_scheme->get_size(); ++i) {
        m_colors->insert(m_color_scheme->get(i), i);
      }
    }
  }

  void on_color_scheme_operation(
      const ListModel<QColor>::Operation& operation) {
    visit(operation,
      [&] (const ListModel<QColor>::AddOperation& operation) {
        if(m_fill_type->get() == FillType::SOLID || m_colors->get_size() < 2) {
          m_colors->insert(m_color_scheme->get(operation.m_index),
            operation.m_index);
        }
      },
      [&] (const ListModel<QColor>::RemoveOperation& operation) {
        if(operation.m_index < m_colors->get_size()) {
          m_colors->remove(operation.m_index);
        }
      });
  }

  void on_color_operation(const ListModel<QColor>::Operation& operation) {
    visit(operation,
      [&] (const ListModel<QColor>::UpdateOperation& operation) {
        if(m_fill_type->get() == FillType::GRADIENT) {
          if(operation.m_index == 0) {
            m_color_scheme->set(operation.m_index, operation.get_value());
          } else {
            m_color_scheme->set(m_color_scheme->get_size() - 1,
              operation.get_value());
          }
          scale(*m_levels->get());
        } else {
          m_color_scheme->set(operation.m_index, operation.get_value());
        }
      });
  }
};

struct BookViewLevelPropertiesPage::PriceLevelWidget : QWidget {
  std::shared_ptr<PriceLevelModel> m_model;
  std::shared_ptr<ValueModel<QFont>> m_font;
  ScrollBox* m_scroll_box;
  QVBoxLayout* m_color_bands_layout;
  QVBoxLayout* m_color_boxes_layout;
  std::array<CheckBox*, 2> m_buttons;
  scoped_connection m_scheme_connection;

  PriceLevelWidget( std::shared_ptr<ValueModel<std::vector<QColor>>> source,
      std::shared_ptr<ValueModel<QFont>> font, QWidget* parent = nullptr)
      : QWidget(parent),
        m_model(std::make_shared<PriceLevelModel>(
          std::make_shared<ArrayValueToListModel<QColor>>(std::move(source)))),
        m_font(std::move(font)) {
    auto body = new QWidget();
    auto body_layout = make_hbox_layout(body);
    auto color_bands = make_color_bands();
    color_bands->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    color_bands->setFixedWidth(scale_width(138));
    body_layout->addWidget(color_bands, 0, Qt::AlignTop);
    body_layout->addSpacing(scale_width(8));
    auto color_boxes = make_color_boxes();
    color_boxes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    body_layout->addWidget(color_boxes, 0, Qt::AlignTop);
    m_scroll_box = new ScrollBox(body);
    m_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_scroll_box->get_vertical_scroll_bar().installEventFilter(this);
    update_style(*m_scroll_box, [] (auto& style) {
      style.get(ScrollBoxOverflow()).set(PaddingRight(scale_width(4)));
    });
    auto layout = make_vbox_layout(this);
    layout->setSpacing(scale_height(8));
    layout->addWidget(make_levels_slot(m_model->m_levels));
    layout->addWidget(make_fill_type_slot());
    layout->addWidget(m_scroll_box);
    m_scheme_connection = m_model->m_color_scheme->connect_operation_signal(
      std::bind_front(&PriceLevelWidget::on_scheme_operation, this));
    m_model->m_colors->connect_operation_signal(
      std::bind_front(&PriceLevelWidget::on_color_operation, this));
    on_type_update(m_model->m_fill_type->get(), true);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(watched == &m_scroll_box->get_vertical_scroll_bar()) {
      if(event->type() == QEvent::Show) {
        match(*m_scroll_box, ScrollBoxOverflow());
      } else if(event->type() == QEvent::Hide) {
        unmatch(*m_scroll_box, ScrollBoxOverflow());
      }
    }
    return QWidget::eventFilter(watched, event);
  }

  void make_button(FillType type) {
    auto button = make_radio_button();
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->set_label(to_text(type));
    button->get_current()->connect_update_signal([=] (auto value) {
      if(m_model->m_fill_type->get() == type && !value) {
        button->get_current()->set(true);
      } else if(value) {
        m_model->m_fill_type->set(type);
      }
    });
    m_model->m_fill_type->get_association(type)->connect_update_signal(
      std::bind_front(&PriceLevelWidget::on_type_update, this, type));
    m_buttons[static_cast<int>(type)] = button;
  }

  QWidget* make_fill_type_slot() {
    make_button(FillType::GRADIENT);
    make_button(FillType::SOLID);
    auto label = make_label(QObject::tr("Fill Type"));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto body = new QWidget();
    auto layout = make_hbox_layout(body);
    layout->addWidget(label);
    layout->addWidget(m_buttons[static_cast<int>(FillType::GRADIENT)]);
    layout->addSpacing(scale_width(18));
    layout->addWidget(m_buttons[static_cast<int>(FillType::SOLID)]);
    auto box = new Box(body);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(vertical_padding(scale_height(5)));
    });
    return box;
  }

  TextBox* make_color_band(int index) {
    auto band = make_label(QString("%1").arg(index + 1));
    band->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    band->setFixedHeight(scale_height(17));
    update_style(*band, [&] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(m_model->m_color_scheme->get(index))).
        set(Font(m_font->get())).
        set(TextColor(get_text_color(m_model->m_color_scheme->get(index)))).
        set(TextAlign(Qt::AlignCenter));
    });
    match(*band, ColorBand());
    link(*this, *band);
    return band;
  }

  ColorBox* make_color_box(int index) {
    auto color_box = new ColorBox(std::make_shared<ListValueModel<QColor>>(
      m_model->m_colors, index));
    color_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    color_box->setFixedHeight(scale_height(15));
    return color_box;
  }

  QWidget* make_color_bands() {
    auto body = new QWidget();
    m_color_bands_layout = make_vbox_layout(body);
    for(auto i = 0; i < m_model->m_color_scheme->get_size(); ++i) {
      m_color_bands_layout->addWidget(make_color_band(i));
    }
    auto box = new Box(body);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(0xE0E0E0)));
    });
    return box;
  }

  QWidget* make_color_boxes() {
    auto body = new QWidget();
    m_color_boxes_layout = make_vbox_layout(body);
    m_color_boxes_layout->setSpacing(scale_height(2));
    for(auto i = 0; i < m_model->m_colors->get_size(); ++i) {
      m_color_boxes_layout->addWidget(make_color_box(i));
    }
    auto box = new Box(body);
    update_style(*box, [] (auto& style) {
      style.get(Any()).set(vertical_padding(scale_height(2)));
    });
    return box;
  }

  void on_type_update(FillType type, bool value) {
    m_buttons[static_cast<int>(type)]->get_current()->set(value);
  }

  void on_scheme_operation(const ListModel<QColor>::Operation& operation) {
    visit(operation,
      [&] (const ListModel<QColor>::AddOperation& operation) {
        m_color_bands_layout->insertWidget(operation.m_index,
          make_color_band(operation.m_index));
      },
      [&] (const ListModel<QColor>::RemoveOperation& operation) {
        if(auto item = m_color_bands_layout->takeAt(operation.m_index)) {
          delete item->widget();
          delete item;
        }
      },
      [&] (const ListModel<QColor>::UpdateOperation& operation) {
        auto band = m_color_bands_layout->itemAt(operation.m_index)->widget();
        update_style(*band, [&] (auto& style) {
          style.get(Any()).
            set(BackgroundColor(operation.get_value())).
            set(TextColor(get_text_color(operation.get_value())));
        });
      });
  }

  void on_color_operation(const ListModel<QColor>::Operation& operation) {
    visit(operation,
      [&] (const ListModel<QColor>::AddOperation& operation) {
        m_color_boxes_layout->insertWidget(operation.m_index,
          make_color_box(operation.m_index));
      },
      [&] (const ListModel<QColor>::RemoveOperation& operation) {
        if(auto item = m_color_boxes_layout->takeAt(operation.m_index)) {
          delete item->widget();
          delete item;
        }
      });
  }
};

BookViewLevelPropertiesPage::BookViewLevelPropertiesPage(
    std::shared_ptr<LevelPropertiesModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_font(m_current->get().m_font) {
  auto text_header = make_header_label(tr("Text"));
  auto font_box = new FontBox(make_field_value_model(m_current,
    &BookViewLevelProperties::m_font));
  font_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_font_connection = font_box->get_current()->connect_update_signal(
    std::bind_front(&BookViewLevelPropertiesPage::on_font, this));
  auto grid_check_box = new CheckBox(make_field_value_model(m_current,
    &BookViewLevelProperties::m_is_grid_enabled));
  grid_check_box->set_label(tr("Show Grid"));
  grid_check_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  auto price_level_header = make_header_label(tr("Price Levels"));
  auto price_level_widget = new PriceLevelWidget(
    make_field_value_model(m_current, &BookViewLevelProperties::m_color_scheme),
    font_box->get_current());
  link(*this, *price_level_widget);
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  layout->addWidget(text_header);
  layout->addWidget(font_box);
  layout->addSpacing(scale_height(24));
  layout->addWidget(grid_check_box, 0, Qt::AlignLeft);
  layout->addSpacing(scale_height(24));
  layout->addWidget(price_level_header);
  layout->addWidget(price_level_widget);
  auto box = new Box(body);
  proxy_style(*this, *box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(18)));
  });
  enclose(*this, *box);
  on_font(font_box->get_current()->get());
}

const std::shared_ptr<LevelPropertiesModel>&
    BookViewLevelPropertiesPage::get_current() const {
  return m_current;
}

void BookViewLevelPropertiesPage::on_font(const QFont& font) {
  if(m_font != font) {
    update_style(*this, [&] (auto& style) {
      style.get(Any() > is_a<PriceLevelWidget>() > ColorBand()).
        set(Font(font));
    });
    m_font = font;
  }
}

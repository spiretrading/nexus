#include "Spire/BookView/BookViewLevelPropertiesPage.hpp"
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <QEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayValueToListModel.hpp"
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
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using FillType = BookViewLevelProperties::FillType;
  using ScrollBoxOverflow = StateSelector<void, struct ScrollBoxOverflowTag>;
  const auto COLOR_BAND_WIDTH = 138;
  const auto COLOR_BAND_HEIGHT = 17;
  const auto COLOR_BOX_HEIGHT = 15;
  const auto COLOR_BOXES_SPACING = 2;
  const auto COLOR_BOXES_VERTICAL_PADDING = 2;
  const auto COLOR_LIST_SPACING = 8;
  const auto COLOR_LIST_RIGHT_PADDING = 4;
  const auto DEBOUNCE_TIME_MS = 100;

  auto INTEGER_BOX_HORIZONTAL_PADDING() {
    static auto padding = scale_width(32);
    return padding;
  }

  auto get_character_width() {
    static auto width = [] {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Normal);
      font.setPixelSize(scale_width(12));
      return Spire::get_character_width(font);
    }();
    return width;
  }

  auto to_text(FillType type) {
    if(type == FillType::GRADIENT) {
      static const auto value = QObject::tr("Gradient");
      return value;
    } else {
      static const auto value = QObject::tr("Solid");
      return value;
    }
  }

  void scale(ListModel<QColor>& scheme, const QColor& start,
      const QColor& end, int levels) {
    auto colors = scale_oklch(start, end, levels);
    auto alphas = scale_alpha(start.alpha(), end.alpha(), levels);
    scheme.transact([&] {
      auto index = 0;
      while(index < levels) {
        if(index < scheme.get_size()) {
          if(scheme.get(index).rgb() != colors[index].rgb() ||
              scheme.get(index).alpha() != alphas[index]) {
            scheme.set(index,
              QColor(colors[index].red(), colors[index].green(),
                colors[index].blue(), alphas[index]));
          }
        } else {
          scheme.insert(QColor(colors[index].red(), colors[index].green(),
            colors[index].blue(), alphas[index]), index);
        }
        ++index;
      }
      while(scheme.get_size() > index) {
        scheme.remove(scheme.get_size() - 1);
      }
    });
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

  auto make_levels_slot(std::shared_ptr<OptionalIntegerModel> levels) {
    auto label = make_label(QObject::tr("Levels"));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      style.get(Any()).set(PaddingRight(scale_height(8)));
    });
    auto levels_box = new IntegerBox(std::move(levels));
    levels_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    levels_box->setFixedWidth(
      6 * ::get_character_width() + INTEGER_BOX_HORIZONTAL_PADDING());
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

  struct FillTypeAssociativeValueModel : AssociativeValueModel<FillType> {
    std::shared_ptr<ValueModel<FillType>> m_fill_type;
    scoped_connection m_connection;

    FillTypeAssociativeValueModel(
      std::shared_ptr<ValueModel<FillType>> fill_type)
      : AssociativeValueModel<FillType>(fill_type->get()),
        m_fill_type(std::move(fill_type)),
  BEAM_SUPPRESS_THIS_INITIALIZER()
        m_connection(m_fill_type->connect_update_signal(
          std::bind_front(&FillTypeAssociativeValueModel::on_update, this))) {}
  BEAM_UNSUPPRESS_THIS_INITIALIZER()

    QValidator::State set(const Type& value) override {
      auto state = AssociativeValueModel<FillType>::set(value);
      if(state == QValidator::Acceptable) {
        auto blocker = shared_connection_block(m_connection);
        m_fill_type->set(value);
      }
      return state;
    }

    void on_update(FillType type) {
      if(type != get()) {
        AssociativeValueModel<FillType>::set(type);
      }
    }
  };

  struct BandColorListModel : ListModel<QColor> {
    std::shared_ptr<ListModel<QColor>> m_color_scheme;
    ArrayListModel<QColor> m_colors;
    bool m_is_blocked;
    scoped_connection m_color_scheme_connection;
    scoped_connection m_connection;

    explicit BandColorListModel(std::shared_ptr<ListModel<QColor>> color_scheme)
        : m_color_scheme(std::move(color_scheme)),
          m_is_blocked(false) {
      m_colors.transact([&] {
        for(auto i = 0; i < m_color_scheme->get_size(); ++i) {
          m_colors.push(m_color_scheme->get(i));
        }
      });
      m_color_scheme_connection = m_color_scheme->connect_operation_signal(
        std::bind_front(&BandColorListModel::on_color_scheme_operation, this));
      m_connection = m_colors.connect_operation_signal(
        std::bind_front(&BandColorListModel::on_operation, this));
    }

    void submit() {
      m_color_scheme->transact([&] {
        auto blocker = shared_connection_block(m_color_scheme_connection);
        for(auto i = 0; i < m_colors.get_size(); ++i) {
          m_color_scheme->set(i, m_colors.get(i));
        }
      });
    }

    int get_size() const override {
      return m_colors.get_size();
    }

    const QColor& get(int index) const override {
      return m_colors.get(index);
    }

    QValidator::State set(int index, const QColor& value) override {
      auto blocker = shared_connection_block(m_connection, m_is_blocked);
      return m_colors.set(index, value);
    }

    QValidator::State insert(const QColor& value, int index) override {
      auto blocker = shared_connection_block(m_connection, m_is_blocked);
      return m_colors.insert(value, index);
    }

    QValidator::State move(int source, int destination) override {
      auto blocker = shared_connection_block(m_connection, m_is_blocked);
      return m_colors.move(source, destination);
    }

    QValidator::State remove(int index) override {
      auto blocker = shared_connection_block(m_connection, m_is_blocked);
      return m_colors.remove(index);
    }

    connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override {
      return m_colors.connect_operation_signal(slot);
    }

    void transact(const std::function<void()>& transaction) override {
      if(m_is_blocked) {
        m_colors.transact([&] {
          transaction();
        });
      } else {
        m_color_scheme->transact([&] {
          m_colors.transact([&] {
            transaction();
          });
        });
      }
    }

    void on_color_scheme_operation(
        const ListModel<QColor>::Operation& operation) {
      visit(operation,
        [&] (const ListModel<QColor>::AddOperation& operation) {
          auto blocker = shared_connection_block(m_connection);
          m_colors.insert(m_color_scheme->get(operation.m_index),
            operation.m_index);
        },
        [&] (const ListModel<QColor>::RemoveOperation& operation) {
          auto blocker = shared_connection_block(m_connection);
          m_colors.remove(operation.m_index);
        },
        [&] (const ListModel<QColor>::MoveOperation& operation) {
          auto blocker = shared_connection_block(m_connection);
          m_colors.move(operation.m_source, operation.m_destination);
        },
        [&] (const ListModel<QColor>::UpdateOperation& operation) {
          auto blocker = shared_connection_block(m_connection);
          m_colors.set(operation.m_index, operation.get_value());
        });
    }

    void on_operation(const ListModel<QColor>::Operation& operation) {
      visit(operation,
        [&] (const ListModel<QColor>::AddOperation& operation) {
          auto blocker = shared_connection_block(m_color_scheme_connection);
          m_color_scheme->insert(m_colors.get(operation.m_index),
            operation.m_index);
        },
        [&] (const ListModel<QColor>::RemoveOperation& operation) {
          auto blocker = shared_connection_block(m_color_scheme_connection);
          m_color_scheme->remove(operation.m_index);
        },
        [&] (const ListModel<QColor>::MoveOperation& operation) {
          auto blocker = shared_connection_block(m_color_scheme_connection);
          m_color_scheme->move(operation.m_source, operation.m_destination);
        },
        [&] (const ListModel<QColor>::UpdateOperation& operation) {
          auto blocker = shared_connection_block(m_color_scheme_connection);
          m_color_scheme->set(operation.m_index, operation.get_value());
        });
    }
  };

  struct PriceLevelPropertiesModel {
    std::shared_ptr<FillTypeAssociativeValueModel> m_fill_type;
    std::shared_ptr<BandColorListModel> m_band_colors;
    std::shared_ptr<ArrayListModel<QColor>> m_colors;
    std::shared_ptr<LocalOptionalIntegerModel> m_levels;
    QColor m_end_color;
    QTimer* m_timer;
    scoped_connection m_scheme_connection;
    scoped_connection m_levels_connection;
    scoped_connection m_type_connection;
    scoped_connection m_band_colors_connection;
    scoped_connection m_colors_connection;

    PriceLevelPropertiesModel(std::shared_ptr<ValueModel<FillType>> fill_type,
        std::shared_ptr<ListModel<QColor>> color_scheme, QTimer& timer)
        : m_fill_type(std::make_shared<FillTypeAssociativeValueModel>(
            std::move(fill_type))),
          m_band_colors(
            std::make_shared<BandColorListModel>(std::move(color_scheme))),
          m_colors(std::make_shared<ArrayListModel<QColor>>()),
          m_levels(std::make_shared<LocalOptionalIntegerModel>(
            m_band_colors->get_size())),
          m_timer(&timer) {
      if(m_band_colors->get_size() > 0) {
        m_end_color = m_band_colors->get(m_band_colors->get_size() - 1);
      }
      m_levels->set_minimum(1);
      m_levels->set_maximum(99);
      m_levels_connection = m_levels->connect_update_signal(
        std::bind_front(&PriceLevelPropertiesModel::on_levels_update, this));
      m_type_connection = m_fill_type->connect_update_signal(
        std::bind_front(&PriceLevelPropertiesModel::on_type_update, this));
      m_band_colors_connection = m_band_colors->connect_operation_signal(
        std::bind_front(&PriceLevelPropertiesModel::on_band_colors_operation,
          this));
      m_colors_connection = m_colors->connect_operation_signal(
        std::bind_front(&PriceLevelPropertiesModel::on_colors_operation, this));
      on_type_update(m_fill_type->get());
      m_timer->setSingleShot(true);
      QObject::connect(m_timer, &QTimer::timeout,
        std::bind_front(&PriceLevelPropertiesModel::on_timeout, this));
    }

    void update_gradient_colors(int levels) {
      if(m_band_colors->get_size() > 1) {
        m_end_color = m_band_colors->get(m_band_colors->get_size() - 1);
      }
      scale(*m_band_colors, m_band_colors->get(0), m_end_color, levels);
    }

    void on_levels_update(const optional<int> levels) {
      if(!levels || *levels < m_levels->get_minimum() ||
          *levels > m_levels->get_maximum()) {
        return;
      }
      if(m_fill_type->get() == FillType::GRADIENT) {
        update_gradient_colors(*levels);
      } else {
        m_band_colors->transact([&] {
          if(m_band_colors->get_size() < *levels) {
            auto color = m_band_colors->get(m_band_colors->get_size() - 1);
            for(auto i = m_band_colors->get_size(); i < *levels; ++i) {
              m_band_colors->insert(color, i);
            }
          } else {
            while(m_band_colors->get_size() > *levels) {
              m_band_colors->remove(m_band_colors->get_size() - 1);
            }
          }
        });
      }
    }

    void on_type_update(FillType type) {
      auto blocker = shared_connection_block(m_colors_connection);
      if(type == FillType::GRADIENT) {
        m_colors->transact([&] {
          if(m_colors->get_size() == 0) {
            auto size = std::min(m_band_colors->get_size(), 2);
            for(auto i = 0; i < size; ++i) {
              if(i == 1) {
                m_colors->insert(
                  m_band_colors->get(m_band_colors->get_size() - 1), i);
              } else {
                m_colors->insert(m_band_colors->get(i), i);
              }
            }
          } else if(m_colors->get_size() > 2) {
            while(m_colors->get_size() > 2) {
              m_colors->remove(1);
            }
          }
        });
        update_gradient_colors(m_band_colors->get_size());
      } else {
        m_colors->transact([&] {
          for(auto i = 0; i < m_colors->get_size(); ++i) {
            m_colors->set(i, m_band_colors->get(i));
          }
          for(auto i = m_colors->get_size(); i < m_band_colors->get_size();
              ++i) {
            m_colors->insert(m_band_colors->get(i), i);
          }
        });
      }
    }

    void on_timeout() {
      m_band_colors->m_is_blocked = true;
      update_gradient_colors(m_band_colors->get_size());
      m_band_colors->m_is_blocked = false;
    }

    void on_band_colors_operation(
        const ListModel<QColor>::Operation& operation) {
      auto update_levels = [&] {
        if(auto levels = m_levels->get();
            levels && *levels != m_band_colors->get_size()) {
          auto blocker = shared_connection_block(m_levels_connection);
          m_levels->set(m_band_colors->get_size());
        }
      };
      visit(operation,
        [&] (const ListModel<QColor>::AddOperation& operation) {
          auto blocker = shared_connection_block(m_colors_connection);
          if(m_fill_type->get() == FillType::SOLID) {
            m_colors->insert(m_band_colors->get(operation.m_index),
              operation.m_index);
          } else if(operation.m_index == 0) {
            m_colors->insert(m_band_colors->get(0), operation.m_index);
          } else if(operation.m_index == 1) {
            m_colors->insert(m_end_color, operation.m_index);
          } else {
            m_colors->set(1, m_band_colors->get(operation.m_index));
          }
          update_levels();
        },
        [&] (const ListModel<QColor>::RemoveOperation& operation) {
          if(operation.m_index < m_colors->get_size()) {
            auto blocker = shared_connection_block(m_colors_connection);
            m_colors->remove(operation.m_index);
          }
          update_levels();
        },
        [&] (const ListModel<QColor>::UpdateOperation& operation) {
          if(operation.m_index == m_band_colors->get_size() - 1 &&
              operation.m_index != 0) {
            m_end_color = operation.get_value();
          }
        });
    }

    void on_colors_operation(const ListModel<QColor>::Operation& operation) {
      visit(operation,
        [&] (const ListModel<QColor>::UpdateOperation& operation) {
          m_band_colors->m_is_blocked = true;
          if(m_fill_type->get() == FillType::GRADIENT) {
            if(operation.m_index == 0) {
              m_band_colors->set(operation.m_index, operation.get_value());
            } else {
              m_band_colors->set(m_band_colors->get_size() - 1,
                operation.get_value());
            }
            m_timer->start(DEBOUNCE_TIME_MS);
          } else {
            m_band_colors->set(operation.m_index, operation.get_value());
          }
          m_band_colors->m_is_blocked = false;
        });
    }
  };
}

struct BookViewLevelPropertiesPage::PriceLevelWidget : QWidget {
  QTimer m_timer;
  std::shared_ptr<PriceLevelPropertiesModel> m_model;
  std::shared_ptr<ValueModel<QFont>> m_font;
  QWidget* m_color_boxes;
  ScrollBox* m_scroll_box;
  QHBoxLayout* m_body_layout;
  QVBoxLayout* m_color_bands_layout;
  QVBoxLayout* m_color_boxes_layout;
  std::array<CheckBox*, 2> m_buttons;
  int m_gradient_color_levels;
  scoped_connection m_levels_connection;
  scoped_connection m_band_colors_connection;
  scoped_connection m_colors_connection;

  PriceLevelWidget(std::shared_ptr<ValueModel<FillType>> fill_type,
      std::shared_ptr<ValueModel<std::vector<QColor>>> color_scheme,
      std::shared_ptr<ValueModel<QFont>> font, QWidget* parent = nullptr)
      : QWidget(parent),
BEAM_SUPPRESS_THIS_INITIALIZER()
        m_timer(this),
BEAM_UNSUPPRESS_THIS_INITIALIZER()
        m_model(std::make_shared<PriceLevelPropertiesModel>(
          std::move(fill_type), std::make_shared<ArrayValueToListModel<QColor>>(
            std::move(color_scheme)), m_timer)),
        m_font(std::move(font)),
        m_gradient_color_levels(0) {
    auto body = new QWidget();
    m_body_layout = make_hbox_layout(body);
    auto color_bands = make_color_bands();
    color_bands->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    color_bands->setFixedWidth(scale_width(COLOR_BAND_WIDTH));
    m_body_layout->addWidget(color_bands, 0, Qt::AlignTop | Qt::AlignLeft);
    m_body_layout->addSpacing(scale_width(COLOR_LIST_SPACING));
    m_color_boxes = make_color_boxes();
    m_color_boxes->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Expanding);
    m_body_layout->addWidget(m_color_boxes, 0, Qt::AlignTop);
    m_scroll_box = new ScrollBox(body);
    m_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_scroll_box->get_body().installEventFilter(this);
    m_scroll_box->get_vertical_scroll_bar().installEventFilter(this);
    m_scroll_box->get_vertical_scroll_bar().connect_position_signal(
      std::bind_front(&PriceLevelWidget::on_position_update, this));
    update_style(*m_scroll_box, [] (auto& style) {
      style.get(ScrollBoxOverflow()).
        set(PaddingRight(scale_width(COLOR_LIST_RIGHT_PADDING)));
    });
    auto layout = make_vbox_layout(this);
    layout->setSpacing(scale_height(8));
    layout->addWidget(make_levels_slot(m_model->m_levels));
    layout->addWidget(make_fill_type_slot());
    layout->addWidget(m_scroll_box);
    m_levels_connection = m_model->m_levels->connect_update_signal(
      std::bind_front(&PriceLevelWidget::on_levels_update, this));
    m_band_colors_connection = m_model->m_band_colors->connect_operation_signal(
      std::bind_front(&PriceLevelWidget::on_band_colors_operation, this));
    m_colors_connection = m_model->m_colors->connect_operation_signal(
      std::bind_front(&PriceLevelWidget::on_colors_operation, this));
    on_type_update(m_model->m_fill_type->get(), true);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(watched == &m_scroll_box->get_vertical_scroll_bar()) {
      if(event->type() == QEvent::Show) {
        match(*m_scroll_box, ScrollBoxOverflow());
      } else if(event->type() == QEvent::Hide) {
        unmatch(*m_scroll_box, ScrollBoxOverflow());
      }
    } else if(watched == &m_scroll_box->get_body() &&
        event->type() == QEvent::Resize &&
        m_model->m_fill_type->get() == FillType::GRADIENT) {
      auto scroll_bar_width = [&] {
        if(m_scroll_box->get_vertical_scroll_bar().isVisible()) {
          return m_scroll_box->get_vertical_scroll_bar().sizeHint().width();
        }
        return 0;
      }();
      m_color_boxes->resize(
        m_scroll_box->width() - scroll_bar_width -
          scale_width(COLOR_LIST_RIGHT_PADDING) - m_color_boxes->x(),
        m_color_boxes->height());
    }
    return QWidget::eventFilter(watched, event);
  }

  void make_button(FillType type) {
    auto button = make_radio_button();
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->set_label(::to_text(type));
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
    band->setFixedHeight(scale_height(COLOR_BAND_HEIGHT));
    update_style(*band, [&] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(m_model->m_band_colors->get(index))).
        set(Font(m_font->get())).
        set(TextColor(apca_text_color(m_model->m_band_colors->get(index)))).
        set(TextAlign(Qt::AlignCenter));
    });
    link(*this, *band);
    return band;
  }

  ColorBox* make_color_box(int index) {
    auto color_box = new ColorBox(
      std::make_shared<ListValueModel<QColor>>(m_model->m_colors, index));
    color_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    color_box->setFixedHeight(scale_height(COLOR_BOX_HEIGHT));
    color_box->connect_submit_signal([=] (auto value) {
      m_model->m_band_colors->submit();
    });
    return color_box;
  }

  QWidget* make_color_bands() {
    auto body = new QWidget();
    m_color_bands_layout = make_vbox_layout(body);
    for(auto i = 0; i < m_model->m_band_colors->get_size(); ++i) {
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
    m_color_boxes_layout->setSpacing(scale_height(COLOR_BOXES_SPACING));
    for(auto i = 0; i < m_model->m_colors->get_size(); ++i) {
      m_color_boxes_layout->addWidget(make_color_box(i));
    }
    auto box = new Box(body);
    update_style(*box, [] (auto& style) {
      style.get(Any()).
        set(vertical_padding(scale_height(COLOR_BOXES_VERTICAL_PADDING)));
    });
    return box;
  }

  void update_color_boxes_geometry(int levels) {
    if(m_model->m_fill_type->get() != FillType::GRADIENT || levels == 0) {
      return;
    }
    auto color_levels = [&] {
      if(levels >= 2) {
        return 2;
      }
      return 1;
    }();
    m_gradient_color_levels = color_levels;
    auto vertical_paddings = COLOR_BOXES_VERTICAL_PADDING * 2;
    auto height = scale_height(vertical_paddings +
      COLOR_BOX_HEIGHT * color_levels +
      COLOR_BOXES_SPACING * (color_levels - 1));
    if(height != m_color_boxes->height()) {
      m_color_boxes->setGeometry(
        scale_width(COLOR_BAND_WIDTH + COLOR_LIST_SPACING),
        m_scroll_box->get_vertical_scroll_bar().get_position(),
        m_color_boxes->width(), height);
    }
  }

  void on_position_update(int position) {
    if(m_model->m_fill_type->get() == FillType::GRADIENT) {
      m_color_boxes->move(m_color_boxes->x(), position);
    }
  }

  void on_type_update(FillType type, bool value) {
    m_buttons[static_cast<int>(type)]->get_current()->set(value);
    if(value) {
      if(type == FillType::SOLID) {
        m_body_layout->addWidget(m_color_boxes, 0, Qt::AlignTop);
      } else {
        m_body_layout->removeWidget(m_color_boxes);
        if(auto levels = m_model->m_levels->get()) {
          update_color_boxes_geometry(*levels);
        } else {
          update_color_boxes_geometry(m_gradient_color_levels);
        }
      }
    }
  }

  void on_levels_update(const optional<int> levels) {
    if(!levels) {
      return;
    }
    update_color_boxes_geometry(*levels);
  }

  void on_band_colors_operation(const ListModel<QColor>::Operation& operation) {
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
            set(TextColor(apca_text_color(operation.get_value())));
        });
      });
  }

  void on_colors_operation(const ListModel<QColor>::Operation& operation) {
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
  m_price_level_widget = new PriceLevelWidget(
    make_field_value_model(m_current, &BookViewLevelProperties::m_fill_type),
    make_field_value_model(m_current, &BookViewLevelProperties::m_color_scheme),
    font_box->get_current());
  auto body = new QWidget();
  auto layout = make_vbox_layout(body);
  layout->addWidget(text_header);
  layout->addWidget(font_box);
  layout->addSpacing(scale_height(24));
  layout->addWidget(grid_check_box, 0, Qt::AlignLeft);
  layout->addSpacing(scale_height(24));
  layout->addWidget(price_level_header);
  layout->addWidget(m_price_level_widget);
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
    update_style(*m_price_level_widget, [&] (auto& style) {
      style.get(Any() > is_a<TextBox>()).set(Font(font));
    });
    m_font = font;
  }
}

#include "Spire/Ui/TableHeaderItem.hpp"
#include <unordered_map>
#include <QMouseEvent>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/FieldValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ResponsiveLabel.hpp"
#include "Spire/Ui/ToggleButton.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using FilterActive = Styles::StateSelector<void, struct FilterActiveTag>;
  using FilterControl = Styles::StateSelector<void, struct FilterControlTag>;
  using ResizeHandle = Styles::StateSelector<void, struct ResizeHandleTag>;
  using Resizeable = Styles::StateSelector<void, struct ResizeableTag>;
  using Sash = Styles::StateSelector<void, struct SashTag>;

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    auto hover_selector = Any() > (Sash() && !Hover()) < Hover();
    auto highlight_selector = (hover_selector && TableHeaderItem::Sortable()) ||
      Any() > (TableHeaderItem::FilterButton() && Hover()) < Any() ||
      FilterActive();
    style.get(Any() > TableHeaderItem::ActiveElement()).
      set(Visibility::INVISIBLE);
    style.get(Any() > FilterControl()).set(Visibility::NONE);
    style.get(Any() > (ResizeHandle() || Sash())).
      set(Visibility::NONE);
    style.get(highlight_selector > TableHeaderItem::Label()).
      set(TextColor(QColor(0x4B23A0)));
    style.get(highlight_selector > TableHeaderItem::ActiveElement()).
      set(BackgroundColor(QColor(0x4B23A0))).
      set(Visibility::VISIBLE);
    style.get((TableHeaderItem::Filtered() || FilterActive() ||
        hover_selector || FocusVisible()) > FilterControl()).
      set(Visibility::VISIBLE);
    style.get(TableHeaderItem::Filtered() > TableHeaderItem::FilterButton() >
        is_a<Icon>()).
      set(Fill(QColor(0x4B23A0)));
    style.get(FocusVisible() > is_a<Box>()).
      set(border_color(QColor(0x4B23A0)));
    style.get(Resizeable() > (ResizeHandle() || Sash())).
      set(Visibility::VISIBLE);
    style.get(Any() > (Sash() && Drag()) < Any() > ResizeHandle()).
      set(BackgroundColor(QColor(0x4B23A0)));
    return style;
  }

  struct SortIndicator : QWidget {
    static const QImage& ASCENDING_IMAGE() {
      static auto image =
        image_from_svg(":/Icons/sort_ascending.svg", scale(6, 6));
      return image;
    }

    static const QImage& DESCENDING_IMAGE() {
      static auto image =
        image_from_svg(":/Icons/sort_descending.svg", scale(6, 6));
      return image;
    }

    std::shared_ptr<ValueModel<TableHeaderItem::Order>> m_order;
    optional<TableHeaderItem::Order> m_current_order;
    scoped_connection m_order_connection;

    explicit SortIndicator(
        std::shared_ptr<ValueModel<TableHeaderItem::Order>> order)
        : m_order(std::move(order)) {
      auto layout = make_hbox_layout(this);
      on_order(m_order->get());
      m_order_connection = m_order->connect_update_signal(
        std::bind_front(&SortIndicator::on_order, this));
    }

    void on_order(TableHeaderItem::Order order) {
      if(m_current_order == order) {
        return;
      }
      m_current_order = order;
      if(auto previous_icon = layout()->takeAt(0)) {
        delete previous_icon->widget();
        delete previous_icon;
      }
      auto icon = [&] () -> QWidget* {
        if(order == TableHeaderItem::Order::ASCENDING) {
          return new Icon(ASCENDING_IMAGE());
        } else if(order == TableHeaderItem::Order::DESCENDING) {
          return new Icon(DESCENDING_IMAGE());
        } else if(order == TableHeaderItem::Order::NONE) {
          return new Box(nullptr);
        }
        return nullptr;
      }();
      if(icon) {
        icon->setFixedSize(scale(6, 6));
        update_style(*icon, [] (auto& style) {
          style.get(Any()).set(BackgroundColor(Qt::transparent));
          style.get(Any() > is_a<Icon>()).set(Fill(QColor(0x4B23A0)));
        });
        layout()->addWidget(icon);
      }
    }
  };

  auto make_filter_button() {
    static auto icon = image_from_svg(":/Icons/filter.svg", scale(6, 6));
    auto button = make_icon_button(icon);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFixedSize(scale(16, 16));
    button->setFocusPolicy(Qt::NoFocus);
    update_style(*button, [] (auto& style) {
      style.get(Any() > Body()).set(BackgroundColor(Qt::transparent));
      style.get(Any() > is_a<Icon>()).set(Fill(QColor(0xC8C8C8)));
      style.get(Press() || Any() > (Body() && Hover())).
        set(BackgroundColor(QColor(0xF2F2F2)));
    });
    return button;
  }

  struct SashLayer : QWidget {
    QWidget* m_sash;

    explicit SashLayer(QWidget* parent = nullptr)
        : QWidget(parent) {
      auto layout = make_hbox_layout(this);
      layout->addStretch(1);
      m_sash = new QWidget();
      m_sash->setFixedWidth(scale_width(4));
      m_sash->setCursor(Qt::SplitHCursor);
      match(*m_sash, Sash());
      layout->addWidget(m_sash);
    }

    void resizeEvent(QResizeEvent* event) override {
      setMask(m_sash->geometry());
      QWidget::resizeEvent(event);
    }
  };

  class HeaderNameListModel : public ListModel<QString> {
    public:
      explicit HeaderNameListModel(
          std::shared_ptr<ValueModel<TableHeaderItem::Model>> source)
          : m_source(std::move(source)),
            m_connection(m_source->connect_update_signal(
              std::bind_front(&HeaderNameListModel::on_update, this))) {
        on_update(m_source->get());
      }

      int get_size() const override {
        return m_names.get_size();
      }

      const QString& get(int index) const override {
        return m_names.get(index);
      }

      connection connect_operation_signal(
          const OperationSignal::slot_type& slot) const override {
        return m_names.connect_operation_signal(slot);
      }

    protected:
      void transact(const std::function<void()>& transaction) override {
        m_names.transact([&] { transaction(); });
      }

    private:
      enum class Type {
        NAME,
        SHORT_NAME
      };
      std::shared_ptr<ValueModel<TableHeaderItem::Model>> m_source;
      ArrayListModel<QString> m_names;
      std::unordered_map<Type, int> m_name_map;
      scoped_connection m_connection;

      void update(Type type, const QString& current) {
        if(current.isEmpty()) {
          if(auto index = m_name_map.find(type); index != m_name_map.end()) {
            m_names.remove(index->second);
            for(auto& name_index : m_name_map) {
              if(name_index.second > index->second) {
                name_index.second -= 1;
              }
            }
            m_name_map.erase(index);
          }
        } else if(auto index = m_name_map.find(type);
            index == m_name_map.end()) {
          m_names.push(current);
          m_name_map.emplace(type, m_names.get_size() - 1);
        } else {
          m_names.set(m_name_map.at(type), current);
        }
      }

      void on_update(const TableHeaderItem::Model& current) {
        update(Type::NAME, current.m_name);
        update(Type::SHORT_NAME, current.m_short_name);
      }
  };

  auto make_name_label(std::shared_ptr<HeaderNameListModel> model) {
    auto label = new ResponsiveLabel(std::move(model));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*label, [] (auto& style) {
      auto font = QFont("Roboto");
      font.setWeight(QFont::Medium);
      font.setPixelSize(scale_width(12));
      style.get(Any()).
        set(text_style(font, QColor(0x595959))).
        set(PaddingBottom(scale_height(4))).
        set(PaddingTop(scale_height(6)));
    });
    match(*label, TableHeaderItem::Label());
    return label;
  }

  auto make_filter_button(std::shared_ptr<BooleanModel> model) {
    static auto icon = image_from_svg(":/Icons/filter.svg", scale(16, 16));
    static auto close_icon =
      image_from_svg(":/Icons/filter-close.svg", scale(16, 16));
    auto button = make_icon_toggle_button(icon, close_icon, std::move(model));
    button->setFocusPolicy(Qt::NoFocus);
    update_style(*button, [] (auto& style) {
      style.get(Any() > Body()).set(BackgroundColor(Qt::transparent));
      style.get((Hover() || Press()) > Body()).
        set(BackgroundColor(QColor(0xF2F2FF)));
      style.get(Any() > is_a<Icon>()).set(Fill(QColor(0xA5A5A5)));
      style.get(((Hover() || Press()) && !Checked()) > is_a<Icon>()).
        set(Fill(QColor(0xA5A5A5)));
      style.get(Checked() > is_a<Icon>()).set(Fill(QColor(0x4B23A0)));
    });
    match(*button, TableHeaderItem::FilterButton());
    return button;
  }

  std::tuple<QWidget*, ToggleButton*> make_filter_control(
      std::shared_ptr<BooleanModel> model) {
    auto button = make_filter_button(std::move(model));
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    auto control = new QWidget();
    control->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    match(*control, FilterControl());
    enclose(*control, *button);
    return {control, button};
  }

  std::tuple<QWidget*, QWidget*> make_active_indicator() {
    auto box = new Box(nullptr);
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    match(*box, TableHeaderItem::ActiveElement());
    auto indicator = new QWidget();
    indicator->setFixedHeight(scale_height(2));
    enclose(*indicator, *box);
    return {indicator, box};
  }

  auto make_resize_handle() {
    auto resize_handle = new Box(nullptr);
    resize_handle->setFixedWidth(scale_width(1));
    resize_handle->setFixedHeight(scale_width(14));
    update_style(*resize_handle, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0xC8C8C8)));
    });
    match(*resize_handle, ResizeHandle());
    return resize_handle;
  }

  auto make_outline() {
    auto outline = new Box(nullptr);
    outline->setAttribute(Qt::WA_TransparentForMouseEvents);
    outline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    update_style(*outline, [] (auto& style) {
      style.get(Any()).set(border(scale_width(1), QColor(Qt::transparent)));
    });
    return outline;
  }
}

TableHeaderItem::TableHeaderItem(
    std::shared_ptr<ValueModel<Model>> model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_is_filter_open(std::make_shared<LocalBooleanModel>(false)),
      m_click_observer(*this),
      m_is_resizeable(true),
      m_is_resizing(false) {
  setFocusPolicy(Qt::StrongFocus);
  m_name_label =
    make_name_label(std::make_shared<HeaderNameListModel>(m_model));
  link(*this, *m_name_label);
  m_sort_indicator =
    new SortIndicator(make_field_value_model(m_model, &Model::m_order));
  link(*this, *m_sort_indicator);
  std::tie(m_active_indicator, m_active_element) = make_active_indicator();
  link(*this, *m_active_element);
  m_active_container = new QWidget();
  enclose(*m_active_container, *m_active_indicator);
  auto content_layout = make_vbox_layout();
  content_layout->addWidget(m_name_label);
  content_layout->addWidget(m_active_container);
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto body_layout = make_hbox_layout(body);
  body_layout->setSpacing(scale_width(5));
  body_layout->addLayout(content_layout, 1);
  m_controls_layout = make_hbox_layout();
  m_controls_layout->setSpacing(scale_width(3));
  m_controls_layout->addWidget(m_sort_indicator, 0, Qt::AlignVCenter);
  auto resize_handle = make_resize_handle();
  link(*this, *resize_handle);
  m_controls_layout->addWidget(resize_handle, 0, Qt::AlignVCenter);
  body_layout->addLayout(m_controls_layout);
  auto container_body = new LayeredWidget();
  container_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  container_body->add(body);
  auto sash_layer = new SashLayer();
  sash_layer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_sash = sash_layer->m_sash;
  m_sash->installEventFilter(this);
  link(*this, *m_sash);
  container_body->add(sash_layer);
  auto container = new Box(container_body);
  container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*container, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(PaddingLeft(scale_width(5)));
  });
  match(*container, Container());
  link(*this, *container);
  auto layers = new LayeredWidget();
  layers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layers->add(container);
  auto outline = make_outline();
  link(*this, *outline);
  layers->add(outline);
  enclose(*this, *layers);
  set_style(*this, DEFAULT_STYLE());
  on_update(m_model->get());
  on_label_style();
  on_label_name_update(m_name_label->get_current()->get());
  on_filter_checked(m_is_filter_open->get());
  if(m_is_resizeable) {
    match(*this, Resizeable());
  }
  m_is_filter_open->connect_update_signal(
    std::bind_front(&TableHeaderItem::on_filter_checked, this));
  m_connection = m_model->connect_update_signal(
    std::bind_front(&TableHeaderItem::on_update, this));
  m_label_style_connection = connect_style_signal(*m_name_label,
    std::bind_front(&TableHeaderItem::on_label_style, this));
  m_label_name_connection = m_name_label->get_current()->connect_update_signal(
    std::bind_front(&TableHeaderItem::on_label_name_update, this));
  m_click_observer.connect_click_signal(
    std::bind_front(&TableHeaderItem::on_click, this));
}

const std::shared_ptr<ValueModel<TableHeaderItem::Model>>&
    TableHeaderItem::get_model() const {
  return m_model;
}

const std::shared_ptr<BooleanModel>& TableHeaderItem::is_filter_open() const {
  return m_is_filter_open;
}

bool TableHeaderItem::is_resizeable() const {
  return m_is_resizeable;
}

void TableHeaderItem::set_is_resizeable(bool is_resizeable) {
  if(m_is_resizeable == is_resizeable) {
    return;
  }
  m_is_resizeable = is_resizeable;
  if(m_is_resizeable) {
    match(*this, Resizeable());
  } else {
    unmatch(*this, Resizeable());
  }
}

connection TableHeaderItem::connect_sort_signal(
    const SortSignal::slot_type& slot) const {
  return m_sort_signal.connect(slot);
}

QSize TableHeaderItem::minimumSizeHint() const {
  auto size_hint = QWidget::minimumSizeHint();
  if(!m_is_resizeable) {
    return {0, size_hint.height()};
  }
  return size_hint;
}

bool TableHeaderItem::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_sash && isEnabled()) {
    if(event->type() == QEvent::MouseButtonPress) {
      match(*m_sash, Drag());
      m_is_resizing = true;
      return true;
    } else if(event->type() == QEvent::MouseButtonRelease) {
      unmatch(*m_sash, Drag());
      m_is_resizing = false;
      return true;
    } else if(event->type() == QEvent::MouseMove && m_is_resizing) {
      auto& mouse_event = *static_cast<QMouseEvent*>(event);
      setFixedWidth(
        std::max(scale_width(16), mapFromGlobal(mouse_event.globalPos()).x()));
      return true;
    } else if(event->type() == QEvent::MouseButtonDblClick) {
      setFixedWidth(sizeHint().width());
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TableHeaderItem::keyPressEvent(QKeyEvent* event) {
  if(m_model->get().m_filter != TableFilter::Filter::NONE) {
    if(event->key() == Qt::Key_F &&
        (event->modifiers() & Qt::ControlModifier) &&
        (event->modifiers() & Qt::ShiftModifier)) {
      event->accept();
      if(!m_is_filter_open->get()) {
        m_is_filter_open->set(true);
      }
    } else if(event->key() == Qt::Key_Escape) {
      event->accept();
      if(m_is_filter_open->get()) {
        m_is_filter_open->set(false);
      }
    }
  }
  QWidget::keyPressEvent(event);
}

void TableHeaderItem::on_click() {
  auto order = m_model->get().m_order;
  if(order == Order::NONE || order == Order::DESCENDING) {
    m_sort_signal(Order::ASCENDING);
  } else if(order == Order::ASCENDING) {
    m_sort_signal(Order::DESCENDING);
  }
}

void TableHeaderItem::on_filter_checked(bool checked) {
  if(checked) {
    if(m_model->get().m_filter != TableFilter::Filter::NONE) {
      m_active_element->setMaximumWidth(QWIDGETSIZE_MAX);
      match(*this, FilterActive());
    }
  } else {
    m_active_element->setMaximumWidth(scale_width(18));
    unmatch(*this, FilterActive());
  }
}

void TableHeaderItem::on_update(const Model& model) {
  if(model.m_filter == TableFilter::Filter::NONE) {
    if(m_filter_control) {
      m_controls_layout->removeWidget(m_filter_control);
      m_filter_control->deleteLater();
      m_filter_control = nullptr;
    }
    m_is_filter_open->set(false);
  } else if(!m_filter_control) {
    auto [filter_control, filter_button] =
      make_filter_control(m_is_filter_open);
    m_filter_control = filter_control;
    link(*this, *filter_button);
    link(*this, *m_filter_control);
    m_controls_layout->insertWidget(1, m_filter_control);
  }
  m_sort_indicator->setVisible(model.m_order != Order::UNORDERED &&
    model.m_order != Order::NONE);
  auto& stylist = find_stylist(*this);
  if(model.m_order != Order::UNORDERED) {
    stylist.match(Sortable());
  } else {
    stylist.unmatch(Sortable());
  }
  if(model.m_filter == TableFilter::Filter::FILTERED) {
    stylist.match(Filtered());
  } else {
    stylist.unmatch(Filtered());
  }
  updateGeometry();
}

void TableHeaderItem::on_label_name_update(const QString& name) {
  m_active_indicator->setFixedWidth(
    QFontMetrics(m_font).horizontalAdvance(name));
}

void TableHeaderItem::on_label_style() {
  auto& stylist = find_stylist(*m_name_label);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const Font& font) {
        stylist.evaluate(font, [=] (const auto& font) {
          if(m_font != font) {
            m_font = font;
            on_label_name_update(m_name_label->get_current()->get());
          }
        });
      },
      [&] (const TextAlign& alignment) {
        stylist.evaluate(alignment, [=] (auto alignment) {
          auto layout = m_active_container->layout();
          if(layout->alignment() != alignment) {
            layout->setAlignment(alignment);
            layout->invalidate();
            layout->activate();
          }
        });
      });
  }
}

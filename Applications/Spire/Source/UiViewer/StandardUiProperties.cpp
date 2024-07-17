#include "Spire/UiViewer/StandardUiProperties.hpp"
#include <limits>
#include <QCheckBox>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QResizeEvent>
#include <QSpinBox>
#include <QTextEdit>
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/StyleParser/DataTypes/PropertyParser.hpp"
#include "Spire/StyleParser/DataTypes/SelectorParser.hpp"
#include "Spire/StyleParser/DataTypes/TokenParser.hpp"
#include "Spire/Ui/Window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct SizeFilter : QObject {
    TypedUiProperty<int>* m_width;
    TypedUiProperty<int>* m_height;

    SizeFilter(TypedUiProperty<int>* width,
      TypedUiProperty<int>* height, QObject* parent)
      : QObject(parent),
        m_width(width),
        m_height(height) {}

    bool eventFilter(QObject* object, QEvent* event) override {
      if(event->type() == QEvent::Resize) {
        auto& resizeEvent = static_cast<QResizeEvent&>(*event);
        if(scale_width(m_width->get()) != resizeEvent.size().width()) {
          m_width->set(unscale_width(resizeEvent.size().width()));
        }
        if(scale_height(m_height->get()) != resizeEvent.size().height()) {
          m_height->set(unscale_height(resizeEvent.size().height()));
        }
      }
      return QObject::eventFilter(object, event);
    }
  };

  class StyleEditorWindow : public Window {
    public:
      using SubmitTextSignal = Signal<void(const QString& style_text)>;

      explicit StyleEditorWindow(std::shared_ptr<TextModel> current,
          QWidget* parent = nullptr)
          : Window(parent),
            m_current(std::move(current)),
            m_submission(m_current->get()),
            m_current_connection(m_current->connect_update_signal(
              std::bind_front(&StyleEditorWindow::on_current, this))) {
        setWindowTitle(tr("Style Editor"));
        set_svg_icon(":/Icons/spire.svg");
        setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
        auto body = new QWidget();
        auto layout = new QVBoxLayout(body);
        m_editor = new QTextEdit();
        auto metrics = QFontMetricsF(m_editor->font());
        m_editor->setTabStopDistance(metrics.horizontalAdvance("  "));
        m_editor->setText(m_current->get());
        layout->addWidget(m_editor);
        auto buttons_layout = new QHBoxLayout();
        m_error_label = new QLabel();
        buttons_layout->addWidget(m_error_label);
        buttons_layout->addStretch(1);
        auto ok_button = new QPushButton("OK");
        buttons_layout->addWidget(ok_button);
        auto cancel_button = new QPushButton("Cancel");
        buttons_layout->addWidget(cancel_button);
        layout->addLayout(buttons_layout);
        set_body(body);
        connect(m_editor, &QTextEdit::textChanged,
          this, &StyleEditorWindow::on_text_changed);
        connect(ok_button, &QPushButton::clicked,
          this, &StyleEditorWindow::on_ok);
        connect(cancel_button, &QPushButton::clicked,
          this, &StyleEditorWindow::on_cancel);
      }

      const std::shared_ptr<TextModel>& get_current() const {
        return m_current;
      }

      void set_error_info(const QString& error_info) {
        m_error_label->setText(error_info);
      }

      connection connect_submit_text_signal(
          const SubmitTextSignal::slot_type& slot) const {
        return m_submit_text_signal.connect(slot);
      }

    private:
      mutable SubmitTextSignal m_submit_text_signal;
      std::shared_ptr<TextModel> m_current;
      QString m_submission;
      QLabel* m_error_label;
      QTextEdit* m_editor;
      boost::signals2::scoped_connection m_current_connection;

      void on_current(const QString& current) {
        m_editor->setText(current);
      }

      void on_text_changed() {
        auto blocker = shared_connection_block(m_current_connection);
        m_current->set(m_editor->toPlainText());
      }

      void on_ok() {
        m_submission = m_current->get();
        m_submit_text_signal(m_submission);
        close();
      }

      void on_cancel() {
        m_current->set(m_submission);
        close();
      }
  };

  auto parse_style(const QString& input) {
    auto style_sheet = StyleSheet();
      auto parser = TokenParser();
      parser.feed(input.toStdString());
      while(parser.get_size() > 0) {
        auto selector = parse_selector(parser,
          std::make_shared<DefaultSelectorParseStrategy>());
        auto& rule = style_sheet.get(selector);
        parse_block(parser, rule);
      }
    return style_sheet;
  }

  QString trim_text(const QString& text) {
    auto lines = text.split('\n', Qt::SkipEmptyParts);
    if(lines.empty()) {
      return "";
    }
    auto trimmed_lines = QStringList();
    auto i = std::find_if_not(lines[0].begin(), lines[0].end(), [] (auto& ch) {
      return ch.isSpace();
    });
    auto spaces = static_cast<int>(std::distance(lines[0].begin(), i));
    for(auto& line : lines) {
      trimmed_lines.append(line.remove(0, spaces));
    }
    return trimmed_lines.join('\n');
  }
}

void Spire::populate_widget_size_properties(const QString& width_name,
    const QString& height_name,
    std::vector<std::shared_ptr<UiProperty>>& properties) {
  properties.push_back(make_standard_property<int>(width_name));
  properties.push_back(make_standard_property<int>(height_name));
}

void Spire::apply_widget_size_properties(QWidget* widget,
    const QString& width_name, const QString& height_name,
    const std::vector<std::shared_ptr<UiProperty>>& properties) {
  auto& width = get<int>(width_name, properties);
  auto& height = get<int>(height_name, properties);
  width.connect_changed_signal([=] (auto value) {
    if(value != 0) {
      if(unscale_width(widget->width()) != value) {
        widget->setFixedWidth(scale_width(value));
      }
    }
  });
  height.connect_changed_signal([=] (auto value) {
    if(value != 0) {
      if(unscale_height(widget->height()) != value) {
        widget->setFixedHeight(scale_height(value));
      }
    }
  });
  widget->installEventFilter(new SizeFilter(&width, &height, widget));
}

void Spire::populate_widget_properties(
    std::vector<std::shared_ptr<UiProperty>>& properties) {
  properties.push_back(make_standard_property("enabled", true));
  populate_widget_size_properties("width", "height", properties);
}

void Spire::apply_widget_properties(QWidget* widget,
    const std::vector<std::shared_ptr<UiProperty>>& properties) {
  auto& enabled = get<bool>("enabled", properties);
  enabled.connect_changed_signal([=] (auto value) {
    widget->setEnabled(value);
  });
  apply_widget_size_properties(widget, "width", "height", properties);
}

template<>
std::shared_ptr<TypedUiProperty<bool>> Spire::make_standard_property<bool>(
    QString name, bool value) {
  return std::make_shared<StandardUiProperty<bool>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<bool>& property) {
      auto setter = new QCheckBox(parent);
      property.connect_changed_signal([=] (auto value) {
        if(value) {
          setter->setCheckState(Qt::Checked);
        } else {
          setter->setCheckState(Qt::Unchecked);
        }
      });
      QObject::connect(setter, &QCheckBox::stateChanged, [&] (auto value) {
        property.set(value == Qt::Checked);
      });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<CurrencyId>>
    Spire::make_standard_property<CurrencyId>(QString name, CurrencyId value) {
  return std::make_shared<StandardUiProperty<CurrencyId>>(std::move(name),
    value,
    [] (QWidget* parent, StandardUiProperty<CurrencyId>& property) {
      auto setter = new QLineEdit(parent);
      property.connect_changed_signal([=] (auto value) {
        auto code = GetDefaultCurrencyDatabase().FromId(value).m_code;
        setter->setText(QString::fromStdString(code.GetData()));
      });
      QObject::connect(setter, &QLineEdit::textChanged, [&] (const auto& text) {
        auto id = GetDefaultCurrencyDatabase().FromCode(
          text.toUpper().toStdString()).m_id;
        if(id != CurrencyId::NONE) {
          property.set(id);
        }
      });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<int>> Spire::make_standard_property<int>(
    QString name, int value) {
  return std::make_shared<StandardUiProperty<int>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<int>& property) {
      auto setter = new QSpinBox(parent);
      setter->setMinimum(std::numeric_limits<int>::min());
      setter->setMaximum(std::numeric_limits<int>::max());
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(value);
      });
      QObject::connect(setter, qOverload<int>(&QSpinBox::valueChanged),
        [&] (auto value) {
          property.set(value);
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<std::int64_t>>
    Spire::make_standard_property<std::int64_t>(
      QString name, std::int64_t value) {
  return std::make_shared<StandardUiProperty<std::int64_t>>(std::move(name),
    value,
    [] (QWidget* parent, StandardUiProperty<std::int64_t>& property) {
      auto setter = new QSpinBox(parent);
      setter->setMinimum(std::numeric_limits<int>::min());
      setter->setMaximum(std::numeric_limits<int>::max());
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(static_cast<int>(value));
      });
      QObject::connect(setter, qOverload<int>(&QSpinBox::valueChanged),
        [&] (auto value) {
          property.set(value);
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<Money>> Spire::make_standard_property<Money>(
    QString name, Money value) {
  return std::make_shared<StandardUiProperty<Money>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<Money>& property) {
      auto setter = new QDoubleSpinBox(parent);
      setter->setMinimum(std::numeric_limits<double>::lowest());
      setter->setMaximum(std::numeric_limits<double>::max());
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(static_cast<double>(value));
      });
      QObject::connect(setter, &QDoubleSpinBox::textChanged,
        [&] (const auto& value) {
          if(auto money = Money::FromValue(value.toStdString())) {
            property.set(*money);
          }
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<Quantity>>
    Spire::make_standard_property<Quantity>(QString name, Quantity value) {
  return std::make_shared<StandardUiProperty<Quantity>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<Quantity>& property) {
      auto setter = new QDoubleSpinBox(parent);
      setter->setMinimum(std::numeric_limits<double>::lowest());
      setter->setMaximum(std::numeric_limits<double>::max());
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(static_cast<double>(value));
      });
      QObject::connect(setter, &QDoubleSpinBox::textChanged,
        [&] (const auto& value) {
          if(auto quantity = Quantity::FromValue(value.toStdString())) {
            property.set(*quantity);
          }
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<Decimal>>
    Spire::make_standard_property<Decimal>(QString name, Decimal value) {
  return std::make_shared<StandardUiProperty<Decimal>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<Decimal>& property) {
      auto setter = new QDoubleSpinBox(parent);
      setter->setMinimum(std::numeric_limits<double>::lowest());
      setter->setMaximum(std::numeric_limits<double>::max());
      auto parse_decimal = [] (auto decimal) -> std::optional<Decimal> {
        try {
          return Decimal(decimal.toStdString().c_str());
        } catch(const std::exception&) {
          return {};
        }
      };
      property.connect_changed_signal([=] (auto value) {
        setter->setValue(static_cast<double>(value));
      });
      QObject::connect(setter, &QDoubleSpinBox::textChanged,
        [&] (const auto& value) {
          if(auto decimal = parse_decimal(value)) {
            property.set(*decimal);
          }
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<QColor>>
    Spire::make_standard_property<QColor>(QString name) {
  return make_standard_property(std::move(name), QColorConstants::White);
}

template<>
std::shared_ptr<TypedUiProperty<QColor>>
    Spire::make_standard_property<QColor>(QString name, QColor value) {
  return std::make_shared<StandardUiProperty<QColor>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<QColor>& property) {
      auto setter = new QLineEdit(property.get().name().toUpper(), parent);
      property.connect_changed_signal([=] (auto value) {
        setter->setText(value.name());
      });
      QObject::connect(setter, &QLineEdit::textChanged,
        [&] (const auto& text) {
          if(text.length() == 7) {
            auto color = QColor(text.toUpper());
            if(color.isValid()) {
              property.set(color);
            }
          }
        });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<QString>>
    Spire::make_standard_property<QString>(QString name, QString value) {
  return std::make_shared<StandardUiProperty<QString>>(std::move(name), value,
    [] (QWidget* parent, StandardUiProperty<QString>& property) {
      auto setter = new QLineEdit(property.get(), parent);
      property.connect_changed_signal([=] (const auto& value) {
        setter->setText(value);
      });
      QObject::connect(setter, &QLineEdit::textChanged, [&] (const auto& text) {
        property.set(text);
      });
      return setter;
    });
}

template<>
std::shared_ptr<TypedUiProperty<DateFormat>>
    Spire::make_standard_property<DateFormat>(QString name, DateFormat value) {
  auto formats = define_enum<DateFormat>(
    {{"YYYYMMDD", DateFormat::YYYYMMDD}, {"MMDD", DateFormat::MMDD}});
  return make_standard_enum_property(std::move(name), value, formats);
}

template<>
std::shared_ptr<TypedUiProperty<DateFormat>>
    Spire::make_standard_property<DateFormat>(QString name) {
  return make_standard_property(std::move(name), DateFormat::YYYYMMDD);
}

std::shared_ptr<TypedUiProperty<optional<StyleSheet>>>
    Spire::make_style_property(QString name, QString style_text) {
  auto style_text_model =
    std::make_shared<LocalTextModel>(trim_text(style_text));
  return std::make_shared<StandardUiProperty<optional<StyleSheet>>>(
    std::move(name), none,
    [style_text_model, style_text = style_text_model->get()] (QWidget* parent,
        StandardUiProperty<optional<StyleSheet>>& property) {
      auto widget = new QWidget(parent);
      auto layout = make_hbox_layout(widget);
      auto line_editor = QPointer<QLineEdit>(
        new QLineEdit(style_text_model->get()));
      layout->addWidget(line_editor);
      auto button = new QPushButton("...");
      button->setFixedSize(scale_width(20), line_editor->sizeHint().height());
      layout->addWidget(button);
      QObject::connect(button, &QPushButton::clicked, [=, &property] () {
        auto style_editor = QPointer<StyleEditorWindow>(
          new StyleEditorWindow(style_text_model, parent));
        style_editor->setAttribute(Qt::WA_DeleteOnClose);
        style_editor->get_current()->connect_update_signal(
          [&, style_editor] (auto& current) {
            if(!style_editor) {
              return;
            }
            try {
              property.set(parse_style(current));
              style_editor->set_error_info("");
            } catch(std::exception& e) {
              style_editor->set_error_info(e.what());
            }
          });
        style_editor->connect_submit_text_signal([=] (const auto& text) {
          if(line_editor) {
            line_editor->setText(text);
          }
        });
        style_editor->show();
      });
      if(!property.get()) {
        style_text_model->set(style_text);
        if(line_editor) {
          line_editor->setText(style_text);
        }
        if(!style_text.isEmpty()) {
          try {
            property.set(parse_style(style_text));
          } catch(std::exception&) {
          }
        }
      }
      return widget;
    });
}

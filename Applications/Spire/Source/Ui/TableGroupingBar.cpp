#include "Spire/Ui/TableGroupingBar.hpp"
#include <algorithm>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/MenuButton.hpp"
#include "Spire/Ui/Tag.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  class GroupByButton final : public MenuButton {
    public:
      explicit GroupByButton(QString label)
          : MenuButton(*new QWidget()),
            m_label(std::move(label)),
            m_focus(*this) {
        get_body().setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_Hover);
        setCursor(Qt::PointingHandCursor);
        setAccessibleName(m_label);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        auto font = QFont("Roboto");
        font.setPixelSize(scale_height(13));
        font.setWeight(QFont::Medium);
        setFont(font);
        m_focus.connect_state_signal([=, this] (auto) {
          update();
        });
      }

      QSize sizeHint() const override {
        return QSize(scale_width(7) + icon().width() + scale_width(4) +
          fontMetrics().horizontalAdvance(m_label) + scale_width(9),
          scale_height(24));
      }

    protected:
      void paintEvent(QPaintEvent*) override {
        auto background = QColor(0xF5F2FF);
        auto border = QColor(0xC7BAFF);
        auto text = QColor(0x684BC7);
        if(isEnabled() && underMouse()) {
          background = QColor(0xE2E0FF);
          border = QColor(0x4B23A0);
          text = border;
        }
        auto pen = QPen(border, scale_width(1.0), Qt::DashLine);
        if(isEnabled() &&
            m_focus.get_state() == FocusObserver::State::FOCUS_VISIBLE) {
          pen.setColor(QColor(0x684BC7));
          pen.setStyle(Qt::SolidLine);
        }
        auto painter = QPainter(this);
        if(!isEnabled()) {
          painter.setOpacity(0.4);
        }
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(pen);
        painter.setBrush(background);
        auto inset = pen.widthF() / 2;
        painter.drawRoundedRect(QRectF(rect()).adjusted(
          inset, inset, -inset, -inset), scale_width(4), scale_height(4));
        auto left = scale_width(7);
        painter.drawImage(left, (height() - icon().height()) / 2, icon());
        painter.setPen(text);
        painter.drawText(rect().adjusted(
          left + icon().width() + scale_width(4), 0, -scale_width(9), 0),
          Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, m_label);
      }

    private:
      QString m_label;
      FocusObserver m_focus;

      static const QImage& icon() {
        static const auto ICON =
          image_from_svg(":/Icons/plus-circle.svg", scale(14, 14));
        return ICON;
      }
  };

  Icon* make_separator() {
    static const auto ARROW =
      image_from_svg(":/Icons/arrow-right.svg", scale(6, 8));
    auto icon = new Icon(ARROW);
    icon->setFixedSize(icon->sizeHint());
    auto style = StyleSheet();
    style.get(Any()).set(Fill(QColor(0xA0A0A0)));
    set_style(*icon, std::move(style));
    return icon;
  }
}

TableGroupingBar::TableGroupingBar(std::shared_ptr<GroupedTableModel> table,
    const QStringList& columns, QWidget* parent)
    : QWidget(parent),
      m_table(std::move(table)),
      m_tags(make_hbox_layout()) {
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  auto layout = make_hbox_layout(this);
  layout->setContentsMargins(
    scale_width(8), scale_height(4), scale_width(8), scale_height(4));
  layout->setSpacing(scale_width(4));
  m_tags->setSpacing(scale_width(4));
  layout->addLayout(m_tags);
  auto button = new GroupByButton(tr("Group by"));
  layout->addWidget(button);
  layout->addStretch();
  for(auto& name : columns) {
    auto index = static_cast<int>(m_columns.size());
    auto selected = button->get_menu().add_check_box(name);
    auto connection = selected->connect_update_signal([=, this] (auto value) {
      on_select(index, value);
    });
    m_columns.emplace_back(name, std::move(selected), std::move(connection));
  }
  button->get_menu().connect_submit_signal([=] (const auto&, const auto&) {
    button->get_menu().hide();
  });
  on_grouping(m_table->get_grouping());
  m_grouping_connection = m_table->connect_grouping_signal(
    std::bind_front(&TableGroupingBar::on_grouping, this));
}

void TableGroupingBar::on_select(int column, bool selected) {
  auto grouping = m_table->get_grouping();
  auto i = std::ranges::find(grouping, column);
  if(selected == (i != grouping.end())) {
    return;
  } else if(selected) {
    grouping.push_back(column);
  } else {
    grouping.erase(i);
  }
  m_table->set_grouping(std::move(grouping));
}

void TableGroupingBar::on_grouping(const std::vector<int>& grouping) {
  while(auto item = m_tags->takeAt(0)) {
    item->widget()->hide();
    item->widget()->deleteLater();
    delete item;
  }
  for(auto index : grouping) {
    auto& column = m_columns.at(index);
    if(m_tags->count() != 0) {
      m_tags->addWidget(make_separator(), 0, Qt::AlignVCenter);
    }
    auto tag = new Tag(column.m_name);
    tag->connect_delete_signal([selected = column.m_selected] {
      selected->set(false);
    });
    m_tags->addWidget(tag);
  }
  for(auto i = 0; i != std::ssize(m_columns); ++i) {
    auto& column = m_columns[i];
    auto selected = std::ranges::contains(grouping, i);
    if(column.m_selected->get() != selected) {
      column.m_selected->set(selected);
    }
  }
}

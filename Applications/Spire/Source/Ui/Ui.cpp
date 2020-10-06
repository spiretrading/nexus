#include "Spire/Ui/Ui.hpp"
#include <Beam/Collections/EnumIterator.hpp>
#include <QIcon>
#include <QPainter>
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

QImage Spire::imageFromSvg(const QString& path, const QSize& size) {
  return imageFromSvg(path, size, QRect(0, 0, size.width(), size.height()));
}

QImage Spire::imageFromSvg(const QString& path, const QSize& size,
    const QRect& box) {
  // TODO: Revert this when Qt fixes the regression.
  // https://bugreports.qt.io/browse/QTBUG-81259
  auto svg_pixmap = QIcon(path).pixmap(box.width(), box.height());
  auto image = QImage(size, QImage::Format_ARGB32);
  image.fill(QColor(0, 0, 0, 0));
  auto painter = QPainter(&image);
  painter.drawPixmap(box.topLeft(), svg_pixmap);
  return image;
}

QHeaderView* Spire::make_fixed_header(QWidget* parent) {
  auto header = new QHeaderView(Qt::Horizontal, parent);
  header->setFixedHeight(scale_height(30));
  header->setStretchLastSection(true);
  header->setSectionsClickable(false);
  header->setSectionsMovable(false);
  header->setSectionResizeMode(QHeaderView::Fixed);
  header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  header->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    })").arg(scale_width(8)).arg(scale_height(12)));
  return header;
}

QHeaderView* Spire::make_header(QWidget* parent) {
  auto header = make_fixed_header(parent);
  header->setSectionsMovable(true);
  header->setSectionResizeMode(QHeaderView::Interactive);
  header->setStyleSheet(QString(R"(
    QHeaderView {
      padding-left: %1px;
    }

    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:/Icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)).arg(scale_height(12)));
  return header;
}

std::vector<QVariant> Spire::make_currency_list(
    const CurrencyDatabase& database) {
  auto entries = database.GetEntries();
  return [&] {
    auto currencies = std::vector<QVariant>();
    currencies.reserve(entries.size());
    std::transform(entries.begin(), entries.end(),
      std::back_inserter(currencies), [] (const auto& entry) {
        return QVariant::fromValue(entry.m_id);
      });
    return currencies;
  }();
}

std::vector<QVariant> Spire::make_market_list(
    const MarketDatabase& database) {
  auto entries = database.GetEntries();
  return [&] {
    auto markets = std::vector<QVariant>();
    markets.reserve(entries.size());
    std::transform(entries.begin(), entries.end(),
      std::back_inserter(markets), [] (const auto& entry) {
        return QVariant::fromValue<MarketToken>(entry.m_code);
      });
    return markets;
  }();
}

std::vector<QVariant> Spire::make_order_status_list() {
  static auto list = [] {
    auto statuses = std::vector<QVariant>();
    statuses.reserve(OrderStatus::COUNT);
    for(auto status : Beam::MakeRange<OrderStatus>()) {
      statuses.push_back(QVariant::fromValue(status));
    }
    return statuses;
  }();
  return list;
}

std::vector<QVariant> Spire::make_order_type_list() {
  static auto list = [] {
    auto types = std::vector<QVariant>();
    types.reserve(OrderType::COUNT);
    for(auto type : Beam::MakeRange<OrderType>()) {
      types.push_back(QVariant::fromValue(type));
    }
    return types;
  }();
  return list;
}

std::vector<QVariant> Spire::make_side_list() {
  static auto list = std::vector<QVariant>(
    {QVariant::fromValue<Side>(Side::ASK),
    QVariant::fromValue<Side>(Side::BID)});
  return list;
}

std::vector<QVariant> Spire::make_time_in_force_list() {
  static auto list = [] {
    auto times = std::vector<QVariant>();
    times.reserve(TimeInForce::Type::COUNT);
    for(auto time : Beam::MakeRange<TimeInForce::Type>()) {
      times.push_back(QVariant::fromValue<TimeInForce>(time));
    }
    return times;
  }();
  return list;
}

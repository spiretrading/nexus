#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/MoneyBox.hpp"
#include "Spire/Ui/QuantityBox.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("DecimalBox") {
  UI_TEST_CASE("money_label_formatting") {
    auto current = std::make_shared<LocalMoneyModel>(Money(84.2));
    auto label = std::unique_ptr<MoneyBox>(make_money_label(current));
    label->show();
    QApplication::processEvents();
    auto text = label->get_text();
    REQUIRE(text->get() == "84.20");
    REQUIRE(to_decimal(current->get()) == Decimal("84.20"));
    auto verify = [&] (Money value, const QString& expected) {
      current->set(value);
      REQUIRE(text->get() == expected);
      REQUIRE(current->get() == value);
    };
    verify(Money(-1.000000000000001), "-1.00");
    verify(Money(1.000000000000001), "1.00");
    verify(parse_money("1.234567"), "1.234567");
    verify(parse_money("-1.234567"), "-1.234567");
    verify(Money::CENT, "0.01");
    verify(Money::ZERO, "0.00");
  }

  UI_TEST_CASE("quantity_formatting") {
    auto current =
      std::make_shared<LocalOptionalQuantityModel>(Quantity(84.2));
    auto box = QuantityBox(current);
    REQUIRE(box.get_text()->get() == "84.2");
    REQUIRE(to_decimal(*current->get()) == Decimal("84.2"));
    auto verify = [&] (Quantity value, const QString& expected) {
      current->set(value);
      REQUIRE(box.get_text()->get() == expected);
      REQUIRE(*current->get() == value);
    };
    verify(Quantity(-1.000000000000001), "-1");
    verify(Quantity(1.000000000000001), "1");
    verify(parse_quantity("1.234567"), "1.234567");
    verify(parse_quantity("-1.234567"), "-1.234567");
    verify(Quantity(0), "0");
    current->set(boost::none);
    REQUIRE(box.get_text()->get().isEmpty());
  }

  TEST_CASE("validate_narrow_range") {
    CHECK(DecimalBox::validate(1, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(2, Decimal(29), Decimal(31)) ==
      QValidator::Intermediate);
    CHECK(DecimalBox::validate(3, Decimal(29), Decimal(31)) ==
      QValidator::Intermediate);
    CHECK(DecimalBox::validate(4, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(5, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(6, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(7, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(8, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(9, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(10, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(20, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(28, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
    CHECK(DecimalBox::validate(29, Decimal(29), Decimal(31)) ==
      QValidator::Acceptable);
    CHECK(DecimalBox::validate(30, Decimal(29), Decimal(31)) ==
      QValidator::Acceptable);
    CHECK(DecimalBox::validate(31, Decimal(29), Decimal(31)) ==
      QValidator::Acceptable);
    CHECK(DecimalBox::validate(32, Decimal(29), Decimal(31)) ==
      QValidator::Invalid);
  }

  TEST_CASE("validate_overshoot") {
    CHECK(DecimalBox::validate(3, Decimal(5), Decimal(100)) ==
      QValidator::Intermediate);
    CHECK(DecimalBox::validate(2900, Decimal(29301), Decimal(31700)) ==
      QValidator::Invalid);
  }

  TEST_CASE("validate_negatives") {
    CHECK(DecimalBox::validate(-4, Decimal(-50), Decimal(-10)) ==
      QValidator::Intermediate);
    CHECK(DecimalBox::validate(-20, Decimal(-50), Decimal(-10)) ==
      QValidator::Acceptable);
    CHECK(DecimalBox::validate(-1, Decimal(50), Decimal(100)) ==
      QValidator::Invalid);
  }

  TEST_CASE("validate_fractional") {
    CHECK(DecimalBox::validate(Decimal("0.3"), Decimal("0.1"),
      Decimal("0.4")) == QValidator::Acceptable);
    CHECK(DecimalBox::validate(Decimal("0"), Decimal("0.29"),
      Decimal("0.31")) == QValidator::Intermediate);
    CHECK(DecimalBox::validate(Decimal("0.2"), Decimal("0.29"),
      Decimal("0.31")) == QValidator::Intermediate);
    CHECK(DecimalBox::validate(Decimal("0.02"), Decimal("0.29"),
      Decimal("0.31")) == QValidator::Invalid);
    CHECK(DecimalBox::validate(Decimal("0.21"), Decimal("0.29"),
      Decimal("0.31")) == QValidator::Invalid);
    CHECK(DecimalBox::validate(Decimal("0.32"), Decimal("0.29"),
      Decimal("0.31")) == QValidator::Invalid);
    CHECK(DecimalBox::validate(Decimal("-0.3"), Decimal("-0.4"),
      Decimal("-0.1")) == QValidator::Acceptable);
    CHECK(DecimalBox::validate(Decimal("-0"), Decimal("-0.31"),
      Decimal("-0.29")) == QValidator::Intermediate);
    CHECK(DecimalBox::validate(Decimal("-6.3"), Decimal("-6.35"),
      Decimal("-6.27")) == QValidator::Acceptable);
    CHECK(DecimalBox::validate(Decimal("-6.32"), Decimal("-6.35"),
      Decimal("-6.27")) == QValidator::Acceptable);
    CHECK(DecimalBox::validate(Decimal("-6.37"), Decimal("-6.35"),
      Decimal("-6.27")) == QValidator::Invalid);
    CHECK(DecimalBox::validate(Decimal("-6.2"), Decimal("-6.35"),
      Decimal("-6.27")) == QValidator::Intermediate);
    CHECK(DecimalBox::validate(Decimal("-6.2"), Decimal("-7.35"),
      Decimal("-6.27")) == QValidator::Intermediate);
    CHECK(DecimalBox::validate(Decimal("-6.21"), Decimal("-6.35"),
      Decimal("-6.27")) == QValidator::Invalid);
    CHECK(DecimalBox::validate(Decimal("-6.28"), Decimal("-6.35"),
      Decimal("-6.27")) == QValidator::Acceptable);
    CHECK(DecimalBox::validate(Decimal("0.60"), Decimal("0.606"),
      Decimal("0.611")) == QValidator::Intermediate);
    CHECK(DecimalBox::validate(Decimal("6.00"), Decimal("6.006"),
      Decimal("6.611")) == QValidator::Intermediate);
  }
}

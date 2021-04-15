#include <doctest/doctest.h>
#include "Spire/Ui/DecimalBox.hpp"

using namespace Spire;

TEST_SUITE("DecimalBox") {
  TEST_CASE("validate_narrow_range") {
    CHECK(DecimalBox::validate(
      1, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      2, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Intermediate);
    CHECK(DecimalBox::validate(
      3, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Intermediate);
    CHECK(DecimalBox::validate(
      4, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      5, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      6, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      7, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      8, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      9, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      10, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      20, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      28, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
    CHECK(DecimalBox::validate(
      29, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Acceptable);
    CHECK(DecimalBox::validate(
      30, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Acceptable);
    CHECK(DecimalBox::validate(
      31, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Acceptable);
    CHECK(DecimalBox::validate(
      32, DecimalBox::Decimal(29), DecimalBox::Decimal(31)) ==
        QValidator::Invalid);
  }

  TEST_CASE("validate_overshoot") {
    CHECK(DecimalBox::validate(
      3, DecimalBox::Decimal(5), DecimalBox::Decimal(100)) ==
        QValidator::Intermediate);
    CHECK(DecimalBox::validate(
      2900, DecimalBox::Decimal(29301), DecimalBox::Decimal(31700)) ==
        QValidator::Invalid);
  }

  TEST_CASE("validate_negatives") {
    CHECK(DecimalBox::validate(
      -4, DecimalBox::Decimal(-50), DecimalBox::Decimal(-10)) ==
        QValidator::Intermediate);
    CHECK(DecimalBox::validate(
      -20, DecimalBox::Decimal(-50), DecimalBox::Decimal(-10)) ==
        QValidator::Acceptable);
    CHECK(DecimalBox::validate(
      -1, DecimalBox::Decimal(50), DecimalBox::Decimal(100)) ==
        QValidator::Invalid);
  }

  TEST_CASE("validate_fractional") {
    CHECK(DecimalBox::validate(DecimalBox::Decimal("0.3"),
      DecimalBox::Decimal("0.1"), DecimalBox::Decimal("0.4")) ==
        QValidator::Acceptable);
    CHECK(DecimalBox::validate(DecimalBox::Decimal("0.2"),
      DecimalBox::Decimal("0.29"), DecimalBox::Decimal("0.31")) ==
        QValidator::Intermediate);
    CHECK(DecimalBox::validate(DecimalBox::Decimal("0.02"),
      DecimalBox::Decimal("0.29"), DecimalBox::Decimal("0.31")) ==
        QValidator::Intermediate);
    CHECK(DecimalBox::validate(DecimalBox::Decimal("0.32"),
      DecimalBox::Decimal("0.29"), DecimalBox::Decimal("0.31")) ==
        QValidator::Invalid);
  }
}

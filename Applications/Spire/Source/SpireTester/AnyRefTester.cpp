#include <doctest/doctest.h>
#include <string>
#include "Spire/Spire/AnyRef.hpp"

using namespace Spire;

TEST_SUITE("AnyRef") {
  TEST_CASE("empty") {
    auto any = AnyRef();
    REQUIRE(any.get() == nullptr);
    REQUIRE(!any.has_value());
    REQUIRE(any.get_type() == typeid(void));
    REQUIRE(!any.is_const());
    REQUIRE(!any.is_volatile());
    REQUIRE(!any.is_const_volatile());
  }

  TEST_CASE("ref") {
    auto value = 123;
    auto any = AnyRef(value);
    REQUIRE(any.get() == &value);
    REQUIRE(any.has_value());
    REQUIRE(any.get_type() == typeid(int));
    REQUIRE(!any.is_const());
    REQUIRE(!any.is_volatile());
    REQUIRE(!any.is_const_volatile());
    REQUIRE(&any_cast<int>(any) == &value);
    REQUIRE(&any_cast<const int>(any) == &value);
    REQUIRE_THROWS_AS(any_cast<volatile int>(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<const volatile int>(any), std::bad_any_cast);
    REQUIRE(any_cast<int>(&any) == &value);
    REQUIRE(any_cast<const int>(&any) == &value);
    REQUIRE(any_cast<volatile int>(&any) == nullptr);
    REQUIRE(any_cast<const volatile int>(&any) == nullptr);
    REQUIRE(any_cast<bool>(&any) == nullptr);
  }

  TEST_CASE("const_ref") {
    auto value = false;
    auto any = AnyRef(value);
    REQUIRE(any.get_type() == typeid(bool));
    REQUIRE(any.is_const());
    REQUIRE(!any.is_volatile());
    REQUIRE(!any.is_const_volatile());
    REQUIRE(&any_cast<const bool>(any) == &value);
    REQUIRE_THROWS_AS(any_cast<bool>(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<volatile bool>(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<const volatile bool>(any), std::bad_any_cast);
    REQUIRE(any_cast<bool>(&any) == nullptr);
    REQUIRE(any_cast<const bool>(&any) == &value);
    REQUIRE(any_cast<volatile bool>(&any) == nullptr);
    REQUIRE(any_cast<const volatile bool>(&any) == nullptr);
  }

  TEST_CASE("volatile_ref") {
    volatile auto value = 3.14;
    auto any = AnyRef(value);
    REQUIRE(any.get_type() == typeid(double));
    REQUIRE(!any.is_const());
    REQUIRE(any.is_volatile());
    REQUIRE(!any.is_const_volatile());
    REQUIRE_THROWS_AS(any_cast<const double>(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<double>(any), std::bad_any_cast);
    REQUIRE(&any_cast<volatile double>(any) == &value);
    REQUIRE(&any_cast<const volatile double>(any) == &value);
    REQUIRE(any_cast<double>(&any) == nullptr);
    REQUIRE(any_cast<const double>(&any) == nullptr);
    REQUIRE(any_cast<volatile double>(&any) == &value);
    REQUIRE(any_cast<const volatile double>(&any) == &value);
  }

  TEST_CASE("const_volatile_ref") {
    volatile const auto value = new std::string("abc");
    auto any = AnyRef(value);
    REQUIRE(any.get_type() == typeid(std::string));
    REQUIRE(any.is_const());
    REQUIRE(any.is_volatile());
    REQUIRE(any.is_const_volatile());
    REQUIRE_THROWS_AS(any_cast<const std::string>(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<std::string>(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<volatile std::string>(any), std::bad_any_cast);
    REQUIRE(&any_cast<const volatile std::string>(any) == value);
    REQUIRE(any_cast<std::string>(&any) == nullptr);
    REQUIRE(any_cast<const std::string>(&any) == nullptr);
    REQUIRE(any_cast<volatile std::string>(&any) == nullptr);
    REQUIRE(any_cast<const volatile std::string>(&any) == value);
    delete value;
  }
}

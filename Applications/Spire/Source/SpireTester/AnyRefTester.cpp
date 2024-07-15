#include <string>
#include <doctest/doctest.h>
#include "Spire/Spire/AnyRef.hpp"

using namespace Spire;

TEST_SUITE("AnyRef") {
  TEST_CASE("empty") {
    auto any = AnyRef();
    REQUIRE(!any.has_value());
    REQUIRE(any.get_type() == typeid(void));
    REQUIRE(!any.is_const());
    REQUIRE(!any.is_volatile());
    REQUIRE(!any.is_const_volatile());
  }

  TEST_CASE("ref") {
    auto value = 123;
    auto any = AnyRef(value);
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
    REQUIRE((any_cast<volatile int>(&any) == nullptr));
    REQUIRE((any_cast<const volatile int>(&any) == nullptr));
    REQUIRE(any_cast<bool>(&any) == nullptr);
  }

  TEST_CASE("const_ref") {
    const auto value = false;
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
    REQUIRE((any_cast<volatile bool>(&any) == nullptr));
    REQUIRE((any_cast<const volatile bool>(&any) == nullptr));
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
    REQUIRE((&any_cast<volatile double>(any) == &value));
    REQUIRE((&any_cast<const volatile double>(any) == &value));
    REQUIRE(any_cast<double>(&any) == nullptr);
    REQUIRE(any_cast<const double>(&any) == nullptr);
    REQUIRE((any_cast<volatile double>(&any) == &value));
    REQUIRE((any_cast<const volatile double>(&any) == &value));
  }

  TEST_CASE("const_volatile_ref") {
    auto r = std::string("abc");
    volatile const auto& value = r;
    auto any = AnyRef(value);
    REQUIRE(any.get_type() == typeid(std::string));
    REQUIRE(any.is_const());
    REQUIRE(any.is_volatile());
    REQUIRE(any.is_const_volatile());
    REQUIRE_THROWS_AS(any_cast<const std::string>(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<std::string>(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<volatile std::string>(any), std::bad_any_cast);
    REQUIRE((&any_cast<const volatile std::string>(any) == &value));
    REQUIRE(any_cast<std::string>(&any) == nullptr);
    REQUIRE(any_cast<const std::string>(&any) == nullptr);
    REQUIRE((any_cast<volatile std::string>(&any) == nullptr));
    REQUIRE((any_cast<const volatile std::string>(&any) == &value));
  }

  TEST_CASE("copy_construtor_assigment") {
    volatile auto value = 3.14;
    auto any1 = AnyRef(value);
    auto any2 = any1;
    REQUIRE(any2.get_type() == typeid(double));
    REQUIRE(!any2.is_const());
    REQUIRE(any2.is_volatile());
    REQUIRE(!any2.is_const_volatile());
    REQUIRE_THROWS_AS(any_cast<const double>(any2), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<double>(any2), std::bad_any_cast);
    REQUIRE((&any_cast<volatile double>(any2) == &value));
    REQUIRE((&any_cast<const volatile double>(any2) == &value));
    REQUIRE(any_cast<double>(&any2) == nullptr);
    REQUIRE(any_cast<const double>(&any2) == nullptr);
    REQUIRE((any_cast<volatile double>(&any2) == &value));
    REQUIRE((any_cast<const volatile double>(&any2) == &value));
    auto any3 = AnyRef();
    any3 = any2;
    REQUIRE(any3.get_type() == typeid(double));
    REQUIRE(!any3.is_const());
    REQUIRE(any3.is_volatile());
    REQUIRE(!any3.is_const_volatile());
    REQUIRE_THROWS_AS(any_cast<const double>(any3), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<double>(any3), std::bad_any_cast);
    REQUIRE((&any_cast<volatile double>(any3) == &value));
    REQUIRE((&any_cast<const volatile double>(any3) == &value));
    REQUIRE(any_cast<double>(&any3) == nullptr);
    REQUIRE(any_cast<const double>(&any3) == nullptr);
    REQUIRE((any_cast<volatile double>(&any3) == &value));
    REQUIRE((any_cast<const volatile double>(&any3) == &value));
  }

  TEST_CASE("move_construtor_and_assigment") {
    const auto value = false;
    auto any1 = AnyRef(value);
    auto any2 = std::move(any1);
    REQUIRE(any2.get_type() == typeid(bool));
    REQUIRE(any2.is_const());
    REQUIRE(!any2.is_volatile());
    REQUIRE(!any2.is_const_volatile());
    REQUIRE(&any_cast<const bool>(any2) == &value);
    REQUIRE_THROWS_AS(any_cast<bool>(any2), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<volatile bool>(any2), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<const volatile bool>(any2), std::bad_any_cast);
    REQUIRE(any_cast<bool>(&any2) == nullptr);
    REQUIRE(any_cast<const bool>(&any2) == &value);
    REQUIRE((any_cast<volatile bool>(&any2) == nullptr));
    REQUIRE((any_cast<const volatile bool>(&any2) == nullptr));
    auto any3 = AnyRef();
    any3 = std::move(any2);
    REQUIRE(any3.get_type() == typeid(bool));
    REQUIRE(any3.is_const());
    REQUIRE(!any3.is_volatile());
    REQUIRE(!any3.is_const_volatile());
    REQUIRE(&any_cast<const bool>(any3) == &value);
    REQUIRE_THROWS_AS(any_cast<bool>(any3), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<volatile bool>(any3), std::bad_any_cast);
    REQUIRE_THROWS_AS(any_cast<const volatile bool>(any3), std::bad_any_cast);
    REQUIRE(any_cast<bool>(&any3) == nullptr);
    REQUIRE(any_cast<const bool>(&any3) == &value);
    REQUIRE((any_cast<volatile bool>(&any3) == nullptr));
    REQUIRE((any_cast<const volatile bool>(&any3) == nullptr));
  }

  TEST_CASE("any_ref_from_any") {
    auto value = std::any(123);
    auto ref = AnyRef(value);
    REQUIRE(ref.get_type() == typeid(int));
    REQUIRE(any_cast<int>(ref) == 123);
  }
}

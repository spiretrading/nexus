#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <doctest/doctest.h>
#include "Spire/Spire/AnyRef.hpp"

using namespace Spire;

namespace {
  class Indestructible {
    public:
      static Indestructible& get() {
        static auto value = Indestructible();
        return value;
      }
      Indestructible(const Indestructible&) = delete;
      Indestructible& operator =(const Indestructible&) = delete;

    private:
      Indestructible() noexcept = default;
      ~Indestructible() = default;
  };

  struct MoveCounter {
    int* m_count;

    explicit MoveCounter(int& count) noexcept
      : m_count(&count) {}
    MoveCounter(const MoveCounter&) noexcept = default;
    MoveCounter(MoveCounter&& value) noexcept
        : m_count(value.m_count) {
      ++*m_count;
    }
  };

  struct Addressable {
    auto operator &(this auto& self) {
      return static_cast<decltype(std::addressof(self))>(nullptr);
    }
  };
}

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

  TEST_CASE("owned_copy_assignment") {
    auto value = std::make_shared<int>(123);
    auto destination = AnyRef(value, AnyRef::by_value);
    REQUIRE(value.use_count() == 2);
    SUBCASE("empty") {
      auto source = AnyRef();
      destination = source;
      REQUIRE(!destination.has_value());
    }
    SUBCASE("owned") {
      auto source = AnyRef(456);
      SUBCASE("mutable") {
        destination = source;
      }
      SUBCASE("const") {
        destination = std::as_const(source);
      }
      REQUIRE(any_cast<const int>(destination) == 456);
      REQUIRE(any_cast<const int>(&destination) != any_cast<const int>(&source));
    }
    REQUIRE(value.use_count() == 1);
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

  TEST_CASE("owned_self_move_assignment") {
    auto value = std::make_shared<int>(123);
    auto any = AnyRef(value, AnyRef::by_value);
    auto& source = any;
    REQUIRE(value.use_count() == 2);
    any = std::move(source);
    REQUIRE(value.use_count() == 2);
  }

  TEST_CASE("volatile_value_operations") {
    volatile auto value = 123;
    auto any = AnyRef(value);
    SUBCASE("assign") {
      REQUIRE_THROWS_AS(any.assign(std::any(456)), std::bad_any_cast);
      REQUIRE(value == 123);
    }
    SUBCASE("to_any") {
      REQUIRE_THROWS_AS(to_any(any), std::bad_any_cast);
    }
    SUBCASE("const_to_any") {
      auto constant = AnyRef(std::as_const(value));
      REQUIRE_THROWS_AS(to_any(constant), std::bad_any_cast);
    }
  }

  TEST_CASE("rvalue_constructor_moves") {
    auto count = 0;
    auto value = MoveCounter(count);
    auto any = AnyRef();
    SUBCASE("mutable") {
      any = AnyRef(std::move(value));
      REQUIRE(count == 1);
    }
    SUBCASE("const") {
      any = AnyRef(std::move(std::as_const(value)));
      REQUIRE(count == 0);
    }
    REQUIRE(any.get_type() == typeid(MoveCounter));
    REQUIRE(any.is_const());
    REQUIRE(any_cast<const MoveCounter>(any).m_count == &count);
  }

  TEST_CASE("const_rvalue_constructor") {
    const auto source = AnyRef(123);
    auto copy = AnyRef(std::move(source));
    REQUIRE(copy.get_type() == typeid(int));
    REQUIRE(any_cast<const int>(copy) == 123);
    REQUIRE(any_cast<const int>(&copy) != any_cast<const int>(&source));
    REQUIRE(any_cast<const int>(source) == 123);
  }

  TEST_CASE("owned_any") {
    auto value = std::make_shared<int>(123);
    auto any = AnyRef();
    SUBCASE("temporary") {
      any = AnyRef(std::any(value));
    }
    SUBCASE("rvalue") {
      auto source = std::any(value);
      any = AnyRef(std::move(source));
    }
    SUBCASE("const_rvalue") {
      const auto source = std::any(value);
      any = AnyRef(std::move(source));
    }
    SUBCASE("by_value") {
      auto source = std::any(value);
      any = AnyRef(source, AnyRef::by_value);
    }
    REQUIRE(any.get_type() == typeid(std::shared_ptr<int>));
    REQUIRE(any.is_const());
    REQUIRE(any_cast<const std::shared_ptr<int>>(any) == value);
    REQUIRE(std::any_cast<std::shared_ptr<int>>(to_any(any)) == value);
    REQUIRE(value.use_count() == 2);
    {
      auto copy = any;
      REQUIRE(copy.has_value());
      REQUIRE(any_cast<const std::shared_ptr<int>>(copy) == value);
      REQUIRE(any_cast<const std::shared_ptr<int>>(&copy) !=
        any_cast<const std::shared_ptr<int>>(&any));
      REQUIRE(value.use_count() == 3);
    }
    REQUIRE(value.use_count() == 2);
    any = AnyRef();
    REQUIRE(value.use_count() == 1);
  }

  TEST_CASE("empty_any") {
    auto value = std::any();
    auto any = AnyRef();
    SUBCASE("mutable") {
      any = AnyRef(value);
    }
    SUBCASE("const") {
      any = AnyRef(std::as_const(value));
    }
    REQUIRE(!any.has_value());
    REQUIRE(any.get_type() == typeid(void));
    REQUIRE(!to_any(any).has_value());
    value = 123;
    REQUIRE(any.has_value());
    REQUIRE(any_cast<const int>(any) == 123);
    value.reset();
    REQUIRE(!any.has_value());
    REQUIRE(any.get_type() == typeid(void));
    REQUIRE(!to_any(any).has_value());
  }

  TEST_CASE("owned_empty_any") {
    auto any = AnyRef(std::any());
    REQUIRE(!any.has_value());
    REQUIRE(any.get_type() == typeid(void));
    REQUIRE(!to_any(any).has_value());
    auto copy = any;
    REQUIRE(!copy.has_value());
  }

  TEST_CASE("null_pointer_cast") {
    auto any = static_cast<AnyRef*>(nullptr);
    auto constant = static_cast<const AnyRef*>(nullptr);
    REQUIRE(!any_cast<int>(any));
    REQUIRE(!any_cast<AnyRef>(any));
    REQUIRE(!any_cast<int>(constant));
    REQUIRE(!any_cast<AnyRef>(constant));
  }

  TEST_CASE("non_assignable_ref") {
    using Value = std::pair<const int, int>;
    auto value = Value(123, 456);
    auto any = AnyRef(value);
    REQUIRE(any_cast<Value>(&any) == std::addressof(value));
    REQUIRE(std::any_cast<Value>(to_any(any)) == value);
    REQUIRE_THROWS_AS(any.assign(std::any(value)), std::bad_any_cast);
    REQUIRE(value == Value(123, 456));
  }

  TEST_CASE("move_only_ref") {
    auto value = std::make_unique<int>(123);
    auto any = AnyRef(value);
    auto copy = any;
    REQUIRE(any_cast<std::unique_ptr<int>>(&any) == std::addressof(value));
    REQUIRE(any_cast<std::unique_ptr<int>>(&copy) == std::addressof(value));
    REQUIRE_THROWS_AS(to_any(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any.assign(std::any(456)), std::bad_any_cast);
    REQUIRE(*value == 123);
  }

  TEST_CASE("private_destructor_ref") {
    auto& value = Indestructible::get();
    auto any = AnyRef(value);
    auto copy = any;
    REQUIRE(any_cast<Indestructible>(&any) == std::addressof(value));
    REQUIRE(any_cast<Indestructible>(&copy) == std::addressof(value));
    REQUIRE_THROWS_AS(to_any(any), std::bad_any_cast);
    REQUIRE_THROWS_AS(any.assign(std::any(123)), std::bad_any_cast);
  }

  TEST_CASE("array_ref") {
    auto& value = "abc";
    auto any = AnyRef(value);
    auto copy = any;
    REQUIRE(any.get_type() == typeid(char[4]));
    REQUIRE(any.is_const());
    REQUIRE(!any.is_volatile());
    REQUIRE(any_cast<const char[4]>(&any) == std::addressof(value));
    REQUIRE(any_cast<const char[4]>(&copy) == std::addressof(value));
    REQUIRE_THROWS_AS(to_any(any), std::bad_any_cast);
  }

  TEST_CASE("overloaded_address_ref") {
    auto value = Addressable();
    SUBCASE("mutable") {
      auto any = AnyRef(value);
      REQUIRE(any_cast<Addressable>(&any) == std::addressof(value));
    }
    SUBCASE("const") {
      auto& reference = std::as_const(value);
      auto any = AnyRef(reference);
      REQUIRE(any_cast<const Addressable>(&any) == std::addressof(reference));
    }
    SUBCASE("volatile") {
      auto& reference = static_cast<volatile Addressable&>(value);
      auto any = AnyRef(reference);
      REQUIRE((any_cast<volatile Addressable>(&any) ==
        std::addressof(reference)));
    }
    SUBCASE("const_volatile") {
      auto& reference = static_cast<const volatile Addressable&>(value);
      auto any = AnyRef(reference);
      REQUIRE((any_cast<const volatile Addressable>(&any) ==
        std::addressof(reference)));
    }
  }

  TEST_CASE("any_ref_from_any") {
    auto value = std::any(123);
    auto ref = AnyRef(value);
    REQUIRE(ref.get_type() == typeid(int));
    REQUIRE(any_cast<int>(ref) == 123);
    REQUIRE(any_cast<int>(&ref) == std::any_cast<int>(&value));
    REQUIRE(!any_cast<double>(&ref));
    REQUIRE(!any_cast<volatile int>(&ref));
    auto constant = AnyRef(std::as_const(value));
    REQUIRE(!any_cast<int>(&constant));
    REQUIRE(any_cast<const int>(&constant) == std::any_cast<int>(&value));
    REQUIRE(!any_cast<volatile int>(&constant));
    value.reset();
    REQUIRE(!any_cast<int>(&ref));
    REQUIRE(!any_cast<const int>(&constant));
  }
}

#ifndef NEXUS_DEFINITIONS_ASSET_HPP
#define NEXUS_DEFINITIONS_ASSET_HPP
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <Beam/Serialization/Receiver.hpp>
#include <Beam/Serialization/Sender.hpp>

namespace Nexus {

  /** Encodes an asset type string. */
  constexpr std::uint16_t encode_asset_type(std::string_view code) {
    auto result = std::uint16_t(0);
    result |= static_cast<std::uint16_t>((code[0] - 'A') & 0x1F) << 11;
    result |= static_cast<std::uint16_t>((code[1] - 'A') & 0x1F) << 6;
    result |= static_cast<std::uint16_t>((code[2] - 'A') & 0x1F) << 1;
    return result;
  }

  /** Returns the asset type as a string. */
  constexpr std::string decode_asset_type(std::uint16_t code) {
    auto result = std::string(3, 'A');
    result[0] = static_cast<char>('A' + ((code >> 11) & 0x1F));
    result[1] = static_cast<char>('A' + ((code >> 6) & 0x1F));
    result[2] = static_cast<char>('A' + ((code >> 1) & 0x1F));
    return result;
  }

  /** Identifies an asset. */
  class Asset {
    public:

      /** Represents a currency asset type. */
      static constexpr std::uint16_t CURRENCY = encode_asset_type("CCY");

      /** Represents a currency asset type. */
      static constexpr std::uint16_t CCY = CURRENCY;

      /** Represents an equity asset type. */
      static constexpr std::uint16_t EQUITY = encode_asset_type("EQY");

      /** Represents an equity asset type. */
      static constexpr std::uint16_t EQY = EQUITY;

      /** Constructs a default Asset. */
      constexpr Asset() noexcept;

      /** Constructs an Asset from a value. */
      explicit constexpr Asset(std::uint64_t value) noexcept;

      /** Constructs an Asset from a type and id. */
      constexpr Asset(std::uint16_t type, std::uint64_t id) noexcept;

      /** Constructs an Asset from a type and id. */
      constexpr Asset(std::string_view type, std::uint64_t id) noexcept;

      /** Returns the asset type. */
      constexpr std::uint16_t get_type() const;

      /** Returns the asset id. */
      constexpr std::uint64_t get_id() const;

      /** Tests if this Asset is not equal to zero. */
      explicit constexpr operator bool() const;

      bool operator ==(const Asset&) const = default;

    private:
      std::uint64_t m_value;
  };

  constexpr Asset::Asset() noexcept
    : m_value(0) {}

  constexpr Asset::Asset(std::uint64_t value) noexcept
    : m_value(static_cast<std::uint64_t>(value)) {}

  constexpr Asset::Asset(std::uint16_t type, std::uint64_t id) noexcept
    : m_value((static_cast<std::uint64_t>(type) << 48) |
        (id & 0x0000FFFFFFFFFFFFULL)) {}

  constexpr Asset::Asset(std::string_view type, std::uint64_t id) noexcept
    : Asset(encode_asset_type(type), id) {}

  constexpr Asset::operator bool() const {
    return m_value != 0;
  }

  constexpr std::uint16_t Asset::get_type() const {
    return static_cast<std::uint16_t>(m_value >> 48);
  }

  constexpr std::uint64_t Asset::get_id() const {
    return m_value;
  }
}

namespace Beam {
  template<>
  constexpr auto is_structure<Nexus::Asset> = false;

  template<>
  struct Send<Nexus::Asset> {
    template<IsSender S>
    void operator ()(S& sender, const char* name, Nexus::Asset value) const {
      sender.send(name, value.get_id());
    }
  };

  template<>
  struct Receive<Nexus::Asset> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, Nexus::Asset& value) const {
      value = Nexus::Asset(receive<std::uint64_t>(receiver, name));
    }
  };
}

namespace std {
  template<>
  struct hash<Nexus::Asset> {
    std::size_t operator ()(Nexus::Asset value) const noexcept {
      return std::hash<std::uint64_t>()(value.get_id());
    }
  };
}

#endif

#include "OasisOrderExecutionServer/IirocLegalEntityIdentifier.hpp"
#include <array>
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>

using namespace Beam;
using namespace CryptoPP;
using namespace Nexus;

namespace {
  const auto BROKER_NUMBER_TAG = 6774;
  const auto ORDER_ORIGINATION_TAG = 1724;
  const auto LEI_TAG = 8027;

  std::string base_64_decode(const std::string& source) {
    auto base_64_decoder = Base64Decoder();
    base_64_decoder.Put(
      reinterpret_cast<const byte*>(source.data()), source.size());
    base_64_decoder.MessageEnd();
    auto output = std::string();
    output.resize(static_cast<int>(base_64_decoder.MaxRetrievable()));
    base_64_decoder.Get(reinterpret_cast<byte*>(output.data()), output.size());
    return output;
  }

  std::string base_64_encode(const std::string& source) {
    auto encoder = Base64Encoder(nullptr, false);
    encoder.Put(reinterpret_cast<const byte*>(source.data()), source.size());
    encoder.MessageEnd();
    auto output = std::string();
    output.resize(static_cast<int>(encoder.MaxRetrievable()));
    encoder.Get(reinterpret_cast<byte*>(output.data()), output.size());
    return output;
  }
}

IirocLegalEntityIdentifier::IirocLegalEntityIdentifier(
    const FIX::Dictionary& config) {
  if(config.has("BrokerNumber")) {
    m_broker_number = config.getString("BrokerNumber");
  }
  if(config.has("DealerID") && config.has("OrderOrigination") &&
      config.has("LEIKey") && config.has("CustomerLEI")) {
    static const auto IV_SIZE = 16;
    auto dealer_id = config.getString("DealerID");
    m_order_origination = config.getString("OrderOrigination");
    auto lei_key = base_64_decode(config.getString("LEIKey"));
    auto customer_lei = config.getString("CustomerLEI");
    auto random_pool = AutoSeededRandomPool();
    auto iv = std::array<byte, IV_SIZE>();
    random_pool.GenerateBlock(iv.data(), iv.size());
    auto key = SecByteBlock(lei_key.size());
    std::memcpy(key.BytePtr(), lei_key.data(), key.SizeInBytes());
    auto encryption = CTR_Mode<AES>::Encryption(key, key.size(), iv.data());
    auto encryptor = StreamTransformationFilter(encryption);
    for(auto c : customer_lei) {
      encryptor.Put(static_cast<byte>(c));
    }
    encryptor.MessageEnd();
    auto cipher = std::string(static_cast<int>(encryptor.MaxRetrievable()), 0);
    encryptor.Get(reinterpret_cast<byte*>(cipher.data()), cipher.size());
    auto message = dealer_id;
    message.append(reinterpret_cast<const char*>(iv.data()), iv.size());
    message.append(cipher);
    m_lei = base_64_encode(message);
  }
}

void IirocLegalEntityIdentifier::populate(
    Out<FIX42::NewOrderSingle> new_order_single) const {
  if(m_broker_number) {
    new_order_single->setField(BROKER_NUMBER_TAG, *m_broker_number);
  }
  if(m_order_origination) {
    new_order_single->setField(ORDER_ORIGINATION_TAG, *m_order_origination);
  }
  if(m_lei) {
    new_order_single->setField(LEI_TAG, *m_lei);
  }
}

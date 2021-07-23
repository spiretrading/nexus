#include "OasisOrderExecutionServer/IirocLegalEntityIdentifier.hpp"
#include <array>
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/modes.h>

using namespace Beam;
using namespace CryptoPP;
using namespace Nexus;
using namespace Nexus::OasisOrderExecutionService;

namespace {
  const auto BROKER_NUMBER_TAG = 6774;
  const auto ORDER_ORIGINATION_TAG = 1724;
  const auto LEI_TAG = 8027;

  std::string base64Decode(const std::string& source) {
    auto base64Decoder = Base64Decoder();
    base64Decoder.Put(
      reinterpret_cast<const byte*>(source.data()), source.size());
    base64Decoder.MessageEnd();
    auto output = std::string();
    output.resize(static_cast<int>(base64Decoder.MaxRetrievable()));
    base64Decoder.Get(reinterpret_cast<byte*>(output.data()), output.size());
    return output;
  }

  std::string base64Encode(const std::string& source) {
    auto encoder = Base64Encoder();
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
  if(config.has("BrokerNumber") && config.has("OrderOrigination") &&
      config.has("LEIKey") && config.has("CustomerLEI")) {
    m_brokerNumber = config.getString("BrokerNumber");
    m_orderOrigination = config.getString("OrderOrigination");
    auto brokerNumber = *m_brokerNumber;
    while(brokerNumber.size() < 3) {
      brokerNumber = '0' + brokerNumber;
    }
    auto leiKey = config.getString("LEIKey");
    auto customerLei = config.getString("CustomerLEI");
    auto hexIV = std::array<byte, 16>{
      0xF3, 0x05, 0x16, 0xA4, 0x4E, 0x8D, 0x54, 0x72,
      0xAE, 0xB1, 0x06, 0xB9, 0xF3, 0x56, 0x03, 0x3F};
    auto key = SecByteBlock(leiKey.size());
    std::memcpy(key.BytePtr(), leiKey.data(), key.SizeInBytes());
    auto encryption = CTR_Mode<AES>::Encryption(key, key.size(), hexIV.data());
    auto encryptor = StreamTransformationFilter(encryption);
    for(auto c : customerLei) {
      encryptor.Put(static_cast<byte>(c));
    }
    encryptor.MessageEnd();
    auto cipher = std::string(static_cast<int>(encryptor.MaxRetrievable()), 0);
    encryptor.Get(reinterpret_cast<byte*>(cipher.data()), cipher.size());
    auto message = brokerNumber;
    message.append(reinterpret_cast<const char*>(hexIV.data()), 16);
    message.append(cipher);
    m_lei = base64Encode(message);
  }
}

void IirocLegalEntityIdentifier::populate(
    Out<FIX42::NewOrderSingle> newOrderSingle) const {
  if(m_brokerNumber) {
    newOrderSingle->setField(BROKER_NUMBER_TAG, *m_brokerNumber);
  }
  if(m_orderOrigination) {
    newOrderSingle->setField(ORDER_ORIGINATION_TAG, *m_orderOrigination);
  }
  if(m_lei) {
    newOrderSingle->setField(LEI_TAG, *m_lei);
  }
}

// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TINK_UTIL_TEST_UTIL_H_
#define TINK_UTIL_TEST_UTIL_H_

#include <string>

#include "cc/aead.h"
#include "cc/mac.h"
#include "cc/util/status.h"
#include "cc/util/statusor.h"
#include "google/protobuf/stubs/stringpiece.h"
#include "proto/tink.pb.h"

using google::protobuf::StringPiece;

namespace cloud {
namespace crypto {
namespace tink {
namespace test {

// Various utilities for testing.
///////////////////////////////////////////////////////////////////////////////

// Converts a hexadecimal string into a string of bytes.
// Returns a status if the size of the input is odd or if the input contains
// characters that are not hexadecimal.
util::StatusOr<std::string> HexDecode(google::protobuf::StringPiece hex);

// Converts a hexadecimal string into a string of bytes.
// Dies if the input is not a valid hexadecimal string.
std::string HexDecodeOrDie(google::protobuf::StringPiece hex) {
  return HexDecode(hex).ValueOrDie();
}

// Converts a string of bytes into a hexadecimal string.
std::string HexEncode(google::protobuf::StringPiece bytes);

// Adds the given 'key' with specified parameters and output_prefix_type=TINK
// to the specified 'keyset'.
void AddTinkKey(
    const std::string& key_type,
    uint32_t key_id,
    const google::protobuf::Message& key,
    google::cloud::crypto::tink::KeyStatusType key_status,
    google::cloud::crypto::tink::KeyData::KeyMaterialType material_type,
    google::cloud::crypto::tink::Keyset* keyset);

// Adds the given 'key' with specified parameters and output_prefix_type=LEGACY
// to the specified 'keyset'.
void AddLegacyKey(
    const std::string& key_type,
    uint32_t key_id,
    const google::protobuf::Message& key,
    google::cloud::crypto::tink::KeyStatusType key_status,
    google::cloud::crypto::tink::KeyData::KeyMaterialType material_type,
    google::cloud::crypto::tink::Keyset* keyset);

// Adds the given 'key' with specified parameters and output_prefix_type=RAW
// to the specified 'keyset'.
void AddRawKey(
    const std::string& key_type,
    uint32_t key_id,
    const google::protobuf::Message& key,
    google::cloud::crypto::tink::KeyStatusType key_status,
    google::cloud::crypto::tink::KeyData::KeyMaterialType material_type,
    google::cloud::crypto::tink::Keyset* keyset);

// A dummy implementation of Aead-interface.
// An instance of DummyAead can be identified by a name specified
// as a parameter of the constructor.
class DummyAead : public Aead {
 public:
  DummyAead(const std::string& aead_name) : aead_name_(aead_name) {}

  // Computes a dummy ciphertext, which is concatenation of provided 'plaintext'
  // with the name of this DummyAead.
  util::StatusOr<std::string> Encrypt(
      const StringPiece& plaintext,
      const StringPiece& additional_data) const override {
    return plaintext.ToString().append(aead_name_);
  }

  util::StatusOr<std::string> Decrypt(
      const StringPiece& ciphertext,
      const StringPiece& additional_data) const override {
    std::string c = ciphertext.ToString();
    size_t pos = c.rfind(aead_name_);
    if (pos != std::string::npos &&
        ciphertext.length() == (unsigned)(aead_name_.length() + pos)) {
      return c.substr(0, pos);
    }
    return util::Status(util::error::INVALID_ARGUMENT, "Wrong ciphertext.");
  }

 private:
  std::string aead_name_;
};

// A dummy implementation of Mac-interface.
// An instance of DummyMac can be identified by a name specified
// as a parameter of the constructor.
class DummyMac : public Mac {
 public:
  DummyMac(const std::string mac_name) : mac_name_(mac_name) {}

  // Computes a dummy MAC, which is concatenation of provided 'data'
  // with the name of this DummyMac.
  util::StatusOr<std::string> ComputeMac(
      google::protobuf::StringPiece data) const override {
    return data.ToString().append(mac_name_);
  }

  util::Status VerifyMac(
      google::protobuf::StringPiece mac,
      google::protobuf::StringPiece data) const override {
    if (mac == (data.ToString().append(mac_name_))) {
      return util::Status::OK;
    } else {
      return util::Status(util::error::INVALID_ARGUMENT, "Wrong MAC.");
    }
  }
 private:
  std::string mac_name_;
};


}  // namespace test
}  // namespace tink
}  // namespace crypto
}  // namespace cloud

#endif  // TINK_UTIL_TEST_UTIL_H_

// Candid serialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include <variant>

#include "candid_assert.h"
#include "candid_debug_config.h"
#include "candid_decoder.h"
#include "candid_encoder.h"
#include "icpp_hooks.h"
#include "candid_serialize.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

// Default constructor handles nullary input '()'
CandidSerialize::CandidSerialize() { serialize(); }
CandidSerialize::CandidSerialize(const CandidType &a) {
  m_A.append(a);
  serialize();
}
CandidSerialize::CandidSerialize(const CandidArgs &A) : m_A{A} { serialize(); }

CandidSerialize::~CandidSerialize() {}

void CandidSerialize::serialize() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results
  // B(kv* : <datatype>*) =
  // i8('D') i8('I') i8('D') i8('L')      magic number
  // T*(<comptype>*)                      type definition table
  // I*(<datatype>*)                      types of the argument list
  // M(kv* : <datatype>*)                 values of argument list

  m_B.clear();

  // The magic number, the type table and the argument types: built once,
  // from the types of the arguments, by the encoder
  CandidTypeGraph g;
  std::vector<int> types;
  for (const auto &p_c : m_A.m_args_ptrs) {
    if (!p_c->has_M())
      ICPP_HOOKS::trap("ERROR: cannot serialize an argument without a value: "
                       "a CandidTypeValue made with only a type, or for "
                       "decoding, holds no value that fits its type");
    types.push_back(p_c->graph_type(g));
  }
  std::vector<uint8_t> header;
  std::string err;
  if (!CandidEncoder::encode_header(g, types, header, err))
    ICPP_HOOKS::trap("ERROR: cannot serialize the argument types: " + err);
  for (uint8_t b : header)
    m_B.append_byte(std::byte(b));

  // -------------------------------------------------------------------------------------
  // M(kv* : <datatype>*)                 values of argument list
  for (std::shared_ptr<CandidTypeRoot> p_c : m_A.m_args_ptrs) {
    VecBytes M = p_c->get_M();
    for (std::byte b : M.vec()) {
      m_B.append_byte(b);
    }
  }

  if constexpr (CANDID_SERIALIZE_SELF_CHECK) {
    const std::vector<uint8_t> &bytes = m_B.vec_uint8_t();
    std::vector<CandidValue> values;
    if (!CandidDecoder::decode(bytes.data(), bytes.size(), g, types, values,
                               err))
      ICPP_HOOKS::trap("ERROR: CandidSerialize wrote a message that does not "
                       "decode at its own types: " +
                       err + "\n" + m_B.as_hex_string());
  }

  // Append R
  // Was never implemented in Candid, although it is still in the spec
  // https://www.joachim-breitner.de/blog/786-A_Candid_explainer__Quirks
}

// Assert the serialized candid VecBytes against a string in "hex" format (didc encode)
int CandidSerialize::assert_candid(const std::string &candid_expected,
                                   const bool &assert_value) {
  return CandidAssert::assert_candid(m_B, candid_expected, assert_value);
}

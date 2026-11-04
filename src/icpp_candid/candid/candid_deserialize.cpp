// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include "candid_debug_config.h"
#include "candid_constants.h"
#include "candid_deserialize.h"
#include "candid_decoder.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "icpp_hooks.h"

CandidDeserialize::CandidDeserialize() { deserialize(); }
CandidDeserialize::CandidDeserialize(const VecBytes &B, CandidArgs A) {
  m_A = A;
  m_B = B;
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    m_hex_string_wire = m_B.as_hex_string();
  }
  deserialize();
}
CandidDeserialize::CandidDeserialize(const std::string hex_string,
                                     CandidArgs A) {
  m_A = A;
  m_hex_string_wire = hex_string;
  m_B.store_hex_string(hex_string);
  deserialize();
}
CandidDeserialize::~CandidDeserialize() {}

void CandidDeserialize::deserialize() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#deserialisation
  // The expected types are those of the CandidType objects in m_A. The
  // message is decoded at those types by CandidDecoder, which follows the
  // spec: subtyping, coercion and the opt rules included. Each object then
  // fills its bound C++ variables with its value.
  CandidTypeGraph g;
  std::vector<int> types;
  types.reserve(m_A.m_args_ptrs.size());
  for (auto &p : m_A.m_args_ptrs)
    types.push_back(p->graph_type(g));

  CandidDecodeLimits limits;
  limits.max_depth = MAX_NESTING_DEPTH;
  limits.max_zero_width_values = size_t(MAX_ZERO_WIDTH_VALUES);

  const std::vector<std::byte> &bytes = m_B.vec();
  std::vector<CandidValue> values;
  std::string err;
  if (!CandidDecoder::decode(reinterpret_cast<const uint8_t *>(bytes.data()),
                             bytes.size(), g, types, values, err, limits)) {
    ICPP_HOOKS::trap("ERROR: the Candid message does not decode at the "
                     "expected types: " +
                     err);
  }
  for (size_t i = 0; i < types.size(); ++i)
    m_A.m_args_ptrs[i]->bind(values[i]);
}

// Assert candid VecBytes against a string in "hex" format (didc encode)
int CandidDeserialize::assert_candid(const std::string &candid_expected,
                                     const bool &assert_value) {
  return CandidAssert::assert_candid(m_B, candid_expected, assert_value);
}

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
  for (auto &p : m_A.m_args_ptrs) types.push_back(p->graph_type(g));

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
  for (size_t i = 0; i < types.size(); ++i) m_A.m_args_ptrs[i]->bind(values[i]);
}


// Assert candid VecBytes against a string in "hex" format (didc encode)
int CandidDeserialize::assert_candid(const std::string &candid_expected,
                                     const bool &assert_value) {
  return CandidAssert::assert_candid(m_B, candid_expected, assert_value);
}

void CandidDeserialize::enter_nested() {
  if (++m_nesting_depth > MAX_NESTING_DEPTH) {
    ICPP_HOOKS::trap("ERROR: Candid message nested deeper than " +
                     std::to_string(MAX_NESTING_DEPTH) + " levels");
  }
}

void CandidDeserialize::charge_zero_width_values(__uint128_t n) {
  require_zero_width_values(n);
  m_zero_width_values += n;
}

void CandidDeserialize::require_zero_width_values(__uint128_t n) {
  if (n > MAX_ZERO_WIDTH_VALUES - m_zero_width_values) {
    ICPP_HOOKS::trap("ERROR: Candid message with more than " +
                     ICPP_HOOKS::to_string_128(MAX_ZERO_WIDTH_VALUES) +
                     " values that take no bytes on the wire");
  }
}

bool CandidDeserialize::is_zero_width(int datatype_wire) {
  CandidOpcode candidOpcode;
  if (datatype_wire < 0) {
    return datatype_wire == candidOpcode.Null ||
           datatype_wire == candidOpcode.Reserved;
  }
  CandidTypeTable &type_table = typetable_wire(datatype_wire);
  if (m_zero_width_cache.size() != m_typetables_wire.size()) {
    m_zero_width_cache.assign(m_typetables_wire.size(), -1);
  }
  int8_t &cached = m_zero_width_cache[size_t(datatype_wire)];
  if (cached >= 0) return cached == 1;
  // Each nested record type is a level of recursion, bounded as decoding is
  CandidNestingGuard nesting_guard(*this);
  // Not zero width while it is checked, so a record that contains itself ends
  cached = 0;
  bool zero_width = type_table.get_opcode() == candidOpcode.Record;
  if (zero_width) {
    for (int field_datatype : type_table.get_p_wire()->get_field_datatypes()) {
      if (!is_zero_width(field_datatype)) {
        zero_width = false;
        break;
      }
    }
  }
  cached = zero_width ? 1 : 0;
  return zero_width;
}

CandidTypeTable &CandidDeserialize::typetable_wire(int datatype) {
  if (datatype < 0 || size_t(datatype) >= m_typetables_wire.size()) {
    ICPP_HOOKS::trap("ERROR: Candid type index " + std::to_string(datatype) +
                     " is not in the type table section, which has " +
                     std::to_string(m_typetables_wire.size()) + " entries");
  }
  return m_typetables_wire[size_t(datatype)];
}

int CandidDeserialize::get_opcode_from_datatype_on_wire(int datatype) {
  int opcode = datatype;

  if (opcode >= 0) {
    // A non-negative opcode is an index in the type tables
    // -> try at most num_typetables_wire times to get a negative opcode
    int tries = 0;
    while (opcode >= 0 && tries < m_typetables_wire.size()) {
      if (opcode < m_typetables_wire.size()) {
        opcode = m_typetables_wire[opcode].get_opcode();
      }
      tries++;
    }

    if (opcode >= 0) {
      ICPP_HOOKS::trap(
          "ERROR: Cannot find a negative Opcode by walking the type tables for type table index " +
          std::to_string(datatype));
    }
  }

  return opcode;
}
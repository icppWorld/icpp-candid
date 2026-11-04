// The class for the Primitive Candid Type: float64

#include "candid_type.h"
#include "candid_type_float64.h"
#include "icpp_hooks.h"
#include "candid_assert.h"
#include "candid_opcode.h"

CandidTypeFloat64::CandidTypeFloat64() {

  double v = 0.0;
  initialize(v);
}

// This constructor allows for setting the value during Deserialization
CandidTypeFloat64::CandidTypeFloat64(double *p_v) {

  set_pv(p_v);

  const double v = const_cast<double &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeFloat64::CandidTypeFloat64(const double v) { initialize(v); }

CandidTypeFloat64::~CandidTypeFloat64() {}

// Initialize things
void CandidTypeFloat64::initialize(const double &v) {
  if (sizeof(v) != 8) {
    ICPP_HOOKS::trap(
        "ERROR: The type double is not 8 bytes. \nThis is not yet supported.\n" +
        std::string(__func__));
  }
  m_v = v;
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeFloat64::set_pv(double *v) { m_pv = v; }

void CandidTypeFloat64::set_datatype() {
  m_datatype_opcode = CandidOpcode().Float64;
  m_datatype_hex = OpcodeHex().Float64;
  m_datatype_textual = OpcodeTextual().Float64;
}

void CandidTypeFloat64::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeFloat64::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(z : float<N>) = f<N>(z)
  m_M.append_float_ieee754(m_v);
}

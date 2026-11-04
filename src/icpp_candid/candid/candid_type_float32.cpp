// The class for the Primitive Candid Type: float32

#include "candid_type.h"
#include "candid_type_float32.h"
#include "icpp_hooks.h"
#include "candid_assert.h"

#include "candid_opcode.h"

CandidTypeFloat32::CandidTypeFloat32() {

  float v = 0.0;
  initialize(v);
}

// This constructor allows for setting the value during Deserialization
CandidTypeFloat32::CandidTypeFloat32(float *p_v) {

  set_pv(p_v);

  const float v = const_cast<float &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeFloat32::CandidTypeFloat32(const float v) { initialize(v); }

CandidTypeFloat32::~CandidTypeFloat32() {}

// Initialize things
void CandidTypeFloat32::initialize(const float &v) {
  if (sizeof(v) != 4) {
    ICPP_HOOKS::trap(
        "ERROR: The type float is not 4 bytes. \nThis is not yet supported.\n" +
        std::string(__func__));
  }
  m_v = v;
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeFloat32::set_pv(float *v) { m_pv = v; }

void CandidTypeFloat32::set_datatype() {
  m_datatype_opcode = CandidOpcode().Float32;
  m_datatype_hex = OpcodeHex().Float32;
  m_datatype_textual = OpcodeTextual().Float32;
}

void CandidTypeFloat32::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeFloat32::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(z : float<N>) = f<N>(z)
  m_M.append_float_ieee754(m_v);
}

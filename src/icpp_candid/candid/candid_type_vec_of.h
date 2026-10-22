// The class for the Candid Type: vec of any element type
//
// A vec whose elements are records (with opt, record, variant & vec fields),
// variants with payloads, or vecs of those.
//
// Encode: pass the element type and the elements. Each element must have the
//         same type as element_type:
//   CandidTypeVecOf(element_type, {element_1, element_2, ...})
//
// Decode: pass a function that returns a fresh element, with pointers to where
//         its values go, and a function called after each element is decoded:
//   T item;
//   std::vector<T> items;
//   CandidTypeVecOf([&]() -> CandidType { item = {}; return type_of(item); },
//                   [&]() { items.push_back(item); });
//   The pointers must stay valid until the decode is done.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "candid_type_record.h"
#include "vec_bytes.h"

class CandidTypeVecOf : public CandidTypeBase<CandidTypeVecOf> {
public:
  // Encode
  CandidTypeVecOf(CandidType element_type, std::vector<CandidType> elements);

  // Decode
  CandidTypeVecOf(std::function<CandidType()> make_element,
                  std::function<void()> element_done);

  // Destructor
  ~CandidTypeVecOf();

  bool decode_M(CandidDeserialize &de, VecBytes &B, __uint128_t &offset,
                std::string &parse_error);

  // Binds this decoder to the vec's type table on the wire
  bool bind_wire(CandidDeserialize &de, int datatype_wire);

protected:
  void set_datatype();
  void set_content_type();
  void encode_T();
  void encode_I();
  void encode_M();
  void initialize();

  // The element type, and the elements to encode
  std::shared_ptr<CandidTypeRoot> m_p_element_type{nullptr};
  std::vector<std::shared_ptr<CandidTypeRoot>> m_elements;

  // For decoding
  std::function<CandidType()> m_make_element;
  std::function<void()> m_element_done;
  int m_content_datatype_wire{CANDID_UNDEF};
};

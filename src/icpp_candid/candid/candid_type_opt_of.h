// The class for the Candid Type: opt of any type
//
// An opt of a record, variant, vec, func or opt, which can also be null: a
// null still carries its content type in the type table.
//
// Encode: pass the content type, and the value if there is one. The value
//         must have the same type as content_type:
//   CandidTypeOptOf(content_type)          // null
//   CandidTypeOptOf(content_type, value)   // opt value
//
// Decode: pass a function that returns the content, with pointers to where
//         its values go, and where to store whether there was a value:
//   T item;
//   bool has_value;
//   CandidTypeOptOf([&]() -> CandidType { return type_of(item); }, &has_value);
//   The pointers must stay valid until the decode is done.

#pragma once

#include <functional>
#include <memory>

#include "candid_type_record.h"
#include "vec_bytes.h"

class CandidTypeOptOf : public CandidTypeBase<CandidTypeOptOf> {
public:
  // Decoding, with CandidDecoder: see CandidTypeRoot
  int graph_type(CandidTypeGraph &g);
  void bind(const CandidValue &v);

  // Encode
  CandidTypeOptOf(CandidType content_type);
  CandidTypeOptOf(CandidType content_type, CandidType value);

  // Decode
  CandidTypeOptOf(std::function<CandidType()> make_content, bool *has_value);

  // Destructor
  ~CandidTypeOptOf();

protected:
  void set_datatype();
  void set_content_type();
  void encode_I();
  void encode_M();
  void initialize();

  // The content type, and the value to encode, if any
  std::shared_ptr<CandidTypeRoot> m_p_content_type{nullptr};
  std::shared_ptr<CandidTypeRoot> m_p_value{nullptr};

  // For decoding
  std::function<CandidType()> m_make_content;
  bool *m_p_has_value{nullptr};
};

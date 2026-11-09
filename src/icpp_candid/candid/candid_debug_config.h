// Debug settings for the Candid Library

#pragma once

constexpr bool CANDID_DESERIALIZE_DEBUG_PRINT = false;
// CandidSerialize decodes every message it builds, at the types it wrote, and
// traps if that fails: a type and a value that do not agree surface at once,
// where they are made. On in native builds (debugging, the Mock IC, fuzzing),
// off in a canister, where it would cost instructions on every reply.
#if defined(__wasm32__)
constexpr bool CANDID_SERIALIZE_SELF_CHECK = false;
#else
constexpr bool CANDID_SERIALIZE_SELF_CHECK = true;
#endif

/*
 * Copyright (C) 2019 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#ifndef KRAKENBRIDGE_BINDINGS_QJS_ATOM_STRING_H_
#define KRAKENBRIDGE_BINDINGS_QJS_ATOM_STRING_H_

#include <quickjs/quickjs.h>
#include <memory>
#include "foundation/macros.h"
#include "foundation/native_string.h"
#include "native_string_utils.h"
#include "qjs_engine_patch.h"

namespace kraken {

// An AtomicString instance represents a string, and multiple AtomicString
// instances can share their string storage if the strings are
// identical. Comparing two AtomicString instances is much faster than comparing
// two String instances because we just check string storage identity.
class AtomicString {
 public:
  static AtomicString Empty(JSContext* ctx) { return AtomicString(ctx, JS_ATOM_NULL); };
  static AtomicString From(JSContext* ctx, NativeString* native_string) {
    JSValue str = JS_NewUnicodeString(ctx, native_string->string(), native_string->length());
    auto result = AtomicString(ctx, str);
    JS_FreeValue(ctx, str);
    return result;
  };

  AtomicString() = default;
  AtomicString(JSContext* ctx, JSAtom atom) : ctx_(ctx), atom_(atom){};
  AtomicString(JSContext* ctx, const std::string& string) : ctx_(ctx), atom_(JS_NewAtom(ctx, string.c_str())){};
  AtomicString(JSContext* ctx, JSValue value) : ctx_(ctx), atom_(JS_ValueToAtom(ctx, value)){};
  ~AtomicString() { JS_FreeAtom(ctx_, atom_); };

  // Return the undefined string value from atom key.
  JSValue ToQuickJS(JSContext* ctx) const { return JS_AtomToValue(ctx, atom_); };

  [[nodiscard]] std::string ToStdString() const {
    const char* buf = JS_AtomToCString(ctx_, atom_);
    std::string result = std::string(buf);
    JS_FreeCString(ctx_, buf);
    return result;
  }

  [[nodiscard]] std::unique_ptr<NativeString> ToNativeString() const {
    JSValue stringValue = JS_AtomToValue(ctx_, atom_);
    uint32_t length;
    uint16_t* bytes = JS_ToUnicode(ctx_, stringValue, &length);
    JS_FreeValue(ctx_, stringValue);
    return std::make_unique<NativeString>(bytes, length);
  }

  // Copy assignment
  AtomicString(AtomicString const& value) {
    if (&value != this) {
      atom_ = JS_DupAtom(ctx_, value.atom_);
    }
    ctx_ = value.ctx_;
  };
  AtomicString& operator=(const AtomicString& other) {
    if (&other != this) {
      atom_ = JS_DupAtom(ctx_, other.atom_);
    }
    return *this;
  };

  // Move assignment
  AtomicString(AtomicString&& value) noexcept {
    if (&value != this) {
      atom_ = JS_DupAtom(ctx_, value.atom_);
    }
    ctx_ = value.ctx_;
  };
  AtomicString& operator=(AtomicString&& value) noexcept {
    if (&value != this) {
      atom_ = JS_DupAtom(ctx_, value.atom_);
    }
    ctx_ = value.ctx_;
    return *this;
  }

  bool operator==(const AtomicString& other) const { return other.atom_ == this->atom_; }
  bool operator!=(const AtomicString& other) const { return other.atom_ != this->atom_; };

 protected:
  JSContext* ctx_{nullptr};
  JSAtom atom_{JS_ATOM_NULL};
};

}  // namespace kraken

#endif  // KRAKENBRIDGE_BINDINGS_QJS_ATOM_STRING_H_

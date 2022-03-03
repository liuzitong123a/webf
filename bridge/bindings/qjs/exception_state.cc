/*
 * Copyright (C) 2021 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#include "exception_state.h"

namespace kraken {

void ExceptionState::ThrowException(JSContext* ctx, ErrorType type, const char* message) {
  switch (type) {
    case ErrorType::TypeError:
      exception_ = JS_ThrowTypeError(ctx, "%s", message);
      break;
    case InternalError:
      exception_ = JS_ThrowInternalError(ctx, "%s", message);
      break;
    case RangeError:
      exception_ = JS_ThrowRangeError(ctx, "%s", message);
      break;
    case ReferenceError:
      exception_ = JS_ThrowReferenceError(ctx, "%s", message);
      break;
    case SyntaxError:
      exception_ = JS_ThrowSyntaxError(ctx, "%s", message);
      break;
  }
}

void ExceptionState::ThrowException(JSContext* ctx, JSValue exception) {
  exception_ = JS_DupValue(ctx, exception);
}

bool ExceptionState::HasException() {
  return !JS_IsNull(exception_);
}

JSValue ExceptionState::ToQuickJS() {
  return exception_;
}

}  // namespace kraken

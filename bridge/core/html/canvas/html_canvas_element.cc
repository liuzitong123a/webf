/*
 * Copyright (C) 2019-2022 The Kraken authors. All rights reserved.
 * Copyright (C) 2022-present The WebF authors. All rights reserved.
 */
#include "html_canvas_element.h"
#include "binding_call_methods.h"
#include "foundation/native_value_converter.h"
#include "html_names.h"
#include "canvas_types.h"
#include "canvas_rendering_context_2d.h"

namespace webf {

HTMLCanvasElement::HTMLCanvasElement(Document& document) : HTMLElement(html_names::kcanvas, &document) {}

CanvasRenderingContext* HTMLCanvasElement::getContext(const AtomicString& type, ExceptionState& exception_state) const {
  NativeValue value = InvokeBindingMethod(binding_call_methods::kgetContext, 0, nullptr, exception_state);
  NativeBindingObject* native_binding_object =
      NativeValueConverter<NativeTypePointer<NativeBindingObject>>::FromNativeValue(value);

  if (type == canvas_types::k2d) {
    return MakeGarbageCollected<CanvasRenderingContext2D>(GetExecutingContext(), native_binding_object);
  }

  return nullptr;
}

}  // namespace webf

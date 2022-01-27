/*
 * Copyright (C) 2021 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#ifndef KRAKENBRIDGE_HTML_PARSER_H
#define KRAKENBRIDGE_HTML_PARSER_H

#include "bindings/qjs/dom/element.h"
#include "executing_context.h"
#include "foundation/native_string.h"
#include "third_party/gumbo-parser/src/gumbo.h"

namespace kraken {

class HTMLParser {
 public:
  static bool parseHTML(const char* code, size_t codeLength, Node* rootNode);
  static bool parseHTML(std::string html, Node* rootNode);

 private:
  ExecutionContext* m_context;
  static void traverseHTML(Node* root, GumboNode* node);
  static void parseProperty(Element* element, GumboElement* gumboElement);
};
}  // namespace kraken

#endif  // KRAKENBRIDGE_HTML_PARSER_H

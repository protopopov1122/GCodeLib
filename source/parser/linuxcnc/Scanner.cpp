/*
  SPDX short identifier: MIT
  Copyright 2019 Jevgēnijs Protopopovs
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include "gcodelib/parser/linuxcnc/Scanner.h"
#include "gcodelib/parser/Error.h"
#include <iostream>
#include <string>
#include <regex>
#include <map>

namespace GCodeLib::Parser::LinuxCNC {

  static const std::map<std::string, GCodeKeyword> GCodeKeywords = {
    { "MOD", GCodeKeyword::Mod },
    { "EQ", GCodeKeyword::Eq },
    { "NE", GCodeKeyword::Ne },
    { "GE", GCodeKeyword::Ge },
    { "GT", GCodeKeyword::Gt },
    { "LE", GCodeKeyword::Le },
    { "LT", GCodeKeyword::Lt },
    { "AND", GCodeKeyword::And },
    { "OR", GCodeKeyword::Or },
    { "XOR", GCodeKeyword::Xor },
    { "sub", GCodeKeyword::Sub },
    { "endsub", GCodeKeyword::Endsub },
    { "return", GCodeKeyword::Return },
    { "call", GCodeKeyword::Call },
    { "if", GCodeKeyword::If },
    { "elseif", GCodeKeyword::Elseif },
    { "else", GCodeKeyword::Else },
    { "endif", GCodeKeyword::Endif },
    { "while", GCodeKeyword::While },
    { "endwhile", GCodeKeyword::Endwhile },
    { "do", GCodeKeyword::Do },
    { "repeat", GCodeKeyword::Repeat },
    { "endrepeat", GCodeKeyword::Endrepeat },
    { "break", GCodeKeyword::Break },
    { "continue", GCodeKeyword::Continue }
  };

  static std::regex Whitespaces(R"(^[\s]+)");
  static std::regex Integer(R"(^[0-9]+)");
  static std::regex Float(R"(^[0-9]+\.[0-9]+)");
  static std::regex Literal(R"(^[a-zA-Z_]{2,}[\w_]*)");
  static std::regex Operator(R"(^[A-Z\+\-*/%\[\]#\=<>])", std::regex_constants::ECMAScript | std::regex_constants::icase);
  static std::regex Comment(R"(^;.*$)");
  static std::regex BracedComment(R"(^\([^\)]*\))");

  static bool match_regex(const std::string &string, std::smatch &match, const std::regex &regex) {
    return std::regex_search(string, match, regex) && !match.empty();
  }

  GCodeDefaultScanner::GCodeDefaultScanner(std::istream &is, const std::string &tag)
    : input(is), buffer(""), source_position(tag, 0, 0, 0) {}

  std::optional<GCodeToken> GCodeDefaultScanner::next() {
    if (this->finished()) {
      return std::optional<GCodeToken>();
    }
    this->skipWhitespaces();

    if (this->buffer.empty()) {
      this->next_line();
      return GCodeToken(this->source_position);
    }
    std::smatch match;
    std::optional<GCodeToken> token;
    if (match_regex(this->buffer, match, Float)) {
      token = GCodeToken(std::stod(match.str()), this->source_position);
    } else if (match_regex(this->buffer, match, Integer)) {
      token = GCodeToken(static_cast<int64_t>(std::stoll(match.str())), this->source_position); 
    } else if (match_regex(this->buffer, match, Literal)) {
      if (GCodeKeywords.count(match.str()) != 0) {
        token = GCodeToken(GCodeKeywords.at(match.str()), this->source_position);
      } else {
        token = GCodeToken(match.str(), true, this->source_position);
      }
    } else if (match_regex(this->buffer, match, Operator)) {
      char chr = std::toupper(match.str()[0]);
      token = GCodeToken(static_cast<GCodeOperator>(chr), this->source_position);
    } else if (match_regex(this->buffer, match, Comment) || match_regex(this->buffer, match, BracedComment)) {
      token = GCodeToken(match.str(), false, this->source_position);
    }

    if (!match.empty()) {
      this->shift(match.length());
    } else {
      this->shift(1);
      throw GCodeParseException("Unknown symbol at \'" + this->buffer + '\'', this->source_position);
    }
    return token;
  }

  bool GCodeDefaultScanner::finished() {
    return this->buffer.empty() && !this->input.good();
  }

  void GCodeDefaultScanner::next_line() {
    this->buffer.clear();
    if (this->input.good()) {
      std::getline(this->input, this->buffer);
      this->source_position.update(this->source_position.getLine() + 1, 1, 0);
    }
  }

  void GCodeDefaultScanner::shift(std::size_t len) {
    if (len > this->buffer.length()) {
      len = this->buffer.length();
    }
    uint8_t checksum = this->source_position.getChecksum();
    for (std::size_t i = 0; i < len; i++) {
      checksum ^= this->buffer[i];
    }
    checksum &= 0xff;
    this->buffer.erase(0, len);
    this->source_position.update(this->source_position.getLine(), this->source_position.getColumn() + len, checksum);
  }

  void GCodeDefaultScanner::skipWhitespaces() {
    std::smatch match;
    if (match_regex(this->buffer, match, Whitespaces)) {
      this->shift(match.length());
    }
  }
}
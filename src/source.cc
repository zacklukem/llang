#include "source.hh"
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <iostream>
#include <ostream>
#include <string>

using namespace llang;
class NullBuffer : public std::streambuf {
public:
  int overflow(int c) { return c; }
};

// cant be const unfortunately
static auto dummy_buffer = NullBuffer();
static auto dummy_stream = std::ostream(&dummy_buffer);

bool Source::debugPrintMessages() const {
  if (messages.empty())
    return false;
  for (auto message : messages) {
    auto location = getLocation(message->span.start);
    auto line = getLine(location.first);
    auto trimmed = boost::trim_left_copy(line);
    switch (message->type) {
    case ERROR:
      std::cout << "\033[1;31merror: \033[0m";
      break;
    case NOTE:
      std::cout << "\033[1;34mnote: \033[0m";
      break;
    case WARNING:
      std::cout << "\033[1;33mwarning: \033[0m";
      break;
    }
    std::cout << (path.empty() ? "dummy/path/to/file.ext" : path) << ":" << location.first << ":"
              << location.second << "\n";
    std::cout << trimmed << "\n";
    std::cout << std::string(location.second - 1 - (line.size() - trimmed.size()), ' ');
    for (int i = 0; i < message->span.length; i++) std::cout << '^';
    std::cout << "  " << message->message.str() << "\n";
  }
  return true;
}

std::size_t Source::lineCount() const {
  std::size_t count = 1;
  for (auto c : value) {
    if (c == '\n')
      count++;
  }
  return count;
}

std::string Source::getLine(std::size_t index) const {
  std::size_t count = 1;
  std::string buffer;
  for (auto c : value) {
    if (c == '\n') {
      if (count == index)
        return buffer;
      buffer.clear();
      count++;
    } else {
      buffer += c;
    }
  }
  return buffer;
}

std::pair<std::size_t, std::size_t> Source::getLocation(std::size_t index) const {
  std::size_t line_number = 1;
  std::size_t char_number = 1;
  for (const auto& c : value | boost::adaptors::indexed()) {
    if (c.index() == index) {
      return std::pair(line_number, char_number);
    }
    char_number++;
    if (c.value() == '\n') {
      line_number++;
      char_number = 1;
    }
  }
  throw std::runtime_error("Unexpected end of file");
}

std::string Span::getValue() const { return source->value.substr(start, length); }

Span Span::operator+(const Span& other) const {
  if (this->source != other.source) {
    throw std::runtime_error("Spans must be from the same source to join");
  }

  const Span& first = other.start < this->start ? other : *this;
  const Span& second = other.start < this->start ? *this : other;

  return Span(source, first.start, first.length + second.length);
}

Span Span::subspan(int sub_start, int end) const {
  if (end < 0) {
    return subspan(sub_start, length + end);
  }
  return Span(source, start + sub_start, end - sub_start);
}

std::ostream& Span::expect(bool assertion, MessageType type) {
  if (!assertion) {
    return fail(type);
  }
  return dummy_stream;
}

std::ostream& Span::fail(MessageType type) {
  auto message = std::make_shared<Message>(*this, type);
  source->messages.push_back(message);
  return message->message;
}

std::size_t Span::end() const { return start + length == 0 ? 0 : (length - 1); }

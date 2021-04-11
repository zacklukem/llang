#pragma once

#include <cstdlib>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
namespace llang {

struct Message;
enum MessageType { NOTE, WARNING, ERROR };

class Source {
public:
  Source(std::string value) : value(value) {}
  Source(std::string value, std::string path) : value(value), path(path) {}
  std::size_t lineCount() const;
  std::string getLine(std::size_t index) const;
  std::pair<std::size_t, std::size_t> getLocation(std::size_t index) const;
  bool debugPrintMessages() const;

public:
  std::string value;
  std::string path;
  std::vector<std::shared_ptr<Message>> messages;
};

class Span {
public:
  Span(std::shared_ptr<Source> source) : start(0), length(0), source(source) {}
  Span(std::shared_ptr<Source> source, std::size_t start, std::size_t length)
      : start(start), length(length), source(source) {}
  std::string getValue() const;
  inline std::string str() const { return getValue(); };
  std::ostream& expect(bool assertion, MessageType type = MessageType::ERROR);
  std::ostream& fail(MessageType type = MessageType::ERROR);
  Span operator+(const Span& other) const;
  Span subspan(int start, int end) const;
  std::size_t end() const;

public:
  std::size_t start, length;

private:
  std::shared_ptr<Source> source;
};

struct Message {
  Message(Span span, MessageType type) : type(type), span(span) {}
  MessageType type;
  Span span;
  std::stringstream message;
};

} // namespace llang

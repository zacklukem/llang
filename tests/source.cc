#include <gtest/gtest.h>
#include <memory>
#include <source.hh>
#include <string>

namespace parser = llang;

TEST(Span, SpanGetValue) {
  std::string str = "span with stuff";
  auto source = std::make_shared<parser::Source>(str);
  auto span = parser::Span(source, 0, str.length());
  EXPECT_EQ(span.getValue(), str);
  EXPECT_FALSE(source->debugPrintMessages());
}

TEST(Span, SpanSubspan) {
  std::string str = "span with stuff";
  auto source = std::make_shared<parser::Source>(str);
  auto span = parser::Span(source, 0, str.length());
  auto sub = span.subspan(2, 6);

  EXPECT_EQ(sub.getValue(), "an w");
  auto sub_b = span.subspan(2, -2);
  EXPECT_EQ(sub_b.getValue(), "an with stu");
  EXPECT_FALSE(source->debugPrintMessages());
}

TEST(Span, SpanJoin) {
  std::string str_a = "first span";
  std::string str_b = "second span";
  auto source = std::make_shared<parser::Source>(str_a + str_b);
  auto a = parser::Span(source, 0, str_a.length());
  auto b = parser::Span(source, str_a.length(), str_b.length());
  auto joined = a + b;
  EXPECT_EQ(joined.length, str_a.length() + str_b.length());
  EXPECT_EQ(joined.start, 0);
  EXPECT_FALSE(source->debugPrintMessages());
}

TEST(Source, LineCount) {
  auto source = std::make_shared<parser::Source>("first line\nsecond line\nthird line");
  ASSERT_EQ(source->lineCount(), 3);
}

TEST(Source, GetLine) {
  auto source = std::make_shared<parser::Source>("first line\nsecond line\nthird line");
  ASSERT_EQ(source->getLine(1), "first line");
  ASSERT_EQ(source->getLine(2), "second line");
  ASSERT_EQ(source->getLine(3), "third line");
}

TEST(Source, GetLocation) {
  auto source = std::make_shared<parser::Source>("first line\nsecond line\nthird line");
  ASSERT_EQ(source->getLocation(4),
            std::pair(static_cast<std::size_t>(1), static_cast<std::size_t>(5)));
  ASSERT_EQ(source->getLocation(13),
            std::pair(static_cast<std::size_t>(2), static_cast<std::size_t>(3)));
  ASSERT_EQ(source->getLocation(27),
            std::pair(static_cast<std::size_t>(3), static_cast<std::size_t>(5)));
}

#include <sstream>
#include <iostream>
#include <fstream>
#include <pika/reversed_stream.hpp>
#include <gtest/gtest.h>

std::string random_fill() {
    std::string temp;
    auto size = 1000 + rand() % 10000;
    for (auto i = 0; i < size; ++i) {
        temp.push_back('a' + (rand() % ('z' - 'a')));
    }
    return temp;
}

TEST(RStream, StringStream) {
    auto b = random_fill();
    std::istringstream stream(b);
    pika::reversed_stream::RStream<std::istringstream> test(stream);
    std::string a;
    while (auto res = test.get()) {
        a.push_back(res.value());
    }
    std::reverse(a.begin(), a.end());
    EXPECT_EQ(a, b);
}

TEST(RStream, File) {
    auto b = random_fill();
    {
        std::ofstream test("test.txt");
        test << b;
    }
    std::ifstream fin("test.txt");
    pika::reversed_stream::RStream<std::istream> test(fin);
    std::string a;
    while (auto res = test.get()) {
        a.push_back(res.value());
    }
    std::reverse(a.begin(), a.end());
    EXPECT_EQ(a, b);
}


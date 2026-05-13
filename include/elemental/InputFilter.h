#pragma once
#include <cctype>
#include <functional>
#include <memory>
#include <regex>
#include <string>

namespace gui {
    // Predicate that controls which characters a text input accepts.
    // An empty acceptChar means no filtering (every character is accepted).
    struct InputFilter {
        std::function<bool(char c, const std::string& current, int pos)> acceptChar;

        bool Test(char c, const std::string& current, int pos) const {
            return !acceptChar || acceptChar(c, current, pos);
        }

        // No filtering — every character is accepted.
        static InputFilter Any() { return {}; }

        // Decimal digits 0-9 only.
        static InputFilter Digits() {
            return {[](char c, const std::string&, int) -> bool {
                return std::isdigit((unsigned char)c) != 0;
            }};
        }

        // Signed or unsigned integer: digits and an optional leading '-'.
        static InputFilter Integer(bool allowSign = true) {
            return {[allowSign](char c, const std::string& s, int pos) -> bool {
                if (std::isdigit((unsigned char)c))
                    return true;
                if (allowSign && c == '-' && pos == 0 && s.find('-') == std::string::npos)
                    return true;
                return false;
            }};
        }

        // Floating-point number: digits, an optional leading '-', and one decimal point.
        static InputFilter Numeric(bool allowDecimal = true, bool allowSign = true) {
            return {[allowDecimal, allowSign](char c, const std::string& s, int pos) -> bool {
                if (std::isdigit((unsigned char)c))
                    return true;
                if (allowSign && c == '-' && pos == 0 && s.find('-') == std::string::npos)
                    return true;
                if (allowDecimal && c == '.' && s.find('.') == std::string::npos)
                    return true;
                return false;
            }};
        }

        // ASCII letters a-z and A-Z.
        static InputFilter Alpha() {
            return {[](char c, const std::string&, int) -> bool {
                return std::isalpha((unsigned char)c) != 0;
            }};
        }

        // ASCII letters and digits.
        static InputFilter Alphanumeric() {
            return {[](char c, const std::string&, int) -> bool {
                return std::isalnum((unsigned char)c) != 0;
            }};
        }

        // Each typed character must match the given regular expression.
        static InputFilter Pattern(const std::string& regexStr) {
            auto re = std::make_shared<std::regex>(regexStr);
            return {[re](char c, const std::string&, int) -> bool {
                return std::regex_match(std::string(1, c), *re);
            }};
        }

        // Arbitrary predicate: fn(char, current text, insertion position).
        static InputFilter Custom(std::function<bool(char, const std::string&, int)> fn) {
            return {std::move(fn)};
        }
    };
} // namespace gui

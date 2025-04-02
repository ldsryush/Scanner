#include "scanner.h"
#include <sstream>
#include <cctype>
#include <iostream>

using namespace std;

Token::Token(TokenType type, string value, int lineNumber)
    : type(type), value(value), lineNumber(lineNumber) {}

TokenType Token::getTokenType() const {
    return type;
}

string Token::getTokenValue() const {
    return value;
}

string Token::toString() const {
    return "(" + tokenTypeToString(type) + ",'" + value + "'," + to_string(lineNumber) + ")";
}

string Token::tokenTypeToString(TokenType type) const {
    switch (type) {
        case COMMA: return "COMMA";
        case STRING: return "STRING";
        case COMMENT: return "COMMENT";
        case QUERIES: return "QUERIES";
        case RULES: return "RULES";
        case SCHEMES: return "SCHEMES";
        case FACTS: return "FACTS";
        case COLON: return "COLON";
        case COLON_DASH: return "COLON_DASH";
        case ID: return "ID";
        case LEFT_PAREN: return "LEFT_PAREN";
        case RIGHT_PAREN: return "RIGHT_PAREN";
        case Q_MARK: return "Q_MARK";
        case PERIOD: return "PERIOD";
        case ADD: return "ADD";
        case MULTIPLY: return "MULTIPLY";
        case END: return "END";
        case UNDEFINED: return "UNDEFINED";
        default: return "UNKNOWN";
    }
}

Scanner::Scanner(const string& input)
    : input(input), lineNumber(1) {}

void Scanner::scan() {
    size_t i = 0;

    if (input.empty()) {
        tokens.push_back(Token(END, "", 1));
        return;
    }

    while (i < input.size()) {
        char c = input[i];

        if (isspace(c)) {
            if (c == '\n') lineNumber++;
            i++;
        } else if (c == ',') {
            tokens.push_back(Token(COMMA, ",", lineNumber));
            i++;
        } else if (c == '\'') {
            size_t start = i;
            int startLine = lineNumber;
            i++;
            while (i < input.size() && input[i] != '\'') {
                if (input[i] == '\n') lineNumber++;
                i++;
            }
            if (i < input.size() && input[i] == '\'') {
                i++;
                tokens.push_back(Token(STRING, input.substr(start, i - start), startLine));
            } else {
                tokens.push_back(Token(UNDEFINED, input.substr(start, i - start), startLine));
                cerr << "Warning: Unterminated string starting on line " << startLine << endl;
            }
        } else if (c == '#') {
            while (i < input.size() && input[i] != '\n') {
                i++;
            }
        } else if (isalpha(c)) {
            size_t start = i;
            int startLine = lineNumber;
            while (i < input.size() && (isalnum(input[i]) || input[i] == '_')) {
                i++;
            }
            string value = input.substr(start, i - start);
            TokenType type = (value == "Queries") ? QUERIES :
                             (value == "Rules") ? RULES :
                             (value == "Schemes") ? SCHEMES :
                             (value == "Facts") ? FACTS :
                             ID;
            tokens.push_back(Token(type, value, startLine));
        } else if (c == ':') {
            if (i + 1 < input.size() && input[i + 1] == '-') {
                tokens.push_back(Token(COLON_DASH, ":-", lineNumber));
                i += 2;
            } else {
                tokens.push_back(Token(COLON, ":", lineNumber));
                i++;
            }
        } else if (c == '(') {
            tokens.push_back(Token(LEFT_PAREN, "(", lineNumber));
            i++;
        } else if (c == ')') {
            tokens.push_back(Token(RIGHT_PAREN, ")", lineNumber));
            i++;
        } else if (c == '?') {
            tokens.push_back(Token(Q_MARK, "?", lineNumber));
            i++;
        } else if (c == '.') {
            tokens.push_back(Token(PERIOD, ".", lineNumber));
            i++;
        } else if (c == '+') {
            tokens.push_back(Token(ADD, "+", lineNumber));
            i++;
        } else if (c == '*') {
            tokens.push_back(Token(MULTIPLY, "*", lineNumber));
            i++;
        } else {
            tokens.push_back(Token(UNDEFINED, string(1, c), lineNumber));
            i++;
        }
    }

    tokens.push_back(Token(END, "", lineNumber));
}

const vector<Token>& Scanner::getTokens() const {
    return tokens;
}

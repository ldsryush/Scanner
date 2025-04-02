#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>

enum TokenType {
    COMMA,
    STRING,
    COMMENT,
    QUERIES,
    RULES,
    SCHEMES,
    FACTS,
    COLON,
    COLON_DASH,
    ID,
    LEFT_PAREN,
    RIGHT_PAREN,
    Q_MARK,
    PERIOD,
    ADD,
    MULTIPLY,
    END,
    UNDEFINED
};

class Token {
private:
    TokenType type;
    std::string value;
    int lineNumber;

public:
    Token(TokenType type, std::string value, int lineNumber);

    TokenType getTokenType() const;
    std::string getTokenValue() const;
    std::string toString() const;

private:
    std::string tokenTypeToString(TokenType type) const;
};

class Scanner {
private:
    std::string input;
    std::vector<Token> tokens;
    int lineNumber;

public:
    Scanner(const std::string& input);
    void scan();
    const std::vector<Token>& getTokens() const;
};

#endif

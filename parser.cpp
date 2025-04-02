#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include "scanner.h" 

using namespace std;

class Parameter {
public:
    string value;
    Parameter(string val) : value(val) {}
    string toString() const {
        return value;
    }
};

class Predicate {
public:
    string name;
    vector<Parameter> parameters;
    Predicate(string n) : name(n) {}
    void addParameter(Parameter p) {
        parameters.push_back(p);
    }
    string toString() const {
        stringstream ss;
        ss << name << "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) ss << ",";
            ss << parameters[i].toString();
        }
        ss << ")";
        return ss.str();
    }
};

class Rule {
public:
    Predicate headPredicate;
    vector<Predicate> bodyPredicates;
    Rule(Predicate hp) : headPredicate(hp) {}
    void addBodyPredicate(Predicate p) {
        bodyPredicates.push_back(p);
    }
    string toString() const {
        stringstream ss;
        ss << headPredicate.toString() << " :- ";
        for (size_t i = 0; i < bodyPredicates.size(); ++i) {
            if (i > 0) ss << ",";
            ss << bodyPredicates[i].toString();
        }
        ss << ".";
        return ss.str();
    }
};

class DatalogProgram {
public:
    vector<Predicate> schemes;
    vector<Predicate> facts;
    vector<Rule> rules;
    vector<Predicate> queries;
    set<string> domain;

    void addScheme(Predicate p) {
        schemes.push_back(p);
    }
    void addFact(Predicate p) {
        facts.push_back(p);
        for (const auto& param : p.parameters) {
            domain.insert(param.toString());
        }
    }
    void addRule(Rule r) {
        rules.push_back(r);
    }
    void addQuery(Predicate p) {
        queries.push_back(p);
    }
    string toString() const {
        stringstream ss;
        ss << "Success!\n";
        ss << "Schemes(" << schemes.size() << "):\n";
        for (const auto& scheme : schemes) {
            ss << "  " << scheme.toString() << "\n";
        }
        ss << "Facts(" << facts.size() << "):\n";
        for (const auto& fact : facts) {
            ss << "  " << fact.toString() << ".\n";
        }
        ss << "Rules(" << rules.size() << "):\n";
        for (const auto& rule : rules) {
            ss << "  " << rule.toString() << "\n";
        }
        ss << "Queries(" << queries.size() << "):\n";
        for (const auto& query : queries) {
            ss << "  " << query.toString() << "?\n";
        }
        ss << "Domain(" << domain.size() << "):\n";
        for (const auto& value : domain) {
            ss << "  " << value << "\n";
        }
        return ss.str();
    }
};

class Parser {
public:
    vector<Token> tokens;
    DatalogProgram datalogProgram;
    size_t currentTokenIndex;

    Parser(const vector<Token>& tokens) : tokens(tokens), currentTokenIndex(0) {}

    void skipComments() {
        while (tokens[currentTokenIndex].getTokenType() == COMMENT || 
               tokens[currentTokenIndex].getTokenType() == UNDEFINED) {
            currentTokenIndex++;
        }
    }

    void match(TokenType expectedType) {
        skipComments();
        if (tokens[currentTokenIndex].getTokenType() == expectedType) {
            currentTokenIndex++;
            skipComments();
        } else {
            throw runtime_error(tokens[currentTokenIndex].toString());
        }
    }

    void parse() {
        datalogProgram = DatalogProgram();
        datalogProgramParse();
    }

    void datalogProgramParse() {
        match(SCHEMES);
        match(COLON);
        scheme();
        schemeList();
        match(FACTS);
        match(COLON);
        factList();
        match(RULES);
        match(COLON);
        ruleList();
        match(QUERIES);
        match(COLON);
        query();
        queryList();
        match(END);
    }

    void schemeList() {
        if (tokens[currentTokenIndex].getTokenType() == FACTS) return;
        scheme();
        schemeList();
    }

    void factList() {
        if (tokens[currentTokenIndex].getTokenType() == RULES) return;
        fact();
        factList();
    }

    void ruleList() {
        if (tokens[currentTokenIndex].getTokenType() == QUERIES) return;
        rule();
        ruleList();
    }

    void queryList() {
        if (tokens[currentTokenIndex].getTokenType() == END) return;
        query();
        queryList();
    }

    void scheme() {
        Predicate p(tokens[currentTokenIndex].getTokenValue());
        match(ID);
        match(LEFT_PAREN);
        p.addParameter(Parameter(tokens[currentTokenIndex].getTokenValue()));
        match(ID);
        idList(p);
        match(RIGHT_PAREN);
        datalogProgram.addScheme(p);
    }

    void fact() {
        Predicate p(tokens[currentTokenIndex].getTokenValue());
        match(ID);
        match(LEFT_PAREN);
        p.addParameter(Parameter(tokens[currentTokenIndex].getTokenValue()));
        match(STRING);
        stringList(p);
        match(RIGHT_PAREN);
        match(PERIOD);
        datalogProgram.addFact(p);
    }

    void rule() {
        Predicate head = headPredicate();
        Rule r(head);
        match(COLON_DASH);
        Predicate p = predicate();
        r.addBodyPredicate(p);
        predicateList(r);
        match(PERIOD);
        datalogProgram.addRule(r);
    }

    void query() {
        Predicate p = predicate();
        match(Q_MARK);
        datalogProgram.addQuery(p);
    }

    void idList(Predicate &p) {
        if (tokens[currentTokenIndex].getTokenType() == RIGHT_PAREN) return;
        match(COMMA);
        p.addParameter(Parameter(tokens[currentTokenIndex].getTokenValue()));
        match(ID);
        idList(p);
    }

    void stringList(Predicate &p) {
        if (tokens[currentTokenIndex].getTokenType() == RIGHT_PAREN) return;
        match(COMMA);
        p.addParameter(Parameter(tokens[currentTokenIndex].getTokenValue()));
        match(STRING);
        stringList(p);
    }

    void predicateList(Rule &r) {
        if (tokens[currentTokenIndex].getTokenType() == PERIOD) return;
        match(COMMA);
        Predicate p = predicate();
        r.addBodyPredicate(p);
        predicateList(r);
    }

    Predicate headPredicate() {
        Predicate p(tokens[currentTokenIndex].getTokenValue());
        match(ID);
        match(LEFT_PAREN);
        p.addParameter(Parameter(tokens[currentTokenIndex].getTokenValue()));
        match(ID);
        idList(p);
        match(RIGHT_PAREN);
        return p;
    }

    Predicate predicate() {
        Predicate p(tokens[currentTokenIndex].getTokenValue());
        match(ID);
        match(LEFT_PAREN);
        p.addParameter(Parameter(tokens[currentTokenIndex].getTokenValue()));
        if (tokens[currentTokenIndex].getTokenType() == STRING) {
            match(STRING);
        } else {
            match(ID);
        }
        parameterList(p);
        match(RIGHT_PAREN);
        return p;
    }

    void parameterList(Predicate &p) {
        if (tokens[currentTokenIndex].getTokenType() == RIGHT_PAREN) return;
        match(COMMA);
        if (tokens[currentTokenIndex].getTokenType() == STRING) {
            p.addParameter(Parameter(tokens[currentTokenIndex].getTokenValue()));
            match(STRING);
        } else {
            p.addParameter(Parameter(tokens[currentTokenIndex].getTokenValue()));
            match(ID);
        }
        parameterList(p);
    }
};

//int main(int argc, char* argv[]) {
    //string filename = argv[1];
    //ifstream file(filename);
    //if (!file.is_open()) {
      //  cerr << "Could not open file: " << filename << endl;
        //return 1;
    //}

    //stringstream buffer;
    //buffer << file.rdbuf();
    //string fileContents = buffer.str();

    //Scanner scanner(fileContents);
    //scanner.scan();

    //try {
        //Parser parser(scanner.getTokens());
        //parser.parse();
      //  cout << parser.datalogProgram.toString();
    //} catch (const exception &e) {
      //  cout << "Failure!\n  " << e.what() << "\n";
    //}

  //  return 0;
//}


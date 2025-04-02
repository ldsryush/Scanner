#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "parser.cpp"

using namespace std;

static inline string trimTrailingPeriod(const string& s) {
    if (!s.empty() && s.back() == '.')
        return s.substr(0, s.size()-1);
    return s;
}

class Node {
private:
    set<int> adjacentNodeIDs;
public:
    void addEdge(int adjacentNodeID) {
        adjacentNodeIDs.insert(adjacentNodeID);
    }
    string toString() const {
        stringstream ss;
        bool first = true;
        for (int nodeID : adjacentNodeIDs) {
            if (!first)
                ss << ",";
            first = false;
            ss << "R" << nodeID;
        }
        return ss.str();
    }
};

class Graph {
private:
    map<int, Node> nodes;
public:
    Graph(int size) {
        for (int nodeID = 0; nodeID < size; nodeID++)
            nodes[nodeID] = Node();
    }
    void addEdge(int fromNodeID, int toNodeID) {
        nodes[fromNodeID].addEdge(toNodeID);
    }
    string toString() const {
        ostringstream oss;
        for (const auto &pair : nodes)
            oss << "R" << pair.first << ":" << pair.second.toString() << "\n";
        return oss.str();
    }
};

class Scheme : public vector<string> {
public:
    Scheme() : vector<string>() {}
    Scheme(const vector<string>& attributes) : vector<string>(attributes) {}
};

class Tuple : public vector<string> {
public:
    Tuple() : vector<string>() {}
    Tuple(const vector<string>& values) : vector<string>(values) {}
    bool operator<(const Tuple& other) const {
        return static_cast<vector<string>>(*this) < static_cast<vector<string>>(other);
    }
    string toString(const Scheme& scheme) const {
        stringstream ss;
        for (size_t i = 0; i < scheme.size(); i++) {
            if (i > 0)
                ss << ", ";
            string value = at(i);
            if (!value.empty() && value.front() == '\'' && value.back() == '\'')
                value = value.substr(1, value.size()-2);
            ss << scheme[i] << "='" << value << "'";
        }
        return ss.str();
    }
};

class Relation {
private:
    string name;
    Scheme scheme;
    set<Tuple> tuples;
public:
    Relation() : name(""), scheme() {}
    Relation(string name, Scheme scheme) : name(name), scheme(scheme) {}
    void addTuple(const Tuple& tuple) {
        tuples.insert(tuple);
    }
    Relation select(int index, const string& value) const {
        Relation result(name, scheme);
        for (const Tuple& tuple : tuples)
            if (index >= 0 && index < static_cast<int>(tuple.size()) && tuple[index] == value)
                result.addTuple(tuple);
        return result;
    }
    Relation select(int index1, int index2) const {
        Relation result(name, scheme);
        for (const Tuple& tuple : tuples)
            if (index1 >= 0 && index1 < static_cast<int>(tuple.size()) &&
                index2 >= 0 && index2 < static_cast<int>(tuple.size()) &&
                tuple[index1] == tuple[index2])
                result.addTuple(tuple);
        return result;
    }
    Relation project(const vector<int>& indices) const {
        Scheme newScheme;
        for (int index : indices) {
            if (index >= 0 && index < static_cast<int>(scheme.size()))
                newScheme.push_back(scheme[index]);
            else
                cerr << "Index out of bounds in project(): " << index << endl;
        }
        Relation result(name, newScheme);
        for (const Tuple& tuple : tuples) {
            Tuple newTuple;
            for (int index : indices) {
                if (index >= 0 && index < static_cast<int>(tuple.size()))
                    newTuple.push_back(tuple[index]);
                else
                    cerr << "Index out of bounds in project tuple: " << index << endl;
            }
            result.addTuple(newTuple);
        }
        return result;
    }
    Relation rename(const vector<string>& newAttributes) const {
        Scheme newScheme(newAttributes);
        Relation result(name, newScheme);
        for (const Tuple& tuple : tuples)
            result.addTuple(tuple);
        return result;
    }
    Relation join(const Relation& other) const {
        Scheme newScheme = scheme;
        for (const string& attr : other.scheme)
            if (find(newScheme.begin(), newScheme.end(), attr) == newScheme.end())
                newScheme.push_back(attr);
        Relation result(name, newScheme);
        for (const Tuple& tuple1 : tuples) {
            for (const Tuple& tuple2 : other.tuples) {
                Tuple newTuple = tuple1;
                bool isJoinable = true;
                for (size_t i = 0; i < other.scheme.size(); i++) {
                    auto it = find(scheme.begin(), scheme.end(), other.scheme[i]);
                    if (it != scheme.end()) {
                        size_t index = distance(scheme.begin(), it);
                        if (index >= tuple1.size() || i >= tuple2.size() || tuple1[index] != tuple2[i]) {
                            isJoinable = false;
                            break;
                        }
                    } else {
                        if (i < tuple2.size())
                            newTuple.push_back(tuple2[i]);
                        else {
                            cerr << "Index out of bounds in join: " << i << endl;
                            isJoinable = false;
                        }
                    }
                }
                if (isJoinable)
                    result.addTuple(newTuple);
            }
        }
        return result;
    }
    void unionWith(const Relation& other) {
        for (const Tuple& tuple : other.tuples)
            tuples.insert(tuple);
    }
    string toString() const {
        stringstream ss;
        for (const Tuple& tuple : tuples)
            ss << "  " << tuple.toString(scheme) << "\n";
        return ss.str();
    }
    size_t size() const {
        return tuples.size();
    }
    const Scheme& getScheme() const {
        return scheme;
    }
    set<Tuple>& getTuples() {
        return tuples;
    }
    const set<Tuple>& getTuples() const {
        return tuples;
    }
};

class Database {
private:
    map<string, Relation> relations;
public:
    void addRelation(const string& name, const Relation& relation) {
        relations[name] = relation;
    }
    Relation& getRelation(const string& name) {
        return relations[name];
    }
};

class Interpreter {
private:
    DatalogProgram datalogProgram;
    Database database;
public:
    Interpreter(const DatalogProgram& dp) : datalogProgram(dp) {}
    void evaluateSchemes() {
        for (const auto& scheme : datalogProgram.schemes) {
            vector<string> attributes;
            for (const auto& param : scheme.parameters)
                attributes.push_back(param.value);
            Relation relation(scheme.name, Scheme(attributes));
            database.addRelation(scheme.name, relation);
        }
    }
    void evaluateFacts() {
        for (const auto& fact : datalogProgram.facts) {
            vector<string> values;
            for (const auto& param : fact.parameters)
                values.push_back(param.value);
            Tuple tuple(values);
            database.getRelation(fact.name).addTuple(tuple);
        }
    }
    void evaluateRules() {
        int iterationCount = 0;
        bool databaseChanged = true;
        cout << "Rule Evaluation\n";
        while (databaseChanged) {
            databaseChanged = false;
            ++iterationCount;
            for (const Rule& rule : datalogProgram.rules) {
                Relation result = evaluateRule(rule);
                Relation& existingRelation = database.getRelation(rule.headPredicate.name);
                size_t initialSize = existingRelation.size();
                Relation newTuples = result;
                for (const Tuple& tuple : existingRelation.getTuples())
                    newTuples.getTuples().erase(tuple);
                existingRelation.unionWith(result);
                if (existingRelation.size() > initialSize)
                    databaseChanged = true;
                cout << trimTrailingPeriod(rule.toString()) << "\n";
                cout << newTuples.toString();
            }
        }
        cout << "\nSchemes populated after " << iterationCount << " passes through the Rules.\n";
    }
    Relation evaluateRule(const Rule& rule) {
        Relation result;
        vector<Relation> intermediateResults;
        for (const Predicate& predicate : rule.bodyPredicates)
            intermediateResults.push_back(evaluateQuery(predicate));
        if (intermediateResults.size() > 1) {
            result = intermediateResults[0];
            for (size_t i = 1; i < intermediateResults.size(); i++)
                result = result.join(intermediateResults[i]);
        } else if (intermediateResults.size() == 1) {
            result = intermediateResults[0];
        }
        vector<int> indices;
        vector<string> newAttributes;
        Relation& headRelation = database.getRelation(rule.headPredicate.name);
        const Scheme& targetScheme = headRelation.getScheme();
        if (targetScheme.size() != rule.headPredicate.parameters.size())
            cerr << "Mismatch in number of attributes between rule head and target scheme." << endl;
        for (size_t j = 0; j < rule.headPredicate.parameters.size(); j++) {
            string variable = rule.headPredicate.parameters[j].value;
            bool found = false;
            for (size_t i = 0; i < result.getScheme().size(); i++) {
                if (result.getScheme()[i] == variable) {
                    indices.push_back(i);
                    newAttributes.push_back(targetScheme[j]);
                    found = true;
                    break;
                }
            }
            if (!found)
                cerr << "Attribute not found during evaluateRule(): " << variable << endl;
        }
        result = result.project(indices);
        result = result.rename(newAttributes);
        return result;
    }
    void evaluateQueries() {
        cout << "\nQuery Evaluation\n";
        for (const auto& query : datalogProgram.queries) {
            Relation result = evaluateQuery(query);
            cout << trimTrailingPeriod(query.toString()) << "? ";
            if (result.size() == 0)
                cout << "No\n";
            else
                cout << "Yes(" << result.size() << ")\n" << result.toString();
        }
    }
    Relation evaluateQuery(const Predicate& query) {
        Relation relation = database.getRelation(query.name);
        vector<int> projectIndices;
        vector<string> renameAttributes;
        map<string, int> variableIndices;
        for (size_t i = 0; i < query.parameters.size(); i++) {
            const auto& param = query.parameters[i];
            if (!param.value.empty() && param.value.front() == '\'')
                relation = relation.select(i, param.value);
            else {
                if (variableIndices.find(param.value) != variableIndices.end())
                    relation = relation.select(variableIndices[param.value], i);
                else {
                    variableIndices[param.value] = i;
                    projectIndices.push_back(i);
                    renameAttributes.push_back(param.value);
                }
            }
        }
        relation = relation.project(projectIndices);
        relation = relation.rename(renameAttributes);
        return relation;
    }
    void interpret() {
        evaluateSchemes();
        evaluateFacts();
        evaluateRules();
        evaluateQueries();
    }
    static Graph makeGraph(const vector<Rule>& rules) {
        Graph graph(rules.size());
        for (size_t i = 0; i < rules.size(); i++) {
            for (const Predicate& bodyPred : rules[i].bodyPredicates) {
                for (size_t j = 0; j < rules.size(); j++) {
                    if (bodyPred.name == rules[j].headPredicate.name) {
                        graph.addEdge(i, j);
                    }
                }
            }
        }
        return graph;
    }
};

#endif

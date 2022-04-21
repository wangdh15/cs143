#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  void install_user_classes();
  void check_cycle();



  void abort() {
    cerr << "Compilation halted due to static semantic errors." << endl;
	  exit(1);
  }
  ostream& error_stream;

  Classes classes_;

  std::unordered_map<Symbol, Symbol> graph_;

  template<typename K, typename V>
  class SymTable {
  private:
    std::vector<std::unordered_map<K, V>> table_;
    void fatalError(const std::string& msg) {
      std::cerr << msg << std::endl;
      exit(1);
    }
  public:
    SymTable() = default;

    void enterScope() {
      table_.push_back({});
    }

    void exitScope() {
      if (table_.empty()) fatalError("existScope: Cannot remove scope from an empty table.");
      table_.pop_back();
    }

    void add(const K& k, const V& v) {
      if (table_.empty()) fatalError("add: Cannot add a symbol without a scope.");
      table_.back()[k] = v;
    }

    V lookUp(const K& k) const {
      for (auto iter = table_.rbegin(); iter != table_.rend(); ++iter) {
        if (iter->find(k) != iter->end()) {
          return (*iter)[k];
        }
      }
      return nullptr; // change to optional
    }

     V probe(const K& k) const {
        if (table_.empty()) fatalError("probe: No scope in table.");
        if (table_.back().find(k) != table_.back().end()) {
          return table_.back()[k];
        }
        return nullptr; // change to optional
     }

    void dump() const {
      for (auto iter = table_.rbegin(); iter != table_.rend(); ++iter) {
        std::cerr << "\nScore: \n";
        for (auto& [k, v] : *iter) {
          std::cerr << k << " " << v << std::endl;
        }
      }
    }

  };
  class Enviro {
  private:
    std::unordered_map<Symbol, std::pair<Formals, Symbol>> method_signatures_;
  public:
    Enviro() = default;
  };

public:
  ClassTable(Classes);

  void check_phase1();

  void check_phase2();

  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
};


#endif


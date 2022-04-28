#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <optional>
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

  // collect all method information and put them to env
  void collectAndCheckAllMethod();

  void coolectAndCheckAllMethodSubroution(Symbol cur_class);

  void buildGraphTopDown();


  void abort() {
    cerr << "Compilation halted due to static semantic errors." << endl;
	  exit(1);
  }

  ostream& error_stream;

  Classes classes_;

  std::unordered_map<Symbol, Symbol> graph_;
  std::unordered_map<Symbol, std::vector<Symbol>> graph_rev_;
  std::unordered_map<Symbol, class__class*> symbol_to_class_;

  Enviro env_;

public:
  ClassTable(Classes);

  void check_phase1();

  void check_phase2();

  std::optional<Class_> getClass(Symbol class_name) {
    if (symbol_to_class_.find(class_name) != symbol_to_class_.end()) {
      return symbol_to_class_[class_name];
    }  else {
      return {};
    }
  }

  std::vector<Symbol> getChild(Symbol cur_class) {
    if (graph_rev_.find(cur_class) == graph_rev_.end()) return {};
    else return graph_rev_[cur_class];
  }

  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
};


#endif


#include <assert.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <unordered_map>
#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"

enum Basicness     {Basic, NotBasic};
#define TRUE 1
#define FALSE 0

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;

class CgenNode;
typedef CgenNode *CgenNodeP;

template<typename K, typename V>
class SymbolTab {
private:
   std::vector<std::unordered_map<K, V>> data;

public:
   void enterScope() {
      data.push_back({});
   }

   void exitScope() {
      assert(!data.empty());
      data.pop_back();
   }

   void addPair(const K& k, const V& v) {
      assert(!data.empty());
      assert(data.back().find(k) == data.back().end());
      data.back()[k] = v;
   }

   V lookUp(const K& k) {
      for (auto it = data.rbegin(); it != data.rend(); ++it) {
         if (it->find(k) != it->end()) {
            return (*it)[k];
         }
      }
      std::cerr << "Can not find the symbol! " << k << std::endl;
      exit(1);
   }
};

enum class BASE_LOC_TYPE {
   SELF_,
   FP_
};

struct Loc {
   BASE_LOC_TYPE base_loc;
   int offset;
};

class CgenClassTable : public SymbolTable<Symbol,CgenNode> {
private:
   List<CgenNode> *nds;
   ostream& str;
   int stringclasstag;
   int intclasstag;
   int boolclasstag;

   // for case expression
   std::unordered_map<Symbol, std::pair<int, int>> subclass_idrange;
   std::vector<Symbol> tag_to_class;

   // class_name ->
   // attr_name ->
   // offset of self
   std::unordered_map<Symbol, std::unordered_map<Symbol, int>> class_attr_offset;
   // class_name ->
   // complete_method_name(class_name.method_name) ->
   // offset of disptab
   std::unordered_map<Symbol, std::unordered_map<Symbol, int>> class_method_offset;

   int label_id{0};


   // The following methods emit code for
// constants and global declarations.

   void code_global_data();
   void code_global_text();
   void code_bools(int);
   void code_select_gc();
   void code_constants();
   void set_class_tag();
   void set_class_tag_internal(CgenNodeP cur, int cur_dep);
   void code_class_nameTab();
   void code_class_objTab();
   void code_class_dispTab();
   void gene_dispTab_for_one_class(CgenNodeP cur, std::ostream& str);
   void code_class_protObj();
   std::vector<std::pair<Symbol, Symbol>> collect_all_attr(CgenNodeP cur);

   void code_class_init();

   void code_class_method();

// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

   void install_basic_classes();
   void install_class(CgenNodeP nd);
   void install_classes(Classes cs);
   void build_inheritance_tree();
   void set_relations(CgenNodeP nd);
public:
   CgenClassTable(Classes, ostream& str);
   void code();
   CgenNodeP root();

   int get_method_offset(Symbol class_name, Symbol method_name);


   int get_next_labelid() {
      return label_id++;
   }

   std::pair<int, int> get_subclass_idrang(Symbol class_name) {
      assert(subclass_idrange.find(class_name) != subclass_idrange.end());
      return subclass_idrange[class_name];
   }
private:
   // Map Symbol to Loc
   SymbolTab<Symbol, Loc> symbol_to_loc;
public:

   Loc getSymbolLoc(Symbol name) {
      return symbol_to_loc.lookUp(name);
   }

   SymbolTab<Symbol, Loc>& getSymbolToLoc() {
      return symbol_to_loc;
   }

   void addSymbol(Symbol name, BASE_LOC_TYPE type_, int offset) {
      symbol_to_loc.addPair(name, {type_, offset});
   }

   void enterScope() {
      symbol_to_loc.enterScope();
   }

   void exitScope() {
      symbol_to_loc.exitScope();
   }

private:
   int loc_var_offset = -3;

public:
   int push_new_var() {
      return loc_var_offset--;
   }

   void pop_new_var(int num) {
      loc_var_offset += num;
   }

   void reset_new_var() {
      loc_var_offset = -3;
   }

private:
   CgenNodeP cur_class;

public:
   void setCurClass(CgenNodeP cur_class_) {
      cur_class = cur_class_;
   }

   CgenNodeP getCurClass() {
      return cur_class;
   }

private:
   std::unordered_map<Symbol, int> class_depth;

public:
   int get_depth(Symbol class_name) {
      assert(class_depth.find(class_name) != class_depth.end());
      return class_depth[class_name];
   }
};


class CgenNode : public class__class {
private:
   CgenNodeP parentnd;                        // Parent of class
   List<CgenNode> *children;                  // Children of class
   Basicness basic_status;                    // `Basic' if class is basic
                                              // `NotBasic' otherwise

public:
   CgenNode(Class_ c,
            Basicness bstatus,
            CgenClassTableP class_table);

   void add_child(CgenNodeP child);
   List<CgenNode> *get_children() { return children; }
   void set_parentnd(CgenNodeP p);
   CgenNodeP get_parentnd() { return parentnd; }
   int basic() { return (basic_status == Basic); }
};

class BoolConst
{
 private:
  int val;
 public:
  BoolConst(int);
  void code_def(ostream&, int boolclasstag);
  void code_ref(ostream&) const;
};


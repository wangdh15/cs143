

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <functional>
#include <queue>
#include "cool-tree.h"
#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}

// implement the typeCheck method of cool-tree.h

void class__class::typeCheck(ClassTable& class_table, Enviro& env) {
    // Add newscope and push attr
      env.enterScope();
      env.setCurClass(name);
      // add self to scope
      env.addVar(self, name);
      for (int i = features->first(); features->more(i); i = features->next(i)) {
         if (features->nth(i)->getType() == FeatureType::ATTR) {
            attr_class* cur_attr = dynamic_cast<attr_class*>(features->nth(i));
            env.addVar(cur_attr->getName(), cur_attr->getDeclType());
         }
      }
      // check attr and method
      for (int i = features->first(); features->more(i); i = features->next(i)) {
         features->nth(i)->typeCheck(class_table, env);
      }
      // clear the attr
      env.existScope();
      // the cur_class will be set by other class
}


void method_class::typeCheck(ClassTable& class_table,  Enviro &env) {
        // add new scope
      env.enterScope();
      for (int i = formals->begin(); formals->more(i); i = formals->next(i)) {
          Formal cur_formal = formals->nth(i);
          env.addVar(cur_formal->getName(), cur_formal->getTypeDecl());
      }
      Symbol expr_type = expr->typeCheck(class_table, env);
      if (!checkSubClass(return_type, expr_type)) {
         // TODO type mismatch;
      }
      env.existScope();
}


void attr_class::typeCheck(ClassTable& class_table, Enviro& env) {
    Symbol init_type = init->typeCheck(class_table, env);
    if (!checkSubClass(type_decl, init_type)) {
        class_table->semant_error() << "attr init error with dismatch type: decl type is" << type_decl << " expr type is: " << init_type << std::endl;
    }
}

Symbol assign_class::typeCheck(ClassTable& class_table,  Enviro &env) override {
      Symbol expr_type = expr->typeCheck(class_table, env);
      auto decl_type = env.lookUp(name);
      if (!decl_type) {
          class_table.semant_error() << "Assign Expr: Symbol " << name << " is not defined." << std::endl;
        set_type(Object);  
      } else {
          set_type(decl_type.value());
      }
      return get_type();
}



Symbol bool_const_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    set_type(Bool);
    return Bool;
}


Symbol int_const_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    set_type(Int);
    return Int;
}



Symbol string_const_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    set_type(String);
    return String;
}


Symbol new__class::typeCheck(ClassTable& class_table,  Enviro &env) {
    if (type_name == SELF_TYPE) {
        set_type(env.getCurClass());
    } else {
        set_type(type_name);
    }
    return get_type();
}

static check_formal_with_expr(ClassTable& class_table, Formals formals, Expressions actual) {

}


Symbol dispatch_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    
    Symbol expr_type = expr->typeCheck(class_table, env);
    auto find_result = env.getFuncSig(expr_type, name);
    if (!find_result) {
        class_table.semant_error() << "Dispatch Class: Cannot find method " << expr_type << "::" << name << std::endl;
        set_type(Object);
        return get_type();
    } 

    Formals formals = find_result.value().first();
    Symbol ret_type = find_result.value().second();
    if (actual->len() != formals->len()) {
        class_table.semant_error() << "Dispatch Class: formal len dismatch with args len, formal len:" << formals->len() << \
            ", args len: " << actual->len() << std::endl;
        set_type(Object);
        return get_type();
    } 
    
    std::vector<Symbol> actual_types;
    for (int i = 0; i < actual->len(); ++i) {
        actual_types.push_back(actual->typeCheck(class_table, env));
    } 
    for (int i = 0; i < actual_types.size(); ++i) {
        if (!checkSubClass(actual_types[i], formals->nth(i))) {
            class_table.semant_error() << "Dispatch Class: arg and formal dismatch: arg type: " << actual_types[i] << \
              " formal type: "  << formals->nth(i) << std::endl;
            set_type(Object);
            return get_type();
        }
    }

    


}


ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr), classes_(classes) {
    install_basic_classes();
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");

    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.

    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    //
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object,
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);
    //
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class =
	class_(IO,
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);
    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer.
    //
    Class_ Int_class =
	class_(Int,
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);
    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);
    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //
    Class_ Str_class =
	class_(Str,
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat,
								      single_Formals(formal(arg, Str)),
								      Str,
								      no_expr()))),
			       single_Features(method(substr,
						      append_Formals(single_Formals(formal(arg, Int)),
								     single_Formals(formal(arg2, Int))),
						      Str,
						      no_expr()))),
	       filename);
    graph_[Object] = No_class;
    graph_[IO] = Object;
    graph_[Int] = Object;
    graph_[Bool] = Object;
    graph_[Str] = Object;
    symbol_to_class_[Object] = Object_class;
    symbol_to_class_[IO] = IO_class;
    symbol_to_class_[Int] = Int_class;
    symbol_to_class_[Bool] = Bool_class;
    symbol_to_class_[Str] = Str_class;
}

void ClassTable::install_user_classes() {

    // check redefination of basic class
    for (int i = classes_->first(); classes_->more(i); i = classes_->next(i)) {
        auto cur_class = dynamic_cast<class__class*>(classes_->nth(i));
        Symbol cur_class_name = cur_class->get_name();
        if (cur_class_name == SELF_TYPE ||
            cur_class_name == Object ||
            cur_class_name == IO ||
            cur_class_name == Int ||
            cur_class_name == Bool ||
            cur_class_name == Str) {
                semant_error(classes_->nth(i)) << "Redefinition of basic class " << cur_class_name
                << "." << std::endl;
                abort();
            }
    }

    // check inherits from Str Bool Int SELF_TYPE
    for (int i = classes_->first(); classes_->more(i); i = classes_->next(i)) {
        auto cur_class = dynamic_cast<class__class*>(classes_->nth(i));
        Symbol cur_class_name = cur_class->get_name();
        Symbol cur_class_parent = cur_class->get_parent();
        if (cur_class_parent == SELF_TYPE ||
            cur_class_parent == Str ||
            cur_class_parent == Int ||
            cur_class_parent == Bool) {
            semant_error(classes_->nth(i)) <<  "Class " << cur_class_name <<
                 " cannot inherit class "<< cur_class_parent << "." << std::endl;
            abort();
        }
    }

    // add the user class to graph and check redefine.
    for (int i = classes_->first(); classes_->more(i); i = classes_->next(i)) {
        auto cur_class = dynamic_cast<class__class*>(classes_->nth(i));
        Symbol cur_class_name = cur_class->get_name();
        Symbol cur_class_parent = cur_class->get_parent();
        if (graph_.find(cur_class_name) != graph_.end()) {
            semant_error(classes_->nth(i)) << "Class " << cur_class_name << " was previously defined." << std::endl;
            abort();
        }
        graph_[cur_class_name] = cur_class_parent;
    }

    // check all parent is defined.
    for (int i = classes_->first(); classes_->more(i); i = classes_->next(i)) {
        auto cur_class = dynamic_cast<class__class*>(classes_->nth(i));
        Symbol cur_class_name = cur_class->get_name();
        Symbol cur_class_parent = cur_class->get_parent();
        if (cur_class_name == Object) continue;
        if (graph_.find(cur_class_parent) == graph_.end()) {
            semant_error(classes_->nth(i)) << "Class "
                << cur_class_name << " inherits from an undefined class "
                << cur_class_parent << "." << std::endl;
            abort();
        }
    }

    // check if contain Main class
    bool contain_main = false;
    for (int i = classes_->first(); classes_->more(i); i = classes_->next(i)) {
        auto cur_class = dynamic_cast<class__class*>(classes_->nth(i));
        Symbol cur_class_name = cur_class->get_name();
        if (cur_class_name == Main) {
            contain_main = true;
            break;
        }
    }
    if (!contain_main) {
        semant_error() << "Class Main is not defined." << std::endl;
        abort();
    }

    // add to symbol_to_class_
    for (int i = classes_->first(); classes_->more(i); i = classes_->next(i))  {
        auto cur_class = dynamic_cast<class__class*>(classes_->nth(i));
        symbol_to_class_[cur_class->get_name()] = cur_class;
    }
}


/**
Use typo sort to check cycle.
*/
void ClassTable::check_cycle() {

    // +1 means the parent of Object: No_type
    int all_class_num = graph_.size() + 1;

    std::unordered_map<Symbol, int> degrees;
    for (auto& [k, v] : graph_) {
        degrees[v]++;
    }
    std::queue<Symbol> qe;
    for (auto& [k, v] : graph_) {
        if (degrees.find(k) == degrees.end()) qe.push(k);
    }

    int pop_class_num = 0;
    while (!qe.empty()) {
        auto cur = qe.front();
        qe.pop();
        pop_class_num ++;
        if (cur == No_class) continue;
        degrees[graph_[cur]]--;
        if (degrees[graph_[cur]] == 0) qe.push(graph_[cur]);
    }

    if (pop_class_num != all_class_num) {
        semant_error() << "exist cycle!" << std::endl;
        abort();
    }
}

// check inheritance graph
void ClassTable::check_phase1() {

    if (classes_ == nullptr) {
        return;
    }

    install_user_classes();
    check_cycle();
}

// Build inheritance graph from top to down.
void ClassTable::buildGraphTopDown() {
    for (auto& [k, v] : graph_) {
        if (k == Object) continue;
        graph_rev_[v].push_back(k);
    }
}



// collect all methods and check the correction of override.
// check all attr is not override!
void ClassTable::collectAndCheckAllMethod() {
    SymTable<Symbol, std::pair<std::pair<Formals, Symbol>>> func_table;
    SymTable<Symbol, Symbol> attr_table;

    std::function<void(Symbol)> dfs = [&](Symbol cur_class) {
        func_table.enterScope();
        attr_table.enterScope();
        class__class* t = symbol_to_class_[cur_class];
        Features features = t->get_features();
        for (int i = features->first(); features->more(i); i = features->next(i)) {
            if (cur_feature->getType() == FeatureType::METHOD) {
                // check override method
                method_class* cur_method = dynamic_cast<method_class*>(features->nth(i));
                Symbol method_name = cur_method->getName();
                Formals formals = cur_method->getFormals();
                Symnbol ret_type = cur_method->getRetType();
                // check whether same function def in this class
                auto find_res = func_table.probe(method_name);
                if (!find_res) {
                    // TODO
                    semant_error() << "define same method in this class" << std::endl;
                    abort();
                }
                find_res = func_table.lookUp(method_name);
                if (find_res) {
                    // check arg number, arg type and ret type.
                    Formals old_formals = find_res.value().first;
                    Symbol old_ret_type = find_res.value().second;

                    // check arg number
                    if (old_formals->len() != formals->len()) {
                        // TODO
                        semant_error() << "override with different arg number" << std::endl;
                        abort();
                    }

                    // check arg type
                    for (int j = 0; j < old_formals->len(); ++j) {
                        Formal old_formal = old_formals->nth(j);
                        Formal cur_formal = formals->nth(j);
                        if (!old_formal->equal(cur_formal)) {
                            // TODO
                            semant_error() << "override with different arg type" << std::endl;
                            abort();
                        }
                    }

                    // check ret type
                    if (old_ret_type != ret_type) {
                        semant_error() << "override with different return type" << std::endl;
                        abort();
                    }

                }

                // add this method to funtable
                env_.addFuncSig(t->get_filename(),
                                method_name,
                                formals,
                                ret_type);
                func_table.add(method, {formals, ret_type});
            } else {
                // check attr override
                attr_class* attr = dynamic_cast<attr_class*>(features->nth(j));
                if (attr_table.lookUp(attr->getName())) {
                    semant_error() << "redefined attr!" << std::endl;
                    abort();
                }
                attr_table.add(attr->getName(), attr->getType());
            }
        }

        // check children
        for (auto child : graph_rev_[cur_class]) {
            dfs(child);
        }
        func_table.existScope();
        attr_table.existScope();
    };

    dfs(Object);
}

// check other information
void ClassTable::check_phase2() {
    buildGraphTopDown();
    collectAllMethod();
    // iter all class and check type
    for (int i = classes_->first(); classes_->more(i); i = classes_->next(i)) {
        classes_->nth(i)->typeCheck(env_);
    }
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{
    return semant_error(c->get_filename(),c);
}

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()
{
    semant_errors++;
    return error_stream;
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes);

    // semantic analysis phase 1, check inheritance graph.
    classtable->check_phase1();
    // semantic analysis phase 2, check others.
    classtable->check_phase2();

    if (classtable->errors()) {
    	cerr << "Compilation halted due to static semantic errors." << endl;
	    exit(1);
    }
}

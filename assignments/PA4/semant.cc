

#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include "cool-tree.h"
#include "cool-tree.handcode.h"
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
std::optional<std::pair<Formals, Symbol>> Enviro::getFuncSig(Symbol class_name, Symbol method_name) {
      Symbol cur_class = class_name == SELF_TYPE? cur_class_ : class_name;
      while (cur_class != No_class) {

         auto find_res1 = method_signatures_.find(cur_class);
         if (find_res1 == method_signatures_.end()) {
            cur_class = inherit_graph_[cur_class];
            continue;
         }
         auto find_res2 = find_res1->second.find(method_name);
         if (find_res2 == find_res1->second.end()) {
            cur_class = inherit_graph_[cur_class];
            continue;
         }
         return find_res2->second;
      }
      return {};
}
// check whether class1 is the child of class2
bool Enviro::checkSubClass(Symbol class1, Symbol class2)  {
    class1 = class1 == SELF_TYPE ? cur_class_ : class1;
    class2 = class2 == SELF_TYPE ? cur_class_ : class2;
    if (class1 == No_type) return true;
    Symbol cur = class1;
    while (cur != class2 && cur != Object) {
        cur = inherit_graph_[cur];
    }
    if (cur == class2) return true;
    else return false;
}

// find the lca of classes.
Symbol Enviro::getLca(const std::vector<Symbol>& classes)  {
    const int n = classes.size();
    std::vector<std::vector<Symbol>> paths(n);
    for (int i = 0; i < n; ++i) {
        Symbol cur = classes[i] == SELF_TYPE? cur_class_ : classes[i];
        while (cur != Object) {
            paths[i].push_back(cur);
            cur = inherit_graph_[cur];
        }
    }
    for (auto& x : paths) {
        std::reverse(x.begin(), x.end());
    }
    Symbol ans = Object;
    for (int i = 0; i < paths[0].size(); ++i) {
        bool flag = true;
        Symbol cur = paths[0][i];
        for (int j = 0; j < paths.size(); ++j) {
            if (paths[j].size() <= i || paths[j][i] != cur) {
                flag = false;
                break;
            }
        }
        if (!flag) break;
        else ans = cur;
    }

    if (semant_debug) {
        std::cerr << "find lca!" << std::endl;
        for (auto x : classes) {
            std::cerr << x << "|";
        }
        std::cout << std::endl;
        std::cout << ans << std::endl;
    }

    return ans;
}

void class__class::typeCheck(ClassTable& class_table, Enviro& env) {
    // Add newscope and push attr
      env.newScope();
      env.setCurClass(name);
      // add self to scope
      env.addVar(self, name);
      for (int i = features->first(); features->more(i); i = features->next(i)) {
         if (features->nth(i)->getType() == FeatureType::ATTR_) {
            attr_class* cur_attr = dynamic_cast<attr_class*>(features->nth(i));
            env.addVar(cur_attr->getName(), cur_attr->getDeclType());
         }
      }
      // check attr and method
      for (int i = features->first(); features->more(i); i = features->next(i)) {
         features->nth(i)->typeCheck(class_table, env);
      }

     std::vector<Symbol> child = class_table.getChild(name);
     for (auto x : child) {
         class_table.getClass(x).value()->typeCheck(class_table, env);
     }

      // clear the attr
      env.exitScope();
      // the cur_class will be set by other class
}


void method_class::typeCheck(ClassTable& class_table,  Enviro &env) {
        // add new scope
      env.newScope();
      std::unordered_set<Symbol> se;
      for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
          Formal cur_formal = formals->nth(i);
          if (se.find(cur_formal->getName()) != se.end()) {
            class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Formal parameter " << cur_formal->getName() << " is multiply defined." << std::endl;
          }
          se.insert(cur_formal->getName());
          env.addVar(cur_formal->getName(), cur_formal->getTypeDecl());
      }
      Symbol expr_type = expr->typeCheck(class_table, env);
      if (return_type == SELF_TYPE && (expr_type != SELF_TYPE && expr_type != No_type) || !env.checkSubClass(expr_type, return_type)) {
         // TODO type mismatch;
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Infered return type " << expr_type << " of method " << name << " does not " \
            << " conform to declared return type " << return_type << std::endl;
      }
      env.exitScope();
}


void attr_class::typeCheck(ClassTable& class_table, Enviro& env) {
    Symbol init_type = init->typeCheck(class_table, env);
    if (!env.checkSubClass(init_type, type_decl)) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "attr init error with dismatch type: decl type is" << type_decl << " expr type is: " << init_type << std::endl;
    }
}

Symbol assign_class::typeCheck(ClassTable& class_table,  Enviro &env)  {
      if (name == self) {
          class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Cannot assign to self" << std::endl;
      }
      Symbol expr_type = expr->typeCheck(class_table, env);
      auto decl_type = env.lookUp(name);
      if (!decl_type) {
          class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Assign Expr: Symbol " << name << " is not defined." << std::endl;
        set_type(Object);
      } else {
          if (!env.checkSubClass(expr_type, decl_type.value())) {
              class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) <<  "Type " << expr_type << " of assigned expression does not conform to declared "
                << "type " << decl_type.value() << " of identifier " << name << std::endl;
              set_type(Object);
          } else {
            set_type(decl_type.value());
          }
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
    set_type(Str);
    return Str;
}


Symbol new__class::typeCheck(ClassTable& class_table,  Enviro &env) {
    set_type(type_name);
    return get_type();
}

Symbol dispatch_class::typeCheck(ClassTable& class_table,  Enviro &env) {

    Symbol expr_type = expr->typeCheck(class_table, env);
    auto find_result = env.getFuncSig(expr_type, name);
    if (!find_result) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Dispatch Class: Cannot find method " << expr_type << "::" << name << std::endl;
        set_type(Object);
        return get_type();
    }

    Formals formals = find_result.value().first;
    Symbol ret_type = find_result.value().second;
    if (actual->len() != formals->len()) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Dispatch Class: formal len dismatch with args len, formal len:" << formals->len() << \
            ", args len: " << actual->len() << std::endl;
        set_type(Object);
        return get_type();
    }

    std::vector<Symbol> actual_types;
    for (int i = 0; i < actual->len(); ++i) {
        actual_types.push_back(actual->nth(i)->typeCheck(class_table, env));
    }
    for (int i = 0; i < actual_types.size(); ++i) {
        if (!env.checkSubClass(actual_types[i], formals->nth(i)->getTypeDecl())) {
            class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Dispatch Class: arg and formal dismatch: arg type: " << actual_types[i] << \
              " formal type: "  << formals->nth(i) << std::endl;
            set_type(Object);
            return get_type();
        }
    }

    if (ret_type == SELF_TYPE) {
        set_type(expr_type);
     } else {
        set_type(ret_type);
     }
     return get_type();
}


Symbol static_dispatch_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol expr_type = expr->typeCheck(class_table, env);
    if (!env.checkSubClass(expr_type, type_name)) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Static dispatch error! type dismatch! real: " << \
           expr_type << " Expect: " << type_name << std::endl;
       set_type(Object);
       return get_type();
    }

    auto find_result = env.getFuncSig(type_name, name);
    if (!find_result) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Dispatch Class: Cannot find method " << expr_type << "::" << name << std::endl;
        set_type(Object);
        return get_type();
    }

    Formals formals = find_result.value().first;
    Symbol ret_type = find_result.value().second;
    if (actual->len() != formals->len()) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Dispatch Class: formal len dismatch with args len, formal len:" << formals->len() << \
            ", args len: " << actual->len() << std::endl;
        set_type(Object);
        return get_type();
    }

    std::vector<Symbol> actual_types;
    for (int i = 0; i < actual->len(); ++i) {
        actual_types.push_back(actual->nth(i)->typeCheck(class_table, env));
    }
    for (int i = 0; i < actual_types.size(); ++i) {
        if (!env.checkSubClass(actual_types[i], formals->nth(i)->getTypeDecl())) {
            class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Dispatch Class: arg and formal dismatch: arg type: " << actual_types[i] << \
              " formal type: "  << formals->nth(i) << std::endl;
            set_type(Object);
            return get_type();
        }
    }

    if (ret_type == SELF_TYPE) {
        set_type(expr_type);
    } else {
        set_type(ret_type);
    }
    return get_type();
}


Symbol cond_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol pred_type = pred->typeCheck(class_table, env);
    if (pred_type != Bool) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "cond_class: Pred_type is not Bool, it is " << pred_type << std::endl;
        set_type(Object);
        return Object;
    }

    Symbol then_expr_type = then_exp->typeCheck(class_table, env);
    Symbol else_exp_type = else_exp->typeCheck(class_table, env);
    set_type(env.getLca({then_expr_type, else_exp_type}));
    return get_type();
}


Symbol block_class::typeCheck(ClassTable& class_table,  Enviro &env) {

    Symbol ret_type = No_type;
    for (int i = body->first(); body->more(i); i = body->next(i)) {
        ret_type = body->nth(i)->typeCheck(class_table, env);
    }
    set_type(ret_type);
    return ret_type;
}


Symbol let_class::typeCheck(ClassTable& class_table,  Enviro &env) {

   Symbol init_type = init->typeCheck(class_table, env);
   if (init_type != No_type && !env.checkSubClass(init_type, type_decl)) {
       class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "let class type mismatch: init_type: " << \
           init_type << " type_decl is " << type_decl << std::endl;
   }
   env.newScope();
   if (identifier == self) {
       class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "\'self\' cannot be bound in a \'let\' expression." << std::endl;
   } else {
       env.addVar(identifier, type_decl);
   }
   Symbol ret_type = body->typeCheck(class_table, env);
   env.exitScope();
   set_type(ret_type);
   return get_type();
}


Symbol typcase_class::typeCheck(ClassTable& class_table,  Enviro &env) {

    Symbol expr_type = expr->typeCheck(class_table, env);


    // check each branch has different type

    std::unordered_set<Symbol> se;
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        auto case_ = dynamic_cast<branch_class*>(cases->nth(i));
        if (se.find(case_->getTypeDecl()) != se.end()) {
            class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Duplicate branch " << case_->getTypeDecl() << " in case statement." << std::endl;
        }
        se.insert(case_->getTypeDecl());
    }

    std::vector<Symbol> branch_types;
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
       branch_types.push_back(cases->nth(i)->typeCheck(class_table, env));
    }

    Symbol ret_type = env.getLca(branch_types);
    set_type(ret_type);
    return get_type();

}


Symbol branch_class::typeCheck(ClassTable& class_table,  Enviro &env) {

    env.newScope();
    env.addVar(name, type_decl);
    Symbol ret_type = expr->typeCheck(class_table, env);
    env.exitScope();
    return ret_type;
}


Symbol loop_class::typeCheck(ClassTable& class_table,  Enviro &env) {

    Symbol pred_type = pred->typeCheck(class_table, env);
    if (pred_type != Bool) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "loop class check error: " << pred_type << " is not Bool" << std::endl;
    }
    Symbol expr_type = body->typeCheck(class_table, env);
    set_type(Object);
    return get_type();

}

Symbol isvoid_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    e1->typeCheck(class_table, env);
    set_type(Bool);
    return get_type();
}


Symbol comp_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol tmp_type = e1->typeCheck(class_table, env);

    if (tmp_type != Bool) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Comp class: e1 type is not Bool, is " << tmp_type << std::endl;
        set_type(Object);
    } else {
        set_type(Bool);
    }
    return get_type();

}


Symbol lt_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol e1_type = e1->typeCheck(class_table, env);
    Symbol e2_type = e2->typeCheck(class_table, env);
    if (e1_type != Int || e2_type != Int) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "lt class error: e1 type: " << e1_type  \
            << "e2 type: " << e2_type << std::endl;
        set_type(Object);
    } else {
        set_type(Bool);
    }
    return get_type();
}

Symbol leq_class::typeCheck(ClassTable& class_table, Enviro &env) {
    Symbol e1_type = e1->typeCheck(class_table, env);
    Symbol e2_type = e2->typeCheck(class_table, env);
    if (e1_type != Int || e2_type != Int) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "lt class error: e1 type: " << e1_type  \
            << "e2 type: " << e2_type << std::endl;
        set_type(Object);
    } else {
        set_type(Bool);
    }
    return get_type();
}

Symbol neg_class::typeCheck(ClassTable& class_table,  Enviro &env) {

    Symbol e1_type = e1->typeCheck(class_table, env);
    if (e1_type != Int) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "neg_class error: e1 type is " << e1_type << std::endl;
        set_type(Object);
    } else {
        set_type(Int);
    }
    return get_type();

}


Symbol plus_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol e1_type = e1->typeCheck(class_table, env);
    Symbol e2_type = e2->typeCheck(class_table, env);
    if (e1_type != Int || e2_type != Int) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "plus class error: e1 type: " << e1_type  \
            << " e2 type is " << e2_type << std::endl;
        set_type(Object);
    }  else {
        set_type(Int);
    }
    return get_type();
}

Symbol sub_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol e1_type = e1->typeCheck(class_table, env);
    Symbol e2_type = e2->typeCheck(class_table, env);
    if (e1_type != Int || e2_type != Int) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "plus class error: e1 type: " << e1_type  \
            << " e2 type is " << e2_type << std::endl;
        set_type(Object);
    }  else {
        set_type(Int);
    }
    return get_type();
}

Symbol mul_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol e1_type = e1->typeCheck(class_table, env);
    Symbol e2_type = e2->typeCheck(class_table, env);
    if (e1_type != Int || e2_type != Int) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "plus class error: e1 type: " << e1_type  \
            << " e2 type is " << e2_type << std::endl;
        set_type(Object);
    }  else {
        set_type(Int);
    }
    return get_type();
}

Symbol divide_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol e1_type = e1->typeCheck(class_table, env);
    Symbol e2_type = e2->typeCheck(class_table, env);
    if (e1_type != Int || e2_type != Int) {
        class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "plus class error: e1 type: " << e1_type  \
            << " e2 type is " << e2_type << std::endl;
        set_type(Object);
    }  else {
        set_type(Int);
    }
    return get_type();
}

Symbol eq_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    Symbol e1_type = e1->typeCheck(class_table, env);
    Symbol e2_type = e2->typeCheck(class_table, env);
    if (e1_type == Int || e1_type == Str || e1_type == Bool || e2_type == Int || e2_type == Str || e2_type == Bool) {
        if(e1_type != e2_type) {
            class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "eq class type error: e1 type is " << e1_type  \
                << " e2 type is " << e2_type << std::endl;
            set_type(Object);
        } else {
            set_type(Bool);
        }
    } else {
        set_type(Bool);
    }
    return get_type();
}

Symbol no_expr_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    set_type(No_type);
    return get_type();
}

Symbol object_class::typeCheck(ClassTable& class_table,  Enviro &env) {
    if (name == self) {
        set_type(SELF_TYPE);
    } else {
        auto find_result = env.lookUp(name);
        if (!find_result) {
            class_table.semant_error(class_table.getClass(env.getCurClass()).value()->get_filename(), this) << "Undeclared identifier " << name << std::endl;
            set_type(Object);
         } else {
          set_type(find_result.value());
        }
    }
    return get_type();
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
    symbol_to_class_[Object] = dynamic_cast<class__class*>(Object_class);
    symbol_to_class_[IO] = dynamic_cast<class__class*>(IO_class);
    symbol_to_class_[Int] = dynamic_cast<class__class*>(Int_class);
    symbol_to_class_[Bool] = dynamic_cast<class__class*>(Bool_class);
    symbol_to_class_[Str] = dynamic_cast<class__class*>(Str_class);
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
    SymTable<Symbol, std::pair<Formals, Symbol>> func_table;
    SymTable<Symbol, Symbol> attr_table;

    std::function<void(Symbol)> dfs = [&](Symbol cur_class) {
        func_table.enterScope();
        attr_table.enterScope();
        class__class* t = symbol_to_class_[cur_class];
        Features features = t->get_features();
        for (int i = features->first(); features->more(i); i = features->next(i)) {
            if (features->nth(i)->getType() == FeatureType::METHOD_) {
                // check override method
                method_class* cur_method = dynamic_cast<method_class*>(features->nth(i));
                Symbol method_name = cur_method->getName();
                Formals formals = cur_method->getFormals();
                Symbol ret_type = cur_method->getRetType();
                // check whether same function def in this class
                auto find_res = func_table.probe(method_name);
                if (find_res) {
                    // TODO
                    semant_error(getClass(cur_class).value()->get_filename(), cur_method) << "define same method in this class" << std::endl;
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
                        semant_error(getClass(cur_class).value()->get_filename(), cur_method) << "override with different arg number" << std::endl;
                        abort();
                    }



                    // check arg type
                    for (int j = 0; j < old_formals->len(); ++j) {
                        Formal old_formal = old_formals->nth(j);
                        Formal cur_formal = formals->nth(j);
                        if (!old_formal->equal(cur_formal)) {
                            // TODO
                            semant_error(getClass(cur_class).value()->get_filename(), cur_method) << "override with different arg type" << std::endl;
                            abort();
                        }
                    }

                    // check ret type
                    if (old_ret_type != ret_type) {
                        semant_error(getClass(cur_class).value()->get_filename(), cur_method) << "override with different return type" << std::endl;
                        abort();
                    }

                } else {
                    // check formal type and return type exist.
                    for (int j = formals->first(); formals->more(j); j = formals->next(j)) {
                        Formal cur_formal = formals->nth(j);
                        if (symbol_to_class_.find(cur_formal->getTypeDecl()) == symbol_to_class_.end()) {
                            semant_error(getClass(cur_class).value()->get_filename(), cur_method) << "cannot find the arg type: " << cur_formal->getTypeDecl() << std::endl;
                        }
                    }
                     if (ret_type != SELF_TYPE && symbol_to_class_.find(ret_type) == symbol_to_class_.end()) {
                        semant_error(getClass(cur_class).value()->get_filename(), cur_method) << "cannot find the the ret type: " << ret_type << std::endl;
                    }
                }
                // check arg name
                for (int j = 0; j < formals->len(); ++j) {
                        Formal cur_formal = formals->nth(j);
                        if (semant_debug) {
                            std::cerr << cur_formal->getName() << std::endl;
                        }
                        if (cur_formal->getName() == self) {
                            semant_error(getClass(cur_class).value()->get_filename(), cur_method) << "formal arg cannot be self" << std::endl;
                        }
                    }
                // add this method to funtable
                env_.addFuncSig(cur_class,
                                method_name,
                                formals,
                                ret_type);
                func_table.add(method_name, {formals, ret_type});

                if (semant_debug) {
                    std::cerr << "Insert Func to Env: " << cur_class << "::" << method_name << std::endl;
                }

            } else {
                // check attr override
                attr_class* attr = dynamic_cast<attr_class*>(features->nth(i));
                if (attr->getName() == self) {
                    semant_error(symbol_to_class_[cur_class]) << "\'self\' cannot be the name of attribute." << std::endl;
                    abort();
                }
                if (attr_table.lookUp(attr->getName())) {
                    semant_error(symbol_to_class_[cur_class]) << "Attribute " << attr->getName() << " is an attribute of an inherited class." << std::endl;
                    abort();
                }
                attr_table.add(attr->getName(), attr->get_decl_type());
            }
        }

        // check children
        for (auto child : graph_rev_[cur_class]) {
            dfs(child);
        }
        func_table.exitScope();
        attr_table.exitScope();
    };

    dfs(Object);
}

// check other information
void ClassTable::check_phase2() {

    env_ = Enviro(graph_);
    buildGraphTopDown();

    if (semant_debug) {
        std::cerr << "buidl graph end!" << std::endl;
    }

    collectAndCheckAllMethod();

    if (semant_debug) {
        std::cerr << "Collect and check method end!" << std::endl;
    }

    getClass(Object).value()->typeCheck(*this, env_);
    // iter all class and check type
    // for (int i = classes_->first(); classes_->more(i); i = classes_->next(i)) {
    //     classes_->nth(i)->typeCheck(*this, env_);
    // }
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

ostream& ClassTable::semant_error(Symbol filename, tree_node *t) {
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()
{
    semant_errors++;
    return error_stream;
}

/*   This is the entry point to the semantic checker.

    env_ = Enviro();
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

    if (!semant_debug) {
        if (classtable->errors()) {
    	cerr << "Compilation halted due to static semantic errors." << endl;
	    exit(1);
     }
    }

}

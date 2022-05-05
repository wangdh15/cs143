
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include <unordered_map>
#include <algorithm>
#include <vector>
#include "cgen.h"
#include "cgen_gc.h"

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;

//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol
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

static char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };


//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream &os)
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  CgenClassTable *codegen_classtable = new CgenClassTable(classes,os);

  os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")"
    << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << endl; }

static void emit_load_address(char *dest_reg, char *address, ostream& s)
{ s << LA << dest_reg << " " << address << endl; }

static void emit_partial_load_address(char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream& s)
{ s << MOVE << dest_reg << " " << source_reg << endl; }

static void emit_neg(char *dest, char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << endl; }

static void emit_add(char *dest, char *src1, char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << endl; }

static void emit_addu(char *dest, char *src1, char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << endl; }

static void emit_addiu(char *dest, char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << endl; }

static void emit_div(char *dest, char *src1, char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << endl; }

static void emit_mul(char *dest, char *src1, char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << endl; }

static void emit_sub(char *dest, char *src1, char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << endl; }

static void emit_sll(char *dest, char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << endl; }

static void emit_jalr(char *dest, ostream& s)
{ s << JALR << "\t" << dest << endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << endl; }

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_label_ref(int l, ostream &s)
{ s << "label" << l; }

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }


static void emit_test_collector(ostream &s)
{
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP,SP,4,s);
  emit_load(ACC,0,SP,s);
}

static void emit_gc_check(char *source, ostream &s)
{
  if (source != (char*)A1) emit_move(A1, source, s);
  s << JAL << "_gc_check" << endl;
}


///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s  << LABEL                                             // label
      << WORD << stringclasstag << endl                                 // tag
      << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << endl // size
      << WORD;


 /***** Add dispatch information for class String ******/
      emit_disptable_ref(Str, s);

      s << endl;                                              // dispatch table
      s << WORD;  lensym->code_ref(s);  s << endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag)
{
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,stringclasstag);
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                // label
      << WORD << intclasstag << endl                      // class tag
      << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl  // object size
      << WORD;

 /***** Add dispatch information for class Int ******/
      emit_disptable_ref(Int, s);

      s << endl;                                          // dispatch table
      s << WORD << str << endl;                           // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
  for (List<IntEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,intclasstag);
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}

//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                  // label
      << WORD << boolclasstag << endl                       // class tag
      << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl   // object size
      << WORD;

 /***** Add dispatch information for class Bool ******/
      emit_disptable_ref(Bool, s);
      s << endl;                                            // dispatch table
      s << WORD << val << endl;                             // value (0 or 1)
}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << endl;
  str << GLOBAL; falsebool.code_ref(str);  str << endl;
  str << GLOBAL; truebool.code_ref(str);   str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //
  str << INTTAG << LABEL
      << WORD << intclasstag << endl;
  str << BOOLTAG << LABEL
      << WORD << boolclasstag << endl;
  str << STRINGTAG << LABEL
      << WORD << stringclasstag << endl;
}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void CgenClassTable::code_bools(int boolclasstag)
{
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

void CgenClassTable::code_select_gc()
{
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}

void CgenClassTable::set_class_tag_internal(CgenNodeP cur, int cur_dep) {

  class_depth[cur->get_name()] = cur_dep;
  int begin_idx = tag_to_class.size();
  tag_to_class.push_back(cur->get_name());
  auto children = cur->get_children();
  for (List<CgenNode> *l = children; l; l = l->tl()) {
    CgenNodeP cur_child = l->hd();
    set_class_tag_internal(cur_child, cur_dep + 1);
  }
  int end_idx = tag_to_class.size() - 1;
  subclass_idrange[cur->get_name()] = {begin_idx, end_idx};
}

// set a tag for every class
// use dfs seq number for simplify case expression
void CgenClassTable::set_class_tag() {

  CgenNodeP root_class = root();
  set_class_tag_internal(root_class, 0);

  // set the correct class tag
  stringclasstag = subclass_idrange[idtable.lookup_string("String")].first;
  intclasstag = subclass_idrange[idtable.lookup_string("Int")].first;
  boolclasstag = subclass_idrange[idtable.lookup_string("Bool")].first;

  if (cgen_debug) {
    cout << "The class tag range is: " << std::endl;
    for (auto& [k, v] : subclass_idrange) {
      cout << k << " " << v.first << " " << v.second << endl;
    }
    cout << "The class depth in inherite tree" << std::endl;
    for (auto& [k, v] : class_depth) {
      cout << k << ' ' << v << std::endl;
    }
  }
}

//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  stringtable.add_string("");
  inttable.add_string("0");

  stringtable.code_string_table(str,stringclasstag);
  inttable.code_string_table(str,intclasstag);
  code_bools(boolclasstag);
}

// generate the class nametab
// the seq is index by class tag
void CgenClassTable::code_class_nameTab() {
  const int n = tag_to_class.size();

  str << CLASSNAMETAB << LABEL;
  for (int i = 0; i < n; ++i) {
    str << WORD;
    StringEntryP tt = stringtable.lookup_string(tag_to_class[i]->get_string());
    tt->code_ref(str);
    str << endl;
  }
}

// gene ObjTab
void CgenClassTable::code_class_objTab() {
  const int n = tag_to_class.size();

  str << CLASSOBJTAB << LABEL;
  for (int i = 0; i < n; ++i) {
    str << WORD; emit_protobj_ref(tag_to_class[i], str); str << endl;
    str << WORD; emit_init_ref(tag_to_class[i], str); str << endl;
  }
}

void CgenClassTable::gene_dispTab_for_one_class(CgenNodeP cur, std::ostream& str) {

  std::vector<CgenNodeP> path;

  while (cur -> get_name() != Object) {
    path.push_back(cur);
    cur = cur->get_parentnd();
  }
  path.push_back(cur);

  std::reverse(path.begin(), path.end());

  std::unordered_map<Symbol, int> method_idx;
  std::vector<std::pair<Symbol, Symbol>> methods;

  for (auto _class : path) {
    Features features = _class->get_features();
    for (int i = features->first(); features->more(i); i = features->next(i)) {

      Feature feature = features->nth(i);
      if (feature->getType() == FeatureType::METHOD_) {
        Symbol method_name = feature->get_name();
        if (method_idx.find(method_name) == method_idx.end()) {
          // the first definition of method
          methods.push_back({_class->get_name(), method_name});
          method_idx[method_name] = methods.size() - 1;
        } else {
          // override method
          methods[method_idx[method_name]] = {_class->get_name(), method_name};
        }
      }
    }
  }

  for (auto& [class_name, method_name] : methods) {
    str << WORD; emit_method_ref(class_name, method_name, str); str << endl;
  }

  // fill the class_method_offset
  auto& t = class_method_offset[path.back()->get_name()];
  const int nn = methods.size();
  for (int i = 0; i < nn; ++i) {
    t[methods[i].second] = i;
  }
}

void CgenClassTable::code_class_dispTab() {


 for(List<CgenNode> *l = nds; l; l = l->tl()) {
    CgenNode* cur = l->hd();
    emit_disptable_ref(cur->get_name(), str); str << LABEL;
    gene_dispTab_for_one_class(cur, str);
 }

  if (cgen_debug) {
    std::cerr << "the method offset of each class is: " << std::endl;
    for (auto& [class_name, t] : class_method_offset) {
      std::cerr << class_name << std::endl;
      for (auto& [method_name, idx] : t) {
        std::cerr << "\t" << method_name << "\t" << idx << std::endl;
      }
      std::cerr << "======================" << std::endl;
    }
  }

}


std::vector<std::pair<Symbol, Symbol>> CgenClassTable::collect_all_attr(CgenNodeP cur) {
  std::vector<CgenNodeP> path;
  while (cur->get_name() != Object) {
    path.push_back(cur);
    cur = cur->get_parentnd();
  }
  path.push_back(cur);

  std::reverse(path.begin(), path.end());

  std::vector<std::pair<Symbol, Symbol>> all_attrs;

  for (auto& _class : path) {

    Features features = _class->get_features();
    for (int i = features->first(); features->more(i); i = features->next(i)) {
      Feature feature = features->nth(i);
      if (feature->getType() == FeatureType::ATTR_) {
        all_attrs.push_back({feature->get_name(), feature->get_type()});
      }
    }
  }

  auto& t = class_attr_offset[path.back()->get_name()];
  const int nn = all_attrs.size();
  for (int i = 0; i < nn; ++i) {
    t[all_attrs[i].first] = i;
  }

  return all_attrs;
}

void CgenClassTable::code_class_protObj() {
  for(List<CgenNode> *l = nds; l; l = l->tl()) {
    str << WORD << -1 << endl;
    CgenNode* cur = l->hd();
    emit_protobj_ref(cur->get_name(), str); str << LABEL;
    str << WORD << subclass_idrange[cur->get_name()].first << endl;
    auto all_attrs = collect_all_attr(cur);
    if (cur->get_name() == Str) {
      str << WORD << 5 << endl;
      str << WORD; emit_disptable_ref(cur->get_name(), str); str << endl;
      str << WORD ; inttable.lookup_string("0")->code_ref(str); str << endl;
      emit_string_constant(str, "");
    } else if (cur->get_name() == Int || cur->get_name() == Bool) {
      str << WORD << 4 << endl;

      str << WORD; emit_disptable_ref(cur->get_name(), str); str << endl;
      str << WORD << 0 << endl;
    } else {
      str << WORD << DEFAULT_OBJFIELDS + all_attrs.size() << endl;
      str << WORD; emit_disptable_ref(cur->get_name(), str); str << endl;
      for (auto& x : all_attrs) {
          if (x.second == Int ||
              x.second == Str ||
              x.second == Bool) {
                // for Int String Bool, the default value is protObj
                // else if 0(void)
                str << WORD; emit_protobj_ref(x.second, str); str << endl;
              } else {
                str << WORD << 0 << std::endl;
              }
      }
    }
 }

  if (cgen_debug) {
    std::cerr << "Print the offset of all class attr" << std::endl;
    for (auto& [class_name, t] : class_attr_offset) {
      std::cerr << class_name << std::endl;
      for (auto& [attr_name, idx]: t) {
        std::cerr << '\t' << attr_name << '\t' << idx << std::endl;
      }
    }
    std::cout << "===================================" << std::endl;
  }

}

static void emit_method_begin(std::ostream& s) {
  emit_addiu(SP, SP, -12, s);
  // store fp
  emit_store(FP, 3, SP, s);
  // store ra
  emit_store(RA, 2, SP, s);
  // store SELF
  emit_store(SELF, 1, SP, s);
  // set fp
  emit_addiu(FP, SP, 12, s);
  // set SELF
  emit_move(SELF, ACC, s);
}

static void emit_method_end(std::ostream& s, int num_args) {
  // resotre register
  emit_load(SELF, 1, SP, s);
  emit_load(RA, 2, SP, s);
  emit_load(FP, 3, SP, s);

  // modify sp
  emit_addiu(SP, SP, (3 + num_args) * WORD_SIZE, s);
  emit_return(s);
}


// generate the init code for attr.
void CgenClassTable::code_class_init() {

  for(List<CgenNode> *l = nds; l; l = l->tl()) {
    CgenNodeP cur_class = l->hd();
    setCurClass(cur_class);
    // add attrs to symbol
    auto& t = class_attr_offset[cur_class->get_name()];
    enterScope();
    for (auto& [name, offset] : t) {
      addSymbol(name, BASE_LOC_TYPE::SELF_, offset + DEFAULT_OBJFIELDS);
    }
    // addSymbol(self, BASE_LOC_TYPE::SELF_, 0);

    emit_init_ref(cur_class->get_name(), str); str << LABEL;
    emit_method_begin(str);

    // call init method of parent;
    if (cur_class->get_name() != Object) {
      CgenNodeP parent = cur_class->get_parentnd();
      emit_move(ACC, SELF, str);
      str << JAL; emit_init_ref(parent->get_name(), str); str << endl;
      emit_move(SELF, ACC, str);
    }
    // for Int String Bool, no need to init attr.
    if (cur_class->get_name() != Int &&
        cur_class->get_name() != Str &&
        cur_class->get_name() != Bool) {

        Features features = cur_class->get_features();
        for (int i = features->first(); features->more(i); i = features->next(i)) {
          Feature feature = features->nth(i);
          if (feature->getType() == FeatureType::ATTR_) {
                feature->code(str, *this);
          }
        }

    }
    emit_method_end(str, 0);
    exitScope();
  }

}

// generate the code for other method
void CgenClassTable::code_class_method() {

  for (List<CgenNode> *l = nds; l; l = l -> tl()) {
    CgenNodeP cur_class = l->hd();
    setCurClass(cur_class);
    // add attrs to Symbol
    Symbol cur_class_name = cur_class->get_name();
    if (cur_class_name == Object ||
        cur_class_name == Str ||
        cur_class_name == IO ||
        cur_class_name == Int ||
        cur_class_name == Bool) continue;
    auto& t = class_attr_offset[cur_class->get_name()];
    enterScope();
    for (auto& [name, offset] : t) {
      addSymbol(name, BASE_LOC_TYPE::SELF_, offset + DEFAULT_OBJFIELDS);
    }
    addSymbol(self, BASE_LOC_TYPE::SELF_, 0);

    Features features = cur_class->get_features();
    for (int i = features->first(); features->more(i); i = features->next(i)) {
      Feature feature = features->nth(i);
      if (feature->getType() == FeatureType::METHOD_) {
        emit_method_ref(cur_class->get_name(), feature->get_name(), str); str << LABEL;
        feature->code(str, *this);
      }
    }
    exitScope();
  }
}

CgenClassTable::CgenClassTable(Classes classes, ostream& s) : nds(NULL) , str(s)
{
  //  stringclasstag = 2 /* Change to your String class tag here */;
  //  intclasstag =    3 /* Change to your Int class tag here */;
  //  boolclasstag =   4 /* Change to your Bool class tag here */;

   enterscope();
   if (cgen_debug) cout << "Building CgenClassTable" << endl;
   install_basic_classes();
   install_classes(classes);
   build_inheritance_tree();

    if (cgen_debug) cout << "Set Class Tag" << std::endl;
    set_class_tag();

   code();
   exitscope();
}

void CgenClassTable::install_basic_classes()
{

// The tree package uses these globals to annotate the classes built below.
  //curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
	new CgenNode(class_(No_class,No_class,nil_Features(),filename),
			    Basic,this));
  addid(SELF_TYPE,
	new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
			    Basic,this));
  addid(prim_slot,
	new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
			    Basic,this));

//
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
   new CgenNode(
    class_(Object,
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename),
    Basic,this));

//
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
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
	   filename),
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer.
//
   install_class(
    new CgenNode(
     class_(Int,
	    Object,
            single_Features(attr(val, prim_slot, no_expr())),
	    filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//
   install_class(
    new CgenNode(
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
	     filename),
        Basic,this));

}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
  Symbol name = nd->get_name();

  if (probe(name))
    {
      return;
    }

  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  nds = new List<CgenNode>(nd,nds);
  addid(name,nd);
}

void CgenClassTable::install_classes(Classes cs)
{
  for(int i = cs->first(); cs->more(i); i = cs->next(i)) {
    install_class(new CgenNode(cs->nth(i),NotBasic,this));
  }
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
  for(List<CgenNode> *l = nds; l; l = l->tl())
      set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n)
{
  children = new List<CgenNode>(n,children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}



void CgenClassTable::code()
{
  if (cgen_debug) cout << "coding global data" << endl;
  code_global_data();

  if (cgen_debug) cout << "choosing gc" << endl;
  code_select_gc();

  if (cgen_debug) cout << "coding constants" << endl;
  code_constants();

//                 Add your code to emit
//                   - prototype objects
//                   - class_nameTab
//                   - dispatch tables
//

//  MY CODE BEGIN
  if (cgen_debug) cout << "coding class_nameTab" << endl;

  code_class_nameTab();

  if (cgen_debug) cout << "coding class_objTab" << endl;

  code_class_objTab();

  if (cgen_debug) cout << "coding class_dispTab" << endl;

  code_class_dispTab();

  if (cgen_debug) cout << "coding class_protObj" << endl;

  code_class_protObj();

// MY CODE END

  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();

//                 Add your code to emit
//                   - object initializer
//                   - the class methods
//                   - etc...


// MY CODE BEIGN

  if (cgen_debug) cout << "coding class initializer" << endl;
  code_class_init();

  if (cgen_debug) cout << "coding class other method" << endl;

  code_class_method();


// MY CODE END

}


CgenNodeP CgenClassTable::root()
{
   return probe(Object);
}


///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   children(NULL),
   basic_status(bstatus)
{
   stringtable.add_string(name->get_string());          // Add class name to string table
}


//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************

void assign_class::code(ostream &s, CgenClassTable& cgen_class) {
  // gene the code for expr
  expr->code(s, cgen_class);
  // now the result is in ACC
  auto& symbol_to_loc = cgen_class.getSymbolToLoc();
  auto loc = symbol_to_loc.lookUp(name);
  if (loc.base_loc == BASE_LOC_TYPE::SELF_) {
    // if the name is attr
    emit_store(ACC, loc.offset, SELF, s);
  } else {
    // else the name is args or let or branch, there are in stack.
    emit_store(ACC, loc.offset, FP, s);
  }
}

void static_dispatch_class::code(ostream &s, CgenClassTable& cgen_class) {
  // push bp on stack
  // emit_push(FP, s);
  // eval every arg
  for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
    Expression expression = actual->nth(i);
    // eval args
    expression->code(s, cgen_class);
    emit_push(ACC, s);
  }
  // eval expr, the result, self, is in ACC.
  expr->code(s, cgen_class);

  int no_void_label = cgen_class.get_next_labelid();
  // check void
  emit_bne(ACC, ZERO, no_void_label, s);

    // set the filename
  Symbol file_name = cgen_class.getCurClass()->get_filename();
  emit_partial_load_address(ACC, s);
  stringtable.lookup_string(file_name->get_string())->code_ref(s);
  s << endl;
  // set the lineno
  int line_no = get_line_number();
  emit_load_imm(T1, line_no, s);
  // call abort
  emit_jal("_dispatch_abort", s);// call

  emit_label_def(no_void_label, s);

  // load dispTab
  emit_partial_load_address(T1, s);
  emit_disptable_ref(type_name, s);
  s << endl;

  // emit_load(T1, DISPTABLE_OFFSET, ACC, s);

  // get the method offset
  int method_offset = cgen_class.get_method_offset(type_name, name);

  // load method
  emit_load(T1, method_offset, T1, s);

  // call funciton
  emit_jalr(T1, s);

}

void dispatch_class::code(ostream &s, CgenClassTable& cgen_class) {
  // push bp on stack
  // emit_push(FP, s);
  // eval every arg
  for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
    Expression expression = actual->nth(i);
    // eval args
    expression->code(s, cgen_class);
    emit_push(ACC, s);
  }
  // eval expr, the result, self, is in ACC.
  expr->code(s, cgen_class);

  int no_void_label = cgen_class.get_next_labelid();
  // check void
  emit_bne(ACC, ZERO, no_void_label, s);

    // set the filename
  Symbol file_name = cgen_class.getCurClass()->get_filename();
  emit_partial_load_address(ACC, s);
  stringtable.lookup_string(file_name->get_string())->code_ref(s);
  s << endl;
  // set the lineno
  int line_no = get_line_number();
  emit_load_imm(T1, line_no, s);
  // call abort
  emit_jal("_dispatch_abort", s);// call

  emit_label_def(no_void_label, s);

  // load dispTab
  emit_load(T1, DISPTABLE_OFFSET, ACC, s);

  // get the type of expr
  Symbol type_name = expr->get_type();

  // get the method offset
  int method_offset = cgen_class.get_method_offset(type_name, name);

  // load method
  emit_load(T1, method_offset, T1, s);

  // call funciton
  emit_jalr(T1, s);

}

void cond_class::code(ostream &s, CgenClassTable& cgen_class) {
  // gene the code for pred，the result is in ACC
  pred->code(s, cgen_class);

  // load the address of bool_const1
  emit_load_bool(T1, BoolConst(TRUE), s);

  // get else label
  int else_label = cgen_class.get_next_labelid();

  // compare two Bool
  emit_move(T2, ACC, s);

  emit_load_bool(ACC, BoolConst(TRUE), s);
  emit_load_bool(A1, BoolConst(FALSE), s);

  emit_jal("equality_test", s);

  // compare
  emit_beq(ACC, A1, else_label, s);

  // gene the code for true branch
  then_exp->code(s, cgen_class);

  int end_label = cgen_class.get_next_labelid();
  // jump to end
  emit_branch(end_label, s);

  // gene else label
  emit_label_def(else_label, s);

  // gene else code
  else_exp->code(s, cgen_class);

  // gene the end label
  emit_label_def(end_label, s);
}

void loop_class::code(ostream &s, CgenClassTable& cgen_class) {
  // gene the test label
  int test_idx = cgen_class.get_next_labelid();
  int exit_idx = cgen_class.get_next_labelid();
  emit_label_def(test_idx, s);

  // get the pred result
  pred->code(s, cgen_class);

  // load true to T1
  emit_load_bool(T1, BoolConst(TRUE), s);

  // check the result
  emit_bne(ACC, T1, exit_idx, s);

  // gene body code
  body->code(s, cgen_class);

  // goto test
  emit_branch(test_idx, s);

  // gene exit label
  emit_label_def(exit_idx, s);

  // set result is void
  emit_move(ACC, ZERO, s);
}

void typcase_class::code(ostream &s, CgenClassTable& cgen_class) {
  // gene code for expr;
  expr->code(s, cgen_class);

  // get end idx;
  int end_label = cgen_class.get_next_labelid();

  int no_zero_label = cgen_class.get_next_labelid();
  int case_error1_label = cgen_class.get_next_labelid();

  // test expr is void of not
  emit_bne(ACC, ZERO, no_zero_label, s);

  // set the filename
  Symbol file_name = cgen_class.getCurClass()->get_filename();
  emit_partial_load_address(ACC, s);
  stringtable.lookup_string(file_name->get_string())->code_ref(s);
  s << endl;
  // set the lineno
  int line_no = get_line_number();
  emit_load_imm(T1, line_no, s);
  // call abort
  emit_jal("_case_abort2", s);


  // begin process;
  emit_label_def(no_zero_label, s);
  // get class tag
  emit_load(T1, TAG_OFFSET, ACC, s);

  std::vector<Case> sorted_result;
  for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
    sorted_result.push_back(cases->nth(i));
  }
  std::sort(sorted_result.begin(), sorted_result.end(), [&](Case& a, Case& b) -> bool {
    return cgen_class.get_depth(a->get_type_decl()) > cgen_class.get_depth(b->get_type_decl());
  });

  const int n = sorted_result.size();

  // gene code for each branch
  // int cur_branch_idx = cgen_class.get_next_labelid();
  for (int i = 0; i < n; ++i) {
    // emit_label_def(cur_branch_idx, s);
    Case cur_case = sorted_result[i];
    Symbol type_decl = cur_case->get_type_decl();
    auto [begin_idx, end_idx] = cgen_class.get_subclass_idrang(type_decl);
    int next_branch_idx = cgen_class.get_next_labelid();
    // check match
    emit_blti(T1, begin_idx, next_branch_idx, s);
    emit_bgti(T1, end_idx, next_branch_idx, s);
    // push value to stack
    emit_push(ACC, s);
    // update Symbol table
    cgen_class.enterScope();
    int cur_stack_offset = cgen_class.push_new_var();
    cgen_class.addSymbol(cur_case->get_name(), BASE_LOC_TYPE::FP_,
                         cur_stack_offset);
    // gene expr code
    cur_case->get_expr()->code(s, cgen_class);
    // delete symbol
    cgen_class.exitScope();
    // resume stack state
    emit_addiu(SP, SP, 4, s);
    cgen_class.pop_new_var(1);
    // goto final index
    emit_branch(end_label, s);
    // gene the next branch label
    emit_label_def(next_branch_idx, s);
  }

  // nomatch:
  emit_label_def(case_error1_label, s);
  // call abort
  emit_jal("_case_abort", s);

  // gene the final label
  emit_label_def(end_label, s);
}

void block_class::code(ostream &s, CgenClassTable& cgen_class) {
  for (int i = body->first(); body->more(i); i = body->next(i)) {
    Expression expr = body->nth(i);
    expr->code(s, cgen_class);
  }
}

void let_class::code(ostream &s, CgenClassTable& cgen_class) {
  if (init->get_type() == NULL) {
    // copy the default value
    if (type_decl == Int || type_decl == Str || type_decl == Bool) {
      emit_partial_load_address(ACC, s); emit_protobj_ref(type_decl, s); s << endl;
      emit_jal("Object.copy", s);
    } else {
      // return void
      emit_move(ACC, ZERO, s);
    }
  } else {
    // gene the code for expr
    init->code(s, cgen_class);
  }
  // push value to stack
  emit_push(ACC, s);
  // update symbol
  cgen_class.enterScope();
  int cur_stack_offset = cgen_class.push_new_var();
  cgen_class.addSymbol(identifier, BASE_LOC_TYPE::FP_,
                      cur_stack_offset);
  // gene the code for body
  body->code(s, cgen_class);
  // clear symbol
  cgen_class.exitScope();
  cgen_class.pop_new_var(1);
  // resume stack state
  emit_addiu(SP, SP, 4, s);
}

static void load_two_int(std::ostream &s, CgenClassTable& cgen_class,
                   Expression e1, Expression e2) {
  // gene code for e1
  e1->code(s, cgen_class);
  // push result to stack
  emit_push(ACC, s);

  // gene code for e2;
  e2->code(s, cgen_class);

  // copy the result
  emit_jal("Object.copy", s);

  // load e1 result to T1
  emit_load(T1, 1, SP, s);

  // result stack state
  emit_addiu(SP, SP, 4, s);

  // load the int value
  emit_load(T2, DEFAULT_OBJFIELDS, T1, s);
  emit_load(T3, DEFAULT_OBJFIELDS, ACC, s);
}

void plus_class::code(ostream &s, CgenClassTable& cgen_class) {

  load_two_int(s, cgen_class, e1, e2);
  emit_add(T2, T2, T3, s);

  // store the value to ACC
  emit_store(T2, DEFAULT_OBJFIELDS, ACC, s);
}

void sub_class::code(ostream &s, CgenClassTable& cgen_class) {
  load_two_int(s, cgen_class, e1, e2);

  emit_sub(T2, T2, T3, s);

  emit_store(T2, DEFAULT_OBJFIELDS, ACC, s);
}

void mul_class::code(ostream &s, CgenClassTable& cgen_class) {
  load_two_int(s, cgen_class, e1, e2);

  emit_mul(T2, T2, T3, s);

  emit_store(T2, DEFAULT_OBJFIELDS, ACC, s);
}

void divide_class::code(ostream &s, CgenClassTable& cgen_class) {

  load_two_int(s, cgen_class, e1, e2);

  emit_div(T2, T2, T3, s);

  emit_store(T2, DEFAULT_OBJFIELDS, ACC, s);

}

void neg_class::code(ostream &s, CgenClassTable& cgen_class) {
  // gene code for e1
  e1->code(s, cgen_class);

  // copy new data
  emit_jal("Object.copy", s);

  // load the int value to T1
  emit_load(T1, DEFAULT_OBJFIELDS, ACC, s);

  // neg
  emit_neg(T1, T1, s);

  // store
  emit_store(T1, DEFAULT_OBJFIELDS, ACC, s);
}

void lt_class::code(ostream &s, CgenClassTable& cgen_class) {
  load_two_int(s, cgen_class, e1, e2);

  // load true
  emit_load_bool(ACC, BoolConst(TRUE), s);
  // emit_load_address(ACC, "bool_const1");

  int final_branch_id = cgen_class.get_next_labelid();

  emit_blt(T2, T3, final_branch_id, s);

  // load false
  emit_load_bool(ACC, BoolConst(FALSE), s);
  // emit_load_address(ACC, "bool_const0");

  // gene final label
  emit_label_def(final_branch_id, s);
}

void eq_class::code(ostream &s, CgenClassTable& cgen_class) {

  // load to value to t1 and t2
  e1->code(s, cgen_class);
  emit_push(ACC, s);
  // push to stack
  e2->code(s, cgen_class);
  emit_move(T2, ACC, s);
  emit_load(T1, 1, SP, s);
  emit_addiu(SP, SP, 4, s);

  // load true to a0 and false to a1
  emit_load_bool(ACC, BoolConst(TRUE), s);
  emit_load_bool(A1, BoolConst(FALSE), s);

  // get set false label;
  int false_label = cgen_class.get_next_labelid();

  // get out label
  int out_label = cgen_class.get_next_labelid();

  // compare address first
  emit_beq(T1, T2, out_label, s);

  // // if not equal, check whether one is void
  // emit_beqz(T1, false_label, s);
  // emit_beqz(T2, false_label, s);

  // call the lib function equality test
  emit_jal("equality_test", s);

  // goto final label
  emit_branch(out_label, s);

  // false branch
  // emit_label_def(false_label, s);

  // load false
  emit_load_bool(ACC, BoolConst(FALSE), s);

  // out branch
  emit_label_def(out_label, s);
}

void leq_class::code(ostream &s, CgenClassTable& cgen_class) {
    load_two_int(s, cgen_class, e1, e2);

  // load true
  emit_load_bool(ACC, BoolConst(TRUE), s);
  // emit_load_address(ACC, "bool_const1");

  int final_branch_id = cgen_class.get_next_labelid();

  emit_bleq(T2, T3, final_branch_id, s);

  // load false
  emit_load_bool(ACC, BoolConst(FALSE), s);
  // emit_load_address(ACC, "bool_const0", s);

  // gene final label
  emit_label_def(final_branch_id, s);
}

void comp_class::code(ostream &s, CgenClassTable& cgen_class) {
  // gene code for e1
  e1->code(s, cgen_class);
  // move result to T1
  emit_move(T1, ACC, s);
  emit_load_bool(T2, BoolConst(FALSE), s);
  emit_load_bool(ACC, BoolConst(TRUE), s);
  emit_load_bool(A1, BoolConst(FALSE), s);

  emit_jal("equality_test", s);
}

void int_const_class::code(ostream &s, CgenClassTable& cgen_class)
{
  //
  // Need to be sure we have an IntEntry *, not an arbitrary Symbol
  //
  emit_load_int(ACC,inttable.lookup_string(token->get_string()),s);
}

void string_const_class::code(ostream &s, CgenClassTable& cgen_class)
{
  emit_load_string(ACC,stringtable.lookup_string(token->get_string()),s);
}

void bool_const_class::code(ostream &s, CgenClassTable& cgen_class)
{
  emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(ostream &s, CgenClassTable& cgen_class) {
  // load  the address of protObj

  if (type_name == SELF_TYPE) {
    // if is self-type, should call from the class tag
    emit_load(T1, 0, ACC, s);
    emit_load_address(T2, "class_objTab", s);
    emit_load_imm(T3, 8, s);
    emit_mul(T1, T1, T3, s);
    emit_add(T2, T2, T1, s);
    // load protObj;
    emit_load(ACC, 0, T2, s);
    emit_jal("Object.copy", s);

    // call init
    emit_load(T1, 0, ACC, s);
    emit_load_address(T2, "class_objTab", s);
    emit_load_imm(T3, 8, s);
    emit_mul(T1, T1, T3, s);
    emit_addiu(T1, T1, 4, s);
    emit_add(T2, T2, T1, s);
    // call init ;
    emit_load(A1, 0, T2, s);
    emit_jalr(A1, s);

  } else {
    emit_partial_load_address(ACC, s);
    Symbol real_type = type_name;
    emit_protobj_ref(real_type, s); s << endl;

    // copy the protObj
    emit_jal("Object.copy", s);

    // call the init funcion
    s << JAL; emit_init_ref(real_type, s); s << endl;
  }

}

void isvoid_class::code(ostream &s, CgenClassTable& cgen_class) {
  // gene code for e1
  e1->code(s, cgen_class);

  // move to T1
  emit_move(T1, ACC, s);

  // load true first
  emit_load_bool(ACC, BoolConst(TRUE), s);

  // get final index
  int final_idx = cgen_class.get_next_labelid();

  emit_beqz(T1, final_idx, s);

  // load false
  emit_load_bool(ACC, BoolConst(FALSE), s);

  // gene the final label
  emit_label_def(final_idx, s);
}

void no_expr_class::code(ostream &s, CgenClassTable& cgen_class) {

}

void object_class::code(ostream &s, CgenClassTable& cgen_class) {
  if (name == self) {
    emit_move(ACC, SELF, s);
    return;
  }
  auto loc = cgen_class.getSymbolToLoc().lookUp(name);
  if (loc.base_loc == BASE_LOC_TYPE::SELF_) {
    emit_load(ACC, loc.offset, SELF, s);
  } else {
    emit_load(ACC, loc.offset, FP, s);
  }
}

int CgenClassTable::get_method_offset(Symbol class_name, Symbol method_name) {
      if (class_name == SELF_TYPE) class_name = cur_class->get_name();
      if (class_method_offset.find(class_name) == class_method_offset.end()) {
         std::cerr << "get_method_offset error! cannot find:"  \
                   << class_name << "::" << method_name << std::endl;
      }
      auto& t = class_method_offset[class_name];
      assert(t.find(method_name) != t.end());
      return t[method_name];
}


// cool-tree.h implement begin

void attr_class::code(std::ostream& s, CgenClassTable& cgen_class) {

  if (init->get_type() == NULL) {
    // if the init is no_expr, this field is set proper by protObj,
    // so no need to modify this attr.
    return;
  }

  init->code(s, cgen_class);
  // assign the result to attr
  auto loc = cgen_class.getSymbolToLoc().lookUp(name);
  emit_store(ACC, loc.offset, SELF, s);
  // return self
  emit_move(ACC, SELF, s);
}

void method_class::code(std::ostream& s, CgenClassTable& cgen_class) {
  // add args to Symbol Table
  cgen_class.enterScope();
  std::vector<Symbol> args;
  for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
    Formal formal = formals->nth(i);
    args.push_back(formal->get_name());
  }
  std::reverse(args.begin(), args.end());
  const int n = args.size();
  for (int i = 0; i < n; ++i) {
    cgen_class.addSymbol(args[i], BASE_LOC_TYPE::FP_, i + 1);
  }

  emit_method_begin(s);

  expr->code(s, cgen_class);

  emit_method_end(s, n);

  // pop the args
  // emit_addiu(SP, SP, n * WORD_SIZE, s);

  cgen_class.exitScope();

}


// cool-tree.h end

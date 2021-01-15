
#include <iostream>
#include <map>


class ExprNode {
public:
   virtual void *walk( void *(*combiner)(ExprNode *op, int nargs, void**pargs))=0;
};

class SymbolExprNode: public ExprNode {
public:
   const char *symbol;
   int tokenid;
   void *walk( void *(*combiner)(ExprNode *op, int nargs, void**pargs)) {
      return((*combiner)(this, 0, 0));
   }
   // other stuff later as needed
};

class MultiExprNode: public ExprNode {
public:
   SymbolExprNode *op;
   int nsubexp;
   ExprNode *subexp[10]; 
   void *walk( void *(*combiner)(ExprNode *op, int nargs, void**pargs)) {
      void *sres[10];
      for(int i = 0; i < nsubexp; i++) {
         sres[i] = subexp[i]->walk(combiner);
      }
      return (*combiner)(op, nsubexp, sres);
   }
};

class Lexer {
    struct { int fromstate; int cc; bool final; int tostate;} graph;
    static const int MAXLINE=1024;
    int curtoken;
    int curlinenum;
    char curline[MAXLINE];
    char *curpos;
    char **re_list;
    int n_res;
    int max_res;
    bool compiled;
    void compile_all();        // internal, called by peek/get
public:
    Lexer() {
        max_res = 128;
        n_res = 0;

    Define("formal_arg_list", 
        Opt("(", Lookup("formal_list"), Symbol(")")));

    Define("formal_list", 
        Seq(Lookup("formal"), Opt(",", Lookup("formal_list"))));

    Define("formal",  
        Seq(Symbol("\\w+"), Lookup("spec_type")));

    Define("type",   
       Or(
        Seq(Symbol("array"), Symbol("["), Lookup("expression"), Symbol("]"), Lookup("type")),
        Seq(Symbol("class"), Symbol("["), Lookup("expression"), Symbol("]"), Lookup("type")),
        Symbol("\\w+")));

    Define("statement",  
        Or(
           Seq( Symbol("if"), Lookup("guardlist"), Symbol("fi"), Lookup("opt_assertion")),
           Seq( Symbol("it"), Lookup("opt_invariant") , Lookup("opt_bound"), Lookup("guardlist"), Symbol("fi") , Lookup("opt_assertion")),
           Lookup("assignment")
        ));

    Define("stmt_seq", Seq( Lookup("stmt"), Lookup("opt_Assertion"), Opt(";", Lookup("stmt"))));

    Define("opt_Assertion",  Opt("{", Lookup("predicate"), Symbol("}")));

    Define("guardlist",  Seq( Lookup("guard"), Opt(";;", Lookup("guardlist"))));

    Define("guard",  Seq( Lookup("predicate"), Symbol("->"), Lookup("stmt_seq")));

    Define("assignemnt",  Seq( Lookup("varlist"), Symbol("="), Lookup("exprlist")));

    Define("opt_invariant", Opt("Inv:", Lookup("predicate")) );
    Define("opt_bound", Opt("Bound:", Lookup("expression")) );


    Define("predicate",  
         Seq(Lookup("and_pred"), Or(
             Seq( Symbol("==="), Lookup("predicate")),
             Seq( Symbol("!=="), Lookup("predicate")),
             Seq( Symbol("==>"), Lookup("predicate")),
             Seq( Symbol("<=="), Lookup("predicate")),
             Empty())));

    Define("and_pred",  Seq(Lookup("or_pred"), Opt("&&", Lookup("and_pred"))));

    Define("or_pred",  Seq(Lookup("comp_pred"), Opt("||", Lookup("or_pred"))));

    Define("comp_pred", Seq(Lookup("expression"), Or(
                           Seq( Symbol("=="), Lookup("comp_pred")),
                           Seq( Symbol("!="), Lookup("comp_pred")),
                           Seq( Symbol(">="), Lookup("comp_pred")),
                           Seq( Symbol("<="), Lookup("comp_pred")),
                           Empty())));

    Define("expression", Seq(Lookup("term"), Or(
                           Seq( Symbol("+"), Lookup("expression")),
                           Seq( Symbol("-"), Lookup("expression")),
                           Seq( Symbol("|"), Lookup("expression")),
                           Empty())));

    Define("primary", Or( 
                  Seq( Symbol("-"), Lookup("primary")),
                  Seq( Symbol("!"), Lookup("primary")),
                  Symbol("\\d+"),
                  Symbol("0x\\x+"),
                  Seq( Symbol("\\w+"), Or(
                        Seq( Symbol("["), Lookup("expression"), Symbol("]")),
                        Seq( Symbol("("), Lookup("expression_list"), Symbol(")")),
                        Seq( Symbol("."), Lookup("primary")),
                        Empty()))));
}

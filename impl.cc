
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
    bool compiled;
    void compile_all();        // internal, called by peek/get
public:
    int add_regexp(const char *re);  // add a regexp, geta token-id
    int peek(std::istream &s);  // get token-id of upcoming token
    void syntax_error();        // print error at file:line 
    SymbolExprNode *get(std::istream &s, SymbolExprNode *update);
                               // consume token, put in SymbolExprNode
};

class ParserObj {
public:
    Lexer *lexer;
    virtual ExprNode *parse(std::istream &str, SymbolExprNode **update );
};

class EmptyParserObj : public ParserObj {
    Lexer *lexer;
public:
    ExprNode *parse(std::istream &str, SymbolExprNode **update ) { return 0; }
};

class SymbolParserObj : public ParserObj {
    int _token_id;
public:
    SymbolParserObj(Lexer *l, int token_id){ lexer = l; _token_id=token_id;}
    ExprNode *parse(std::istream &str, SymbolExprNode **update ) {
        if (lexer->peek(str) == _token_id) {
            SymbolExprNode *res = new SymbolExprNode();
            return lexer->get(str, res);
        } else {
            return 0;
        }
    }
};

class SeqParserObj : public ParserObj {
    ParserObj *subs[10];
public:
    SeqParserObj(Lexer *pl, ParserObj *p0, ParserObj *p1=0, ParserObj *p2=0, ParserObj *p3=0, ParserObj *p4=0, ParserObj *p5=0, ParserObj *p6=0, ParserObj *p7=0, ParserObj *p8=0, ParserObj *p9=0) {
      lexer = pl;
      subs[0] = p0;
      subs[1] = p1;
      subs[2] = p2;
      subs[3] = p3;
      subs[4] = p4;
      subs[5] = p5;
      subs[6] = p6;
      subs[7] = p7;
      subs[8] = p8;
      subs[9] = p9;
    }
    ExprNode *parse(std::istream &str, SymbolExprNode **update ) {
        MultiExprNode *res = new MultiExprNode;
        for(int i = 0; i < 10; i++ ) { 
            if(subs[i]) {
                res->subexp[i] = subs[i]->parse(str, update);
                if (res->subexp[i] == 0) {
                    if ( i > 0 ) {
                       lexer->syntax_error();
                    }
                    return 0;
                }
            }
        }
        return res;
    }
};


class OrParserObj : public ParserObj {
    ParserObj *subs[10];
public:
    OrParserObj(Lexer *pl, ParserObj *p0, ParserObj *p1=0, ParserObj *p2=0, ParserObj *p3=0, ParserObj *p4=0, ParserObj *p5=0, ParserObj *p6=0, ParserObj *p7=0, ParserObj *p8=0, ParserObj *p9=0) {
      lexer = pl;
      subs[0] = p0;
      subs[1] = p1;
      subs[2] = p2;
      subs[3] = p3;
      subs[4] = p4;
      subs[5] = p5;
      subs[6] = p6;
      subs[7] = p7;
      subs[8] = p8;
      subs[9] = p9;
    }
    ExprNode *parse(std::istream &str, SymbolExprNode **update ) {
        ExprNode *res;
        for(int i = 0; i < 10; i++ ) { 
            if(subs[i]) {
                res = subs[i]->parse(str, update);
                if( res ) {
                    return res;
                }
            }
        }
        return 0;
    }
};

static Lexer _our_lexer;

static std::map<const char *, ParserObj *> _parser_dict;

void 
Define(const char *name, ParserObj *what) { // define an expression by name
   _parser_dict[name] = what;
}

ParserObj *
Lookup(const char *name) {
   return _parser_dict[name];
}

ParserObj *
Symbol(const char *pc){    // match a regexp 
   int token_id = _our_lexer.add_regexp(pc);
   ParserObj *res = new SymbolParserObj(&_our_lexer, token_id);
   return res;
}

ParserObj *
Or(ParserObj *p0, ParserObj *p1=0, ParserObj *p2=0, ParserObj *p3=0, ParserObj *p4=0, ParserObj *p5=0, ParserObj *p6=0, ParserObj *p7=0, ParserObj *p8=0, ParserObj *p9=0) {
    return new OrParserObj(&_our_lexer, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

ParserObj *
Seq(ParserObj *p0, ParserObj *p1=0, ParserObj *p2=0, ParserObj *p3=0, ParserObj *p4=0, ParserObj *p5=0, ParserObj *p6=0, ParserObj *p7=0, ParserObj *p8=0, ParserObj *p9=0) {
    return new SeqParserObj(&_our_lexer, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

ParserObj *
Empty() {
   static EmptyParserObj empty;
   return &empty;
}

ParserObj *
Opt(const char *p0, ParserObj *p1=0, ParserObj *p2=0, ParserObj *p3=0, ParserObj *p4=0, ParserObj *p5=0, ParserObj *p6=0, ParserObj *p7=0, ParserObj *p8=0, ParserObj *p9=0) {
    return new SeqParserObj(&_our_lexer, Or(Symbol(p0), p1, p2, p3, p4, p5, p6, p7, p8, p9),Empty());
}

void build_parser() {

    Define("func",  
       Seq(Symbol("function"), Symbol("\\w+"),  Lookup("formal_arg_list"), Lookup("spec_type"), Opt("pre:", Lookup("predicate"), Opt("post:", Lookup("predicate")) , Lookup("block"))));

    Define("block", 
        Seq(Symbol("begin"), Lookup("stmt_seq"), Symbol("end")));

    Define("spec_type", 
        Seq(Symbol(":") , Lookup("type")));

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

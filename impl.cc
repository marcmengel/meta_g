
#include <iostream>
#include <map>
#include <string.h>
#include "tokens.h"

extern char yytext[];
extern int gettoken(std::istream &);

class Parser {

public:
    // forward declarations...
    class ParserObj;
    class Lexer;
    class SymbolExprNode;

    Lexer *_our_lexer;
    ParserObj *_top;
    static std::map<const char *, ParserObj *> _parser_dict;

    class ExprNode {
    public:
       virtual void *walk( void *(*combiner)(SymbolExprNode *op, int nargs, void**pargs))=0;
    };

    class SymbolExprNode: public ExprNode {
    public:
       const char *symbol;
       int tokenid;
       void *walk( void *(*combiner)(SymbolExprNode *op, int nargs, void**pargs)) {
          return((*combiner)(this, 0, 0));
       }
       // other stuff later as needed
    };

    class MultiExprNode: public ExprNode {
    public:
       SymbolExprNode *op;
       int nsubexp;
       ExprNode *subexp[10]; 
       void *walk( void *(*combiner)(SymbolExprNode *op, int nargs, void**pargs)) {
          void *sres[10];
          for(int i = 0; i < nsubexp; i++) {
             sres[i] = subexp[i]->walk(combiner);
          }
          return (*combiner)(op, nsubexp, sres);
       }
    };

    class Lexer {
       int _token;
    public:

        Lexer(std::istream &s) {
           _token = gettoken(s);
        }
        Lexer(const Lexer &s) {
           _token = s._token;
        }

        int peek(std::istream &s) {  // get token-id of upcoming token
            return _token;
        }

        SymbolExprNode *get(std::istream &s, SymbolExprNode **update) {
                                   // consume token, put in SymbolExprNode
             #ifdef DEBUG
                std::cout << "token: " << _token << ": " << yytext << "\n";
             #endif
             SymbolExprNode *res = new SymbolExprNode();
             res->tokenid = _token;
             res->symbol = strdup(yytext);
             _token = gettoken(s);
             if (update) {
                 *update = res;
             }
             return res;
        }
    };

    class ParserObj {
    public:
        Lexer *lexer;
        virtual ExprNode *parse(std::istream &str, SymbolExprNode **update ) = 0;
    };

    class EmptyParserObj : public ParserObj {
        
    public:
        ExprNode *parse(std::istream &str, SymbolExprNode **update ) { 
           static SymbolExprNode empty;
           empty.symbol = "";
           empty.tokenid = -1;
           return &empty; 
        }
    };

    class SymbolParserObj : public ParserObj {
      int _token_id;
    public:
        SymbolParserObj(Lexer *l, int token_id){ lexer = l; _token_id=token_id;}
        ExprNode *parse(std::istream &str, SymbolExprNode **update ) {
            #ifdef DEBUG
                std::cout << "SymbolParserObj: looking for: " << tokenstr(_token_id) << "\n";
            #endif
            if (lexer->peek(str) == _token_id) {
                return lexer->get(str, update);
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
            extern void syntax_error(const char *);
            MultiExprNode *res = new MultiExprNode;
             
            #ifdef DEBUG
            std::cout << "starting: SeqParseObj::parse\n";
            #endif

            for(int i = 0; i < 10; i++ ) { 
                if(subs[i]) {
                    res->subexp[i] = subs[i]->parse(str, update);
                    if (res->subexp[i] == 0) {
                        if ( i > 0 ) {
                           syntax_error("unexpected symbol");
                        }
                        return 0;
                    }
                }
            }
            #ifdef DEBUG
            std::cout << "ending: SeqParseObj::parse\n";
            #endif
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
            #ifdef DEBUG
            std::cout << "starting: OrParseObj::parse\n";
            #endif
            for(int i = 0; i < 10; i++ ) { 
                if(subs[i]) {
                    res = subs[i]->parse(str, update);
                    if( res ) {
                        #ifdef DEBUG
                        std::cout << "leaving: OrParseObj::parse\n";
                        #endif
                        return res;
                    }
                }
            }
            #ifdef DEBUG
            std::cout << "leaving: OrParseObj::parse\n";
            #endif
            return 0;
        }
    };

    class PendingLookup : public ParserObj {
    public:
        const char *_name;
        PendingLookup(const char *name) { _name = name; }
        ExprNode *parse(std::istream &str, SymbolExprNode **update ) {
           #ifdef DEBUG
           std::cout << "starting: " << _name << "\n";
           #endif
           ExprNode *res =  _parser_dict[_name]->parse(str,update);
           #ifdef DEBUG
           std::cout << "done: " << _name << "\n";
           #endif
           return res;
        }
    };


    void 
    Define(const char *name, ParserObj *what) { // define an expression by name
       _parser_dict[name] = what;
    }

    ParserObj *
    Lookup(const char *name) {
       #ifdef DEBUG
           return new PendingLookup(name);
       #endif
       if (_parser_dict.find(name) != _parser_dict.end() ) {
           return  _parser_dict[name];
       } else {
           return new PendingLookup(name);
       }
    }

    ParserObj *
    Symbol(int token_id){    // match a regexp 
       ParserObj *res = new SymbolParserObj(_our_lexer, token_id);
       return res;
    } ParserObj *
    Or(ParserObj *p0, ParserObj *p1=0, ParserObj *p2=0, ParserObj *p3=0, ParserObj *p4=0, ParserObj *p5=0, ParserObj *p6=0, ParserObj *p7=0, ParserObj *p8=0, ParserObj *p9=0) {
        return new OrParserObj(_our_lexer, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    ParserObj *
    Seq(ParserObj *p0, ParserObj *p1=0, ParserObj *p2=0, ParserObj *p3=0, ParserObj *p4=0, ParserObj *p5=0, ParserObj *p6=0, ParserObj *p7=0, ParserObj *p8=0, ParserObj *p9=0) {
        return new SeqParserObj(_our_lexer, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    ParserObj *
    Empty() {
       static EmptyParserObj empty;
       return &empty;
    }

    ParserObj *
    Opt(int p0, ParserObj *p1=0, ParserObj *p2=0, ParserObj *p3=0, ParserObj *p4=0, ParserObj *p5=0, ParserObj *p6=0, ParserObj *p7=0, ParserObj *p8=0, ParserObj *p9=0) {
        return new OrParserObj(_our_lexer, Seq(Symbol(p0), p1, p2, p3, p4, p5, p6, p7, p8, p9),Empty());
    }

    void build_parser(std::istream &s) {

        _our_lexer = new Lexer(s);

        Define("func",  
           Seq(Symbol(T_FUNCTION), Symbol(T_NAME),  Lookup("formal_arg_list"), Lookup("spec_type"), Opt(T_PRE, Lookup("predicate")), Opt(T_POST, Lookup("predicate")) , Lookup("block")));

        Define("block", 
            Seq(Symbol(T_BEGIN), Lookup("decl_seq"), Lookup("stmt_seq"), Symbol(T_END)));

        Define("decl_seq",
            Or(Seq(Lookup("decl"), Lookup("decl_seq")),Empty()));

        Define("decl", Seq(Symbol(T_VAR), Lookup("name_list"), Symbol(':'), Lookup("type"), Symbol(';')));

        Define("spec_type", 
            Seq(Symbol(':') , Lookup("type")));

        Define("formal_arg_list", 
            Opt('(', Lookup("formal_list"), Symbol(')')));

        Define("formal_list", 
            Seq(Lookup("formal"), Opt(',', Lookup("formal_list"))));

        Define("formal",  
            Seq(Symbol(T_NAME), Symbol(':') , Lookup("type")));

        Define("type",   
           Or(
            Seq(Symbol(T_ARRAY), Symbol('['), Lookup("expression"), Symbol(']'), Lookup("type")),
            Seq(Symbol(T_CLASS), Symbol('['), Lookup("expression"), Symbol(']'), Lookup("type")),
            Symbol(T_NAME)));

        Define("statement",  
            Or(
               Seq( Symbol(T_IF), Lookup("guardlist"), Symbol(T_FI), Lookup("opt_assertion")),
               Seq( Symbol(T_DO), Lookup("opt_invariant") , Lookup("opt_bound"), Lookup("guardlist"), Symbol(T_OD) , Lookup("opt_assertion")),
               Lookup("assignment")
            ));

        Define("stmt_seq", Seq( Lookup("stmt"), Lookup("opt_Assertion"), Opt(';', Lookup("stmt"))));

        Define("opt_Assertion",  Opt('{', Lookup("predicate"), Symbol('}')));

        Define("guardlist",  Seq( Lookup("guard"), Opt(T_ENDGUARD, Lookup("guardlist"))));

        Define("guard",  Seq( Lookup("predicate"), Symbol(T_ARROW), Lookup("stmt_seq")));

        Define("assignemnt",  Seq( Lookup("varlist"), Symbol('='), Lookup("exprlist")));

        Define("stmt",
           Or( 
               Seq(Symbol(T_IF), Lookup("guardlist"), Symbol(T_FI)),
               Seq(Symbol(T_DO), Lookup("guardlist"), Symbol(T_OD)),
               Seq(Lookup("namelist"), Symbol('='), Lookup("predicate_list"))
           ));

        Define("opt_invariant", Opt(T_INV, Lookup("predicate")) );
        Define("opt_bound", Opt(T_BOUND, Lookup("expression")) );

        Define("predicate_list",
               Seq(Lookup("predicate"), Opt(',',Lookup("predicate_list"))));

        Define("predicate",  
             Seq(Lookup("and_pred"), Or(
                 Seq( Symbol(T_EQUIV), Lookup("predicate")),
                 Seq( Symbol(T_NEQUIV), Lookup("predicate")),
                 Seq( Symbol(T_IMPL), Lookup("predicate")),
                 Seq( Symbol(T_IMPLBY), Lookup("predicate")),
                 Empty())));

        Define("and_pred",  Seq(Lookup("or_pred"), Opt(T_LAND, Lookup("and_pred"))));

        Define("or_pred",  Seq(Lookup("comp_pred"), Opt(T_LOR, Lookup("or_pred"))));

        Define("comp_pred", Seq(Lookup("expression"), Or(
                               Seq( Symbol(T_EQUAL), Lookup("comp_pred")),
                               Seq( Symbol(T_NEQ), Lookup("comp_pred")),
                               Seq( Symbol(T_GEQ), Lookup("comp_pred")),
                               Seq( Symbol(T_LEQ), Lookup("comp_pred")),
                               Seq( Symbol('<'), Lookup("comp_pred")),
                               Seq( Symbol('>'), Lookup("comp_pred")),
                               Empty())));

        Define("expression", Seq(Lookup("term"), Or(
                               Seq( Symbol('+'), Lookup("expression")),
                               Seq( Symbol('-'), Lookup("expression")),
                               Seq( Symbol('|'), Lookup("expression")),
                               Empty())));

        Define("term", Seq(Lookup("primary"), Or(
                              Seq( Symbol('*'), Lookup("term")),
                              Seq( Symbol('/'), Lookup("term")),
                              Seq( Symbol('%'), Lookup("term")),
                              Seq( Symbol('&'), Lookup("term")),
                              Empty())));

        Define("primary", Or( 
                      Seq( Symbol('-'), Lookup("primary")),
                      Seq( Symbol('!'), Lookup("primary")),
                      Symbol(T_NUMBER),
                      Symbol(T_NUMBER),
                      Seq( Symbol(T_NAME), Or(
                            Seq( Symbol('['), Lookup("expression"), Symbol(']')),
                            Seq( Symbol('('), Lookup("predicate_list"), Symbol(')')),
                            Seq( Symbol('.'), Lookup("primary")),
                            Empty()))));
        _top = Lookup("func");
    }
    
    ExprNode *parse(std::istream &str) {
        return _top->parse(str, 0);
    }
};

void *
dumper(Parser::SymbolExprNode *op, int nargs, void**pargs) {
    if (op && op->symbol) {
        std::cout <<  op->symbol << "\n";
    } else {
        std::cout << "(null)" << "\n";
    }
}

std::map<const char *, Parser::ParserObj *> Parser::_parser_dict;

#ifdef UNITTEST
main() {
   Parser::ExprNode *res;
   std::cout <<  "starting:\n";
   Parser p;
   p.build_parser(std::cin);
   std::cout <<  "parse output:\n";
   res = p.parse(std::cin);
   std::cout <<  "expr dump:\n";
   res->walk(dumper);
}

#endif

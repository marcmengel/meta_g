
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
       const char *element;
       virtual void *walk( void *(*combiner)(SymbolExprNode *op, char *element, int nargs, void**pargs))=0;
    };

    class SymbolExprNode: public ExprNode {
    public:
       const char *symbol;
       int tokenid;
       void *walk( void *(*combiner)(SymbolExprNode *op, char *element, int nargs, void**pargs)) {
          return((*combiner)(this, "",  0, 0));
       }
       // other stuff later as needed
    };

    class MultiExprNode: public ExprNode {
    public:
       SymbolExprNode *op;
       int nsubexp;
       ExprNode *subexp[10]; 
       MultiExprNode() { nsubexp = 0; op = 0; for(int i=0; i< 10; i++){subexp[i]=0;}}
       void *walk( void *(*combiner)(SymbolExprNode *op, char *element, int nargs, void**pargs)) {
          void *rev;
          void *sres[10];
          void *res;
          rev = (*combiner)(op, element,  -1,0);
          if (rev != 0) {
              for(int i = nsubexp - 1; i >= 0; i--) {
                 sres[i] = subexp[i]->walk(combiner);
              }
          } else {
              for(int i = 0; i < nsubexp; i++) {
                 sres[i] = subexp[i]->walk(combiner);
              }
          }
          res = (*combiner)(op, element, nsubexp, sres);
          (*combiner)(op, element,  -2,0);
          return res;
       }
    };

    class Lexer {
       int _token;
       char _toktxt[255];
    public:

        Lexer(std::istream &s) {
           _token = gettoken(s);
           strcpy(_toktxt, yytext);
        }
        Lexer(const Lexer &s) {
           _token = s._token;
           strcpy(_toktxt, s._toktxt);
        }

        int peek(std::istream &s) {  // get token-id of upcoming token
            return _token;
        }

        ExprNode *get(std::istream &s, SymbolExprNode **update) {
                                   // consume token, put in SymbolExprNode
             SymbolExprNode *res = new SymbolExprNode();
             ExprNode *res2 = res;
             res->tokenid = _token;
             res->symbol = strdup(_toktxt);
             _token = gettoken(s);
             strcpy(_toktxt, yytext);
             // update the operator in an a parent node if it is not set
             if ((update && !*update) || (update && *update && (*update)->tokenid == T_NAME )) {
                 #ifdef DEBUG
                    std::cout << "updating operator:"<< res->symbol << "\n";
                 #endif
                 *update = res;
             }
             #ifdef DEBUG
                std::cout << "constructed SymbolExprNode: "<< (long int)res2 << "{ " << res->tokenid << ": '" << res->symbol << "' }\n";
             #endif
             return res2;
        }
    };

    class ParserObj {
    public:
        Lexer *lexer;
        virtual ExprNode *parse(std::istream &str, SymbolExprNode **update , const char *element) = 0;
    };

    class EmptyParserObj : public ParserObj {
    public:
        static SymbolExprNode empty;

        ExprNode *parse(std::istream &str, SymbolExprNode **update, const char *element ) { 
           // Empty does *not* set the op in **update
           
           EmptyParserObj::empty.symbol = "__empty__";
           EmptyParserObj::empty.tokenid = -1;
           return &empty; 
        } 
    };


    class SymbolParserObj : public ParserObj {
      int _token_id;
    public:
        SymbolParserObj(Lexer *l, int token_id){ lexer = l; _token_id=token_id;}
        ExprNode *parse(std::istream &str, SymbolExprNode **update, const char *element ) {
            ExprNode *res;
            // expects lexer->get to set *update if it is not set.
            #ifdef DEBUG
                std::cout << "SymbolParserObj: looking for: " << tokenstr(_token_id) << "\n";
            #endif
            if (lexer->peek(str) == _token_id) {
                res = lexer->get(str, update);
                res->element = element;
                return res;
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
        ExprNode *parse(std::istream &str, SymbolExprNode **update, const char *element ) {
            extern void syntax_error(const char *);
            MultiExprNode *res = new MultiExprNode;
            ExprNode *res2 = res;
             
            #ifdef DEBUG
            std::cout << "starting: SeqParseObj::parse\n";
            #endif

            for(int i = 0; i < 10; i++ ) { 
                if(subs[i]) {
                    res->nsubexp++;
                    // have each subexpression try to update our operator
                    res->subexp[i] = subs[i]->parse(str, &(res->op),element);
                    if (res->subexp[i] == 0) {
                        if ( i > 0 ) {
                           syntax_error("unexpected symbol");
                        }
                        #ifdef DEBUG
                          std::cout << "ending: SeqParseObj::parse: no match\n";
                        #endif
                        return 0;
                    }
                }
            }
            res->element = element;
            if (res->nsubexp == 2 && res->subexp[1] == &EmptyParserObj::empty) {
                res2 = res->subexp[0];
                delete res;
            } else {
            #ifdef DEBUG
            std::cout << "constructed: MultiExprNode " << (long int)(res2) << " {" << (long int)(res->op) << ": " 
                      << (long int)(subs[0]) << ","
                      << (long int)(subs[1])<< ","
                      << (long int)(subs[2]) << ","
                      << (long int)(subs[3]) << ","
                      << (long int)(subs[4]) << "}\n";
            std::cout << "ending: SeqParseObj::parse\n";
            #endif
               ;
            }
            return res2;
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
        ExprNode *parse(std::istream &str, SymbolExprNode **update, const char *element ) {
            ExprNode *res;
            #ifdef DEBUG
            std::cout << "starting: OrParseObj::parse\n";
            #endif
            for(int i = 0; i < 10; i++ ) { 
                if(subs[i]) {
                    // which ever subexpression matches sets the
                    // operator of our parent.
                    res = subs[i]->parse(str, update,element);
                    if( res ) {
                        #ifdef DEBUG
                        std::cout << "leaving: OrParseObj::parse\n";
                        #endif
                        return res;
                    } 
                }
            }
            if (res) {
                res->element = element;
            }
            #ifdef DEBUG
            std::cout << "leaving: OrParseObj::parse returning: " << (long int)(res)<< " \n";
            #endif
            return res;
        }
    };

    class PendingLookup : public ParserObj {
    public:
        const char *_name;
        PendingLookup(const char *name) { _name = name; }
        ExprNode *parse(std::istream &str, SymbolExprNode **update, const char *element ) {
           ExprNode *res;
           #ifdef DEBUG
           std::cout << "starting: " << _name << "\n";
           #endif
           res =  _parser_dict[_name]->parse(str,update, _name);
           #ifdef DEBUG
           std::cout << "done: " << _name << " returning " << int(res) << "\n";
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
    } 
 
    ParserObj *
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

#include "grammar_spec"

        _top = Lookup("decl_seq");
    }
    
    ExprNode *parse(std::istream &str) {
        return _top->parse(str, 0, "top");
    }
};

void *
dumper(Parser::SymbolExprNode *op, char *element, int nargs, void**pargs) {
    static int indent = 0;

    if (nargs == -2) {
       indent -= 2;
    }
    for( int i = 0; i < indent; i++) {
        std::cout << ' ';
    }
    if (nargs == -1) {
       std::cout << "[" ;
       std::cout << "('" ;
       if (op && op->symbol) {
          std::cout << op->symbol;
       } else {
          std::cout << "(null)";
       }
       std::cout << "':'" ;
       if (element) {
          std::cout << element;
       }
       std::cout << "')\n";
       indent += 2;
    } else if (nargs == -2) {
       std::cout << "]\n";
    } else if (nargs == 0) {
        if (op && op->symbol) {
            std::cout << op->symbol << "\n";
        } else {
          std::cout << "(null)";
        }
    } else {
       std::cout << "#" << nargs << "\n";
    }
    return 0;
}

void *
meta_dumper(Parser::SymbolExprNode *op, char *element, int nargs, void**pargs) {
    if ( op && op->tokenid == -1) {
        return 0;
    }
    if (nargs == 0) {
        if (op && op->symbol) {
            std::cout << op->symbol;
            if ( op->tokenid ==  ';' || op->tokenid == T_BEGIN) {
                std::cout << '\n';
            } else {
                std::cout << ' ';
            }
        }
    }
    return 0;
}


void *
c_dumper(Parser::SymbolExprNode *op, char *element, int nargs, void**pargs) {
    static bool in_declaration = false;
    static bool in_formals = false;

    if ( nargs == 0 && op && op->tokenid == -1) {
        return (void*)0;
    }
    if (nargs == 0 && 0 == strcmp(element,"formal") || in_declaration)  {
        in_declaration = 1;
        if ( op ) {
            return (void *)(op->symbol);
        } else {
            return (void*)0;
        }
    }
    if (nargs == -2 && in_declaration ) {
        std::cout << " /* declaration: */ " ;
        std::cout << "\n" ;
        in_declaration = false;
        return (void*)0;
    }
    if (nargs == -1 && op && op->tokenid == ':') {
        // do declarations the other way around in C...
        return (void*)1;
    }
    if (nargs == -2 && in_formals) {
        in_formals = false;
        return (void*)0;
    }
    if (nargs == 0) {
        if (op && op->symbol) {
            switch(op->tokenid) {
            case ':':
                // C declarations are type name with no separator..
                break;
            case T_FUNCTION:
                in_formals = true;
                break;
            case T_END:
                std::cout << ";\n}\n";
                break;
            case T_BEGIN:
                std::cout << "{\n";
                break;
            case T_IF:
                std::cout << "if( ";
                break;
            case T_FI:
                std::cout << ";\n} else { assert(0); }\n";
                break;
            case T_DO:
                std::cout << "while(1) if( ";
                break;
            case T_ARROW:
                std::cout << ") {\n";
                break;
            case T_ENDGUARD:
                std::cout << ";\n} else if (";
                break;
            case T_OD:
                std::cout << ";\n} else { break; }\n";
                break;
            default:
                std::cout << op->symbol;
                break;
            }
            if ( op->tokenid ==  ';' || op->tokenid == T_BEGIN) {
                std::cout << '\n';
            } else {
                std::cout << ' ';
            }
        }
    }
    return (void*)0;
}

std::map<const char *, Parser::ParserObj *> Parser::_parser_dict;
Parser::SymbolExprNode Parser::EmptyParserObj::empty;


#ifdef UNITTEST
main() {
   Parser::ExprNode *res;
   std::cout <<  "starting:\n";
   Parser p;
   p.build_parser(std::cin);
   std::cout <<  "parse output:\n";
   res = p.parse(std::cin);
   std::cout <<  "\n-----\ndebug dump:\n";
   res->walk(dumper);
   std::cout <<  "\n-----\nsource dump:\n";
   res->walk(meta_dumper);
   std::cout <<  "\n-----\nC dump:\n";
   res->walk(c_dumper);
}

#endif

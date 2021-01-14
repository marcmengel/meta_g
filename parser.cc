#include <iostream>
#include <ctype.h>
#include <string.h>

class TreeNode {
public:
   char *op;
   TreeNode *subexps[6];
};

class ParserChunk {
public:
   ParserChunk() {;}
   ParserChunk(const ParserChunk &x) {;}
   virtual TreeNode *do_parse(std::istream istr, char **update_op) = 0;
};

class Empty : public ParserChunk {
public:
   Empty() {;}
   TreeNode *do_parse(std::istream istr, char **update_op) { return (TreeNode *)0;}
};

class Empty empty;


class matcher {
// regexp subset matcher, supports *just* enough to match
// \w+
// \s*
// \*
// \+
// [^"]*|\\"
// specific chars

    char *start, *cur, *alt;
    bool saw_backslash;
public:

    matcher(char *s) { start = cur = s; saw_backslash = 0; alt = 0; }
    bool final() { return *cur == 0 || alt && *alt == 0;}
    int clock(int c) {
       char *cur_before = cur;
       int res = 0;
       if (cur[0] == 0) { //end of pattern, does not match :-)
          return 0;
       }
       if (alt && c == *alt) {
          // we match what came after the plus or star.. jump ahead
          // only works for ...a*b not ...a*a type matches
          cur = alt+1;
          alt = 0;
          res = 1;
       } if (cur[0] == '\\') {
           if (cur[1] == 'w' && isalnum(c)) { cur += 2; res = 1; }
           if (cur[1] == 's' && isspace(c)) { cur += 2; res =  1; }
           if (cur[1] == c) { cur += 2; res = 1; }
           return 0;
       } else if (cur[0] == '[') {
          int matched = 0;
          int negated = 0;
          char *start = cur;
        
          if (cur[1] == '^') {
             negated = 1;
             cur += 2;
          } else {
             negated = 0;     
             cur += 1;
          }
          while (*cur && *cur != ']') {
             if (*cur == c) {
                matched = 1;
             }
          }
          if (*cur == ']') {
             cur++;
          }
          res = matched ^ negated;
       } else if (cur[0] == c) {
          cur++;
          res = 1;
       }
       if (*cur && *cur == '*' || *cur == '+') {
          alt = cur+1;
          cur = cur_before;
       }
       return res;
    }
};

class Symbol : public ParserChunk {
    // this is a placeholder for a proper one
    // what we should do is put all the Symbols into a pool
    // make a flex-style matcher for them and 
    // rework for that...
public:
   char *sym_re;
   Symbol(char *re) { sym_re = re; }

   TreeNode * do_parse(std::istream istr, char **update_op) { 
      static const int MAXTOKEN=64;
      static char buf[MAXTOKEN];
      TreeNode *res = 0;
      matcher m(sym_re);
      int c = istr.peek();
      int i = 0;
      while( m.clock(c) ) {
         buf[i++] = c;
         istr.get();
         c = istr.peek();
      }
      if (i > 0 && m.final()) {
         buf[i++] = 0;
         res = new TreeNode; 
         res->op = strdup(buf);
         res->subexps[0] = (TreeNode*) res->op;
         if (update_op != 0 && *update_op == 0) {
             *update_op = res->op;
         }
      } else {
         while( i > 1) {
            i--;
            istr.unget();
         }
         return 0;
      }
   }
};

class Seq : public ParserChunk {
public:
   ParserChunk *parts[6];

   Seq( ParserChunk *p0, ParserChunk *p1 = &empty, ParserChunk *p2 = &empty, ParserChunk *p3 = &empty, ParserChunk *p4 = &empty,ParserChunk *p5 = &empty) {
       parts[0] = p0;
       parts[1] = p1;
       parts[2] = p2;
       parts[3] = p3;
       parts[4] = p4;
       parts[5] = p5;
   }

   TreeNode * do_parse(std::istream istr, char **update_op) { 
   TreeNode *res = new TreeNode();

      for( int i = 0 ; i < 6; i++ ) {
          if ( parts[i] != NULL ) {
              res->subexps[i] = parts[i]->do_parse(istr, &res->op);
          }
      }
   }
}

class Or : public ParserChunk {
   ParserChunk *parts[4];
public:

   Or( ParserChunk *p0, ParserChunk *p1 = &empty, ParserChunk *p2 = &empty, ParserChunk *p3 = &empty) {

       p[0] = p0;
       p[1] = p1;
       p[2] = p2;
       p[3] = p3;
   }
   TreeNode * do_parse(std::istream istr, char **update_op) { 
  
      for( int i = 0 ; i < 4; i++ ) {
          if ( parts[i] != NULL ) {
              res =  parts[i]->do_parse(istr, update_op);
              if (res) {
                  return res;
              }
          }
      }
      return 0;
   }
};

class Opt : Or {
public:
   Opt( char *p1, ParserChunk *p2 = &empty, ParserChunk *p3 = &empty, ParserChunk *p4 = &empty) {
       this.Or( Seq( Symbol(p1), p2, p3, p4), empty);
   }
};

//
// =====================================================
// 

void
buildParser() {
    ParserChunk *statement = new Or( 
          new Seq( new Symbol("if"), (guardlist), new Symbol("fi"), (opt_assertion)),
          new Seq( new Symbol("do"), (opt_invariant) , (opt_bound), (guardlist), new Symbol("od") , (opt_assertion)),
         (assignment)
      );

    ParserChunk *varlist = new Seq( new Symbol("\w+"), new Opt(",", &varlist));

}

ParserChunk *
primary = new Or( new Seq( new Symbol("-"), (primary)),
              Seq( Symbol("!"), (primary)),
              Symbol("\d+"),
              Symbol("0x\x+"),
              Seq( Symbol("\w+"), Or(
                    Seq( Symbol("["), (expression), Symbol("]"))
                    Seq( Symbol("("), (expression_list), Symbol(")"))
                    Seq( Symbol("."), (primary))
                    Empty())))
              
term = Seq((primary), Or(
                       Seq( Symbol("*"), (expression)),
                       Seq( Symbol("/"), (expression)),
                       Seq( Symbol("%"), (expression)),
                       Seq( Symbol("&"), (expression)),
                       Empty()))
expression = Seq((term), Or(
                       Seq( Symbol("+"), (expression)),
                       Seq( Symbol("-"), (expression)),
                       Seq( Symbol("|"), (expression)),
                       Empty()))

comp_pred = Seq((expression), Or(
                       Seq( Symbol("=="), (comp_pred)),
                       Seq( Symbol("!="), (comp_pred)),
                       Seq( Symbol(">="), (comp_pred)),
                       Seq( Symbol("<="), (comp_pred)),
                       Empty()))

or_pred = Seq((comp_pred), Opt("||", (or_pred)))

and_pred = Seq((or_pred), Opt("&&", (and_pred)))
predicate = Seq((and_pred),Or(
                       Seq( Symbol("==="), (predicate))
                       Seq( Symbol("!=="), (predicate))
                       Seq( Symbol("==>"), (predicate))
                       Seq( Symbol("<=="), (predicate))
                       Empty()))

opt_invariant = Opt("Inv:", (predicate) )
opt_bound = Opt("Bound:", (expression) )

stmt_seq = Seq( (statement), (optAssertion), Opt(";", (stmt))

assignemnt = Seq( (varlist), Symbol("="), (exprlist))

type =  Or(
    Seq(Symbol("array"), Symbol("["), (expression), Symbol("]"), (type))
    Seq(Symbol("class"), Symbol("["), (expression), Symbol("]"), (type))
 );

block = Opt("begin", stmt_seq, Symbol("end"));

opt_Assertion = Opt("{", (predicate), Symbol("}"));

spec_type = Seq(Symbol(":") , (type));
formal_arg_list = Opt("(", ( formal_list ), Symbol(")"));
formal_list = Seq((formal), Opt(",", (formal_list)));
formal = Seq(Symbol("\w+"), (spec_type));

func = Seq(Symbol("function"), Symbol("\w+"),  (formal_arg_list), (spec_type), Opt("pre:", predicate), Opt("post:", predicate) , (block));

*/

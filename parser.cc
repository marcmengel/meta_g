
class TreeNode {
public:
   char *op;
   TreeNode *subexps[6];
}

class ParserChunk {
public:
   int ptype;
   virtual TreeNode *do_parse(istream istr, char ***update_op) = 0;
}

class Empty : ParserChunk {
public:
   Empty() { ; }
   do_parse(istream istr, char ***update_op) { return (TreeNode *)0;}
}

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

    matcher(char *s) { start = cur = s; saw_backslash = 0; alt = 0; }
    clock(c) {
       if (cur[0] == 0) { //end of pattern, does not match :-)
          return 0;
       }
       if (alt && c == *alt) {
          // we match what came after the plus or star.. jump ahead
          // only works for ...a*b not ...a*a type matches
          cur = alt+1;
          alt = 0;
          return 1;
       } if (cur[0] == '\\') {
           if (cur[1] == 'w' && isalnum(c)) {
               if(cur[2] != '*' && cur[2] != '+') {
                  cur += 2;
               }
               return 1;
           }
           if (cur[1] == 's' && isspace(c)) {
               if(cur[2] != '*' && cur[2] != '+') {
                  cur += 2;
               }
               return 1;
           }
           if (cur[1] == c) {
              cur += 2;
              return 1;
           }
       }
       if (cur[0] == '[') {
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
          if (*cur == ']')
             cur++;
          }
          if (*cur && *cur == '*') {
             alt = cur+1;
          }
          return matched ^ cur;
       }
       if (cur[0] == c) {
          cur++;
          return 1;
       }
    }
}

class Symbol : ParserChunk {
public:
   char *sym_re;
   Symbol(char *re) { sym_re = re; }

   virtual TreeNode * do_parse(istream istr, char ***update_op) { 
      static const int MAXTOKEN=64;
      static char buf[MAXTOKEN];
      matcher m(sym_re);
      int c = istr.peek();
      int i = 0;
      while( m.clock(c) ) {
         buf[i++] = c;
         istr.get();
         c = istr.peek();
      }
      if (i > 0) {
         buf[i++] = 0;
         res = new TreeNode;
         res->op = res->subexps[0] = strdup(buf);
         if (update_op != 0 && *update_op == 0) {
             *update_op = res->op;
         }
      } else {
         return 0;
      }
   }
}

class Seq : ParserChunk {
public:
   ParserChunk *parts[6];

   Opt( char *p0, ParserChunk &p1 = empty, ParserChunk &p1 = empty, ParserChunk &p3 = empty, ParserChunk &p4 = empty,ParserChunk &p5 = empty) {

       p[0] = &p0;
       p[1] = &p1;
       p[2] = &p2;
       p[3] = &p3;
       p[4] = &p4;
       p[5] = &p5;
   }

   virtual TreeNode * do_parse(istream istr, char ***update_op) { 
      TreeNode *res = new TreeNode();

      for( int i = 0 ; i < 6; i++ ) {
          if ( parts[i] != NULL ) {
              res->subexps[i] = parts[i]->do_parse(istr, &res->op);
          }
      }
}

class Or : 
public:
   ParserChunk *parts[4];

   Opt( char *p0, ParserChunk &p1 = empty, ParserChunk &p1 = empty, ParserChunk &p3 = empty) {

       p[0] = &p0;
       p[1] = &p1;
       p[2] = &p2;
       p[3] = &p3;
   }
   virtual TreeNode * do_parse(istream istr, char ***update_op) { 
  
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
}

class Opt : Or {
public:
   Opt( char *p1, ParserChunk &p2 = empty, ParserChunk &p3 = empty, ParserChunk &p4 = empty) {
       this.Or( Seq( Symbol(p1), p2, p3, p4), empty);
   }
}

// ...

type =  Or(
    Seq(Symbol("array"), Symbol("["), (expression), Symbol("]"), (type))
    Seq(Symbol("class"), Symbol("["), (expression), Symbol("]"), (type))
 );

block = Opt("begin", stmt_seq, Symbol("end"));

spec_type = Seq(Symbol(":") , (type));
formal_arg_list = Opt("(", ( formal_list ), Symbol(")"));
formal_list = Seq((formal), Opt(",", (formal_list)));
formal = Seq(Symbol("\w+"), (spec_type));

func = Seq(Symbol("function"), Symbol("\w+"),  (formal_arg_list), (spec_type), Opt("pre:", predicate), Opt("post:", predicate) , (block));


opt_Assertion = Opt("{", (predicate), Symbol("}"));



statement = Or(
               Seq( Symbol("if"), (guardlist), Symbol("fi"), (opt_assertion)),
               Seq( Symbol("it"), (opt_invariant) , (opt_bound), (guardlist), Symbol("fi") , (opt_assertion)),
               (assignment)
            );

varlist = Seq( Symbol("\w+"), Opt(",", varlist));

stmt_seq = Seq( (statement), (optAssertion), Opt(";", (stmt))

assignemnt = Seq( (varlist), Symbol("="), (exprlist))



primary = Or( Seq( Symbol("-"), (primary)),
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

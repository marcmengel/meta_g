#include "tokens.h"
#include <iostream>
#include <ctype.h>

#ifdef DEBUG
static int _debug = 1;
#else
static int _debug = 0;
#endif

/* Symbols split out with state numbers:
    ===========================
    < 1 = 2
    < 1 = 2 = 3
    = 4
    = 4 = 5
    = 4 = 5 = 6
    = 4 = 5 > 7
    > 8 = 9 
    | 10
    | 10 | 71
    - 11
    - 11 > 12
    : 13
    ; 76 ; 77
    ! 14
    ! 14 = 15
    ! 14 = 15 = 16
    / 17
    . 18
    ( 19
    ) 20
    [ 21
    ] 22
    { 23
    } 24
    * 25
    & 26
    & 26 & 72
    % 27
    + 28
    0 29 x 30 \\x 31 (*)
     \\d 46
    a 32 r 33 r 34 a 35 y 36
      \\w  62
    b 37 e 38 g 38 i 40 n 41
      \\w  62
    c 42 l 43 a 44 s 45 s 74
    \\d 46 (+)
    \\d 46 . 69 \\d 70 (+)
    d 47 o 48
       \\w  62
    e 49 n 50 d 51
        \\w  62 
    f 52 i 53
        \\w  62
    f 52 u 54 n 55 c 56 t 57 i 58 o 59 n 60
        \\w  62
    i 61 f 75
        \\w  62
    i 61 n 78
        \\w  62
    o 72 d 73
       \\w  62
    \\w 62 (+)
    r 63 e 64 t 65 u 66 r 67 n 68
    ===========================
*/

struct graphrow {
  int final_sym;
  const char *trans;
  int nss[35];
} scanner_graph[78] = {
    //0
    { -1, "<=>|-:;!/.()[]{}*&%+0abcDdefioWr\342", { 1,4,8,10,11,13,76,14,16,18,19,20,21,22,23,24,25,26,27,28,29,32,37,42,46,47,49,52,61,72,62,63,79}},
    //1
    {'<',"=",{2}},
    //2
    {T_LEQ, "=", {3}},
    //3
    {T_IMPLBY, "", {}},
    //4
    {'=', "=", {5}},
    //5
    {T_EQUAL, "=>", {6, 7}},
    //6
    {T_EQUIV, "", {}},
    //7
    {T_IMPL, "", {}},
    //8
    {'>', "=", {9}},
    //9
    {T_GEQ, "", {}},
    //10
    {'|', "|", {71}},
    //11
    {'-', ">", {12}},
    //12
    {T_ARROW, "", {}},
    //13
    {':', "", {}},
    //14
    {'!', "=", {15}},
    //15
    {T_NEQ, "=", {16}},
    //16
    {T_NEQUIV, "",{}},
    //17
    {'/', "", {}},
    //18
    {'.', "", {}},
    //19
    {'(', "", {}},
    //20
    {')', "", {}},
    //21
    {'[', "", {}},
    //22
    {']', "", {}},
    //23
    {'{', "", {}},
    //24
    {'}', "", {}},
    //25
    {'*', "", {}},
    //26
    {'&', "&", {72,}},
    //27
    {'%', "", {}},
    //28
    {'+', "", {}},
    //29
    {T_NUM, "xD", {30, 46}},
    //30
    {-1, "X", {31,}},
    //31 
    {T_NUM, "X", {31,}},
    //32
    {T_NAME, "rW", {33,62}},
    //33
    {T_NAME, "rW", {34,62}},
    //34
    {T_NAME, "aW", {35,62}},
    //35
    {T_NAME, "yW", {36,62}},
    //36
    {T_ARRAY, "W", {62,}},
    //37
    {T_NAME, "eW", {38, 62}},
    //38
    {T_NAME, "gW", {39, 62}},
    //39
    {T_NAME, "iW", {40, 62}},
    //40
    {T_NAME, "nW", {41, 62}},
    //41
    {T_BEGIN, "", {}},
    //42
    {T_NAME, "lW", {43, 62}},
    //43
    {T_NAME, "aW", {44, 62}},
    //44
    {T_NAME, "sW", {45, 62}},
    //45
    {T_NAME, "sW", {74, 62}},
    //46
    {T_NUMBER, "D.", {46, 69}},
    //47
    {T_NAME, "oW", {48, 62}},
    //48
    {T_DO,  "W", {62}},
    //49
    {T_NAME, "nW", {50, 62}},
    //50
    {T_NAME, "dW", {51, 62}},
    //51
    {T_END, "W", {62}},
    //52
    {T_NAME, "iuW", {53, 54, 62}},
    //53
    {T_FI, "W", {62}},
    //54
    {T_NAME, "nW", {55, 62}},
    //55
    {T_NAME, "cW", {56, 62}},
    //56
    {T_NAME, "tW", {57, 62}},
    //57
    {T_NAME, "iW", {58, 62}},
    //58
    {T_NAME, "oW", {59, 62}},
    //59
    {T_NAME, "nW", {60, 62}},
    //60
    {T_FUNCTION, "W", {62}},
    //61
    {T_NAME, "fnW", {75, 78, 62}},
    //62
    {T_NAME, "W", {62}},
    //63
    {T_NAME, "eW", {64, 62}},
    //64
    {T_NAME, "tW", {65, 62}},
    //65
    {T_NAME, "uW", {66, 62}},
    //66
    {T_NAME, "rW", {67, 62}},
    //67
    {T_NAME, "nW", {68, 62}},
    //68
    {T_RETURN, "W", {62}},
    //69
    {T_NUMBER, "D", {70}},
    //70
    {T_NUMBER, "D", {70}},
    //71
    {T_LOR, "",{}},
    //72
    {T_NAME, "dW", {73,62}},
    //73
    {T_OD, "W", {62}},
    //74
    {T_CLASS, "W", {62}},
    //75
    {T_IF, "W", {62}},
    //76
    {';', ";", {77}},
    //77
    {T_ENDGUARD, "", {}},
    //78
    {T_IN, "W", {62}},
    //79 utf8 symbols...
    {-1, "\206\210\211\226", {80, 81, 82, 83}},
    //80 utf8 symbols...
    {-1, "\222", {12}},
    //81 utf8 symbols...
    {-1, "\203\212\247\250", {84,78,72,71}},
    //82 utf8 symbols...
    {-1, "\240\244\245\241\242" {15,2,9,6,16}},
    //83 utf8 symbols...
    {-1, "\257", {77}},
    //84 utf8 symbols
    {T_EXISTS, "", {}},
    //86 
};

#define MAXTOKEN 128

char yytext[MAXTOKEN];

int nextstate(int curstate, int c) {
    for(int i = 0; scanner_graph[curstate].trans[i] != 0 ; i++) {
       switch( scanner_graph[curstate].trans[i] ){
       case 'W': 
           if (isalnum(c))     
               return scanner_graph[curstate].nss[i]; 
           break;
       case 'D': 
           if (isdigit(c))     
               return scanner_graph[curstate].nss[i]; 
           break;
       case 'X': 
           if (isxdigit(c)) 
               return scanner_graph[curstate].nss[i]; 
           break;
       default:  
           if (c == scanner_graph[curstate].trans[i])
               return scanner_graph[curstate].nss[i]; 
           break;
       }
   }
   return -1;
}

void
syntax_error(int curline, int curcol, int c){
        std::cerr << "Syntax error at line " << curline << " column " << curcol <<  ": unexpected character '"<< (char)c << "' ";
}

int
gettoken(std::istream  &s) {
    static int curline = 1;
    static int curcol = 0;
    int curstate = 0;
    int curchar = 0;
    int c;
    int ns;
    c = s.get();
    curcol++;
    while( isspace(c) ) {
       _debug && std::cout << "skipping whitespace :'" << (char)c << "'\n";
       if ('\n' == c) {
          curline++;
          curcol = 0;
       }
       c = s.get();
       curcol++;
    }
    ns = nextstate(curstate, c);
    _debug && std::cout << "char '" << (char)c << "' takes us to state " << ns << "\n";
    while (ns != -1) {
        curstate = ns;
        yytext[curchar++] = c;
        curcol++;
        c = s.get();
        ns = nextstate(curstate, c);
        _debug && std::cout << "char '" << (char)c << "' takes us to state " << ns << "\n";
    }

    // if the character is an error, don't push it back
    // just complain about it.  Otherwise we would keep
    // getting it back next time...
    //
    if (scanner_graph[curstate].final_sym != -1 ) {
        s.unget();
        curcol--;
        yytext[curchar] = 0;
        return scanner_graph[curstate].final_sym;
    } else if (c != -1 ) {
        syntax_error(curline,curcol, c);
    }
}

#ifdef UNITTEST
main() {
   int t;
   while( !std::cin.eof() ) {
       t = gettoken(std::cin);
       _debug && std::cout << "\nGot token: " << t << " " << tokenstr(t) << ": '" << yytext << "'\n";
   }
}
#endif

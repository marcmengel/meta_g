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
    v 102 a 103 r 104
    ===========================
*/

struct graphrow {
  int final_sym;
  const char *trans;
  int nss[40];
} scanner_graph[106] = {
    //0
    { -1, ",<=>|-:;!/.()[]{}*&%+0abcDdefiopWrv\342", { 105,1,4,8,10,11,13,76,14,16,18,19,20,21,22,23,24,25,26,27,28,29,32,37,42,46,47,49,52,61,72,86,62,63,102,79}},
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
    {'&', "&", {99,}},
    //27
    {'%', "", {}},
    //28
    {'+', "", {}},
    //29
    {T_NUMBER, "xD", {30, 46}},
    //30
    {-1, "X", {31,}},
    //31 
    {T_NUMBER, "X", {31,}},
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
    {T_NAME, "eoW", {38, 94, 62}},
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
    {T_IN, "vW", {100,62}},
    //79 utf8 symbols...
    {-1, "\206\210\211\226", {80, 81, 82, 83}},
    //80 utf8 symbols...
    {-1, "\222", {12}},
    //81 utf8 symbols...
    {-1, "\203\212\247\250", {84,78,99,71}},
    //82 utf8 symbols...
    {-1, "\240\244\245\241\242", {15,2,9,6,16}},
    //83 utf8 symbols...
    {-1, "\257", {77}},
    //84 utf8 symbols
    {T_EXISTS, "", {}},
    //85 -- unused slot...
    {-1, "", {}},
    //86 
    {T_NAME, "roW", {87,88,62}},
    //87 
    {T_NAME, "eW", {89,62}},
    //88 
    {T_NAME, "sW", {90,62}},
    //89 
    {T_NAME, ":W", {91,62}},
    //90 
    {T_NAME, "tW", {92,62}},
    //91 
    {T_PRE, "", {}},
    //92
    {T_NAME, ":W", {93,62}},
    //93
    {T_POST, "", {}},
    //94
    {T_NAME, "uW", {95,62}},
    //95
    {T_NAME, "nW", {96,62}},
    //96
    {T_NAME, "dW", {97,62}},
    //97
    {T_NAME, ":W", {98,62}},
    //98
    {T_POST, "", {}},
    //99
    {T_LAND, "", {}},
    //100
    {T_NAME, ":W",{101,62}},
    //101
    {T_INV, "",{}},
    //102
    {T_NAME, "aW", {103,62}},
    //103
    {T_NAME, "rW", {104,62}},
    //104
    {T_VAR, "W", {62}},
    //105
    {',', "", {}},
};

#define MAXTOKEN 128

unsigned char yytext[MAXTOKEN];

int nextstate(int curstate, int c) {
    for(int i = 0; scanner_graph[curstate].trans[i] != 0 ; i++) {
       switch( scanner_graph[curstate].trans[i] ) {
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
           if ((unsigned char)c == (unsigned char)scanner_graph[curstate].trans[i])
               return scanner_graph[curstate].nss[i]; 
           break;
       }
   }
   return -1;
}

int yyline, yycol, yychar;

void
syntax_error(const char *msg){
    std::cout.flush();
    std::cerr.flush();
    std::cerr << "Syntax error at line " << yyline << " column " << yycol <<  ": " << msg << " at token " << yytext << " at char '"<< (char)yychar << "' \\"<< std::oct <<  yychar << "\n";
    std::cerr.flush();
}

int
gettoken(std::istream  &s) {
    int curstate = 0;
    int curchar = 0;
    int c;
    int ns;
    c = s.get(); yycol++;
    while( c < 128 && isspace(c) ) {
       _debug && std::cout << "skipping whitespace :'" << (unsigned char)c << "'\n";
       if ('\n' == c) {
          _debug && std::cout << "currrent line: " << yyline << "\n";
          yyline++;
          yycol = 0;
       }
       c = s.get(); yycol++;
    }
    ns = nextstate(curstate, c);
    _debug && (c > 31 && c < 128 && !isspace(c)) &&  std::cout << "char '" << (char)c << "' takes us to state " << ns << "\n";
    _debug && (c < 32 || c > 127 || isspace(c)) &&  std::cout << "char '\\" << std::oct << c << "' takes us to state " << ns << "\n";
    while (ns != -1) {
        curstate = ns;
        yytext[curchar++] = c;
        c = s.get(); yycol++;
        ns = nextstate(curstate, c);
        _debug && std::cout << "char '" << (char)c << "' takes us to state " << ns << "\n";
    }

    // if the character is an error, don't push it back
    // just complain about it.  Otherwise we would keep
    // getting it back next time...
    //
    if (scanner_graph[curstate].final_sym != -1 ) {
        s.unget(); yycol--;
        yytext[curchar] = 0;
        return scanner_graph[curstate].final_sym;
    } else if (c != -1 ) {
        yychar = c;
        syntax_error("unexpected character");
    }
}

#ifdef UNITTEST
main() {
   int t;
   while( std::cin.good() ) {
       t = gettoken(std::cin);
       _debug && std::cout << "\nGot token: " << t << " " << tokenstr(t) << ": '" << yytext << "'\n";
   }
}
#endif

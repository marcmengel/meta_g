#!/bin/sh

(
    mindef=`grep T_ tokens.h | head -1 | sed -e 's/ *$//' -e 's/.* //'`
    echo "const char *tokenmap[] = {"
    cat tokens.h |
       sed -e 's/#define //' -e 's/ [0-9]*//' -e 's/.*/"&",/'
    echo "};"

    cat << EOF

const char *
tokenstr(int t) {
    static char buf[2];

    if (t < $mindef) {
         buf[0] = t;
         return buf;
    } else {
         return tokenmap[t - $mindef];
    }
}

EOF

) > tokenmap.cc

#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="grapa"

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

which cafe-autogen || {
    echo "You need to install cafe-common"
    exit 1
}

which yelp-build || {
    echo "You need to install yelp-tools" 
    exit 1
}

REQUIRED_AUTOMAKE_VERSION=1.10
REQUIRED_CTK_DOC_VERSION=1.13
USE_CAFE2_MACROS=1

. cafe-autogen

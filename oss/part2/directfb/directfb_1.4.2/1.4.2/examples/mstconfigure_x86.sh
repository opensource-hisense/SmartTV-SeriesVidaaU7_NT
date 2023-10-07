

test -z $MST_PREFIX && echo "  The MST_PREFIX must be set to proceed!!" && exit 0

echo "Running autoconf & automake"
autoreconf
autoconf
automake

echo "MST_PREFIX=$MST_PREFIX" 

./configure --prefix=$MST_PREFIX/test \
            CFLAGS="-I$MST_PREFIX/include" \
            LDFLAGS="-L$MST_PREFIX/lib"

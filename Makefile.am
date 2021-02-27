ACLOCAL_AMFLAGS = -I m4
lib_LTLIBRARIES = libjstools.la
AC_CONFIG_AUX_DIR = config
include_HEADERS = jstools.h
noinst_includedir = src
noinst_include_HEADERS = src/JST_list.h src/JST_pairs_compare.h src/JST_string.h
libjstools_la_SOURCES =\
 src/JST_delete.c\
 src/JST_get.c\
 src/JST_list.c\
 src/JST_load.c\
 src/JST_pairs_compare.c\
 src/JST_save_xml.c\
 src/JST_save.c\
 src/JST_serialize.c\
 src/JST_string.c\
 src/JST_update.c\
 src/JST_walk.c
libjstools_la_CFLAGS = -g0 -O3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion -Wcast-align -Wcast-qual\
 -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wsign-conversion -Wswitch-default -Wundef\
 -Wwrite-strings -Wfloat-equal -c -fmessage-length=0 -fPIC -fstack-protector-all -Wformat=2 -Wformat-security -Wstrict-overflow
check_PROGRAMS       = jstoolstests
check_DATA           = data/boolean.json    data/integer.json    data/double.json    data/string.json\
                       data/object.json     data/array.json\
                       data/error1.json\
                       data/pass1.json      data/pass2.json      data/pass3.json     data/pass4.json     data/pass5.json\
                       data/fail1.json      data/fail2.json      data/fail3.json     data/fail4.json     data/fail5.json\
                       data/fail6.json      data/fail7.json      data/fail8.json     data/fail9.json     data/fail10.json\
                       data/fail11.json     data/fail12.json     data/fail13.json    data/fail14.json    data/fail15.json\
                       data/fail16.json     data/fail17.json     data/fail18.json    data/fail19.json    data/fail20.json\
                       data/fail21.json     data/fail22.json     data/fail23.json    data/fail24.json    data/fail25.json\
                       data/fail26.json     data/fail27.json     data/fail28.json    data/fail29.json    data/fail30.json\
                       data/fail31.json     data/fail32.json     data/fail33.json
check_SCRIPTS        = test-scripts/check-with-valgrind\
                       test-scripts/boolean test-scripts/integer test-scripts/double test-scripts/string\
                       test-scripts/object  test-scripts/array\
                       test-scripts/error1  test-scripts/ce-fichier-n-existe-pas\
                       test-scripts/pass1   test-scripts/pass2   test-scripts/pass3  test-scripts/pass4  test-scripts/pass5\
                       test-scripts/fail1   test-scripts/fail2   test-scripts/fail3  test-scripts/fail4  test-scripts/fail5\
                       test-scripts/fail6   test-scripts/fail7   test-scripts/fail8  test-scripts/fail9  test-scripts/fail10\
                       test-scripts/fail11  test-scripts/fail12  test-scripts/fail13 test-scripts/fail14 test-scripts/fail15\
                       test-scripts/fail16  test-scripts/fail17  test-scripts/fail18 test-scripts/fail19 test-scripts/fail20\
                       test-scripts/fail21  test-scripts/fail22  test-scripts/fail23 test-scripts/fail24 test-scripts/fail25\
                       test-scripts/fail26  test-scripts/fail27  test-scripts/fail28 test-scripts/fail29 test-scripts/fail30\
                       test-scripts/fail31  test-scripts/fail32  test-scripts/fail33
jstoolstests_SOURCES = test/main.c
jstoolstests_CFLAGS  = -g3 -O0
jstoolstests_LDFLAGS = -L.libs -ljstools
TESTS                = test-scripts/check-with-valgrind\
                       test-scripts/boolean test-scripts/integer test-scripts/double test-scripts/string\
                       test-scripts/object  test-scripts/array\
                       test-scripts/error1  test-scripts/ce-fichier-n-existe-pas\
                       test-scripts/pass1   test-scripts/pass2   test-scripts/pass3  test-scripts/pass4  test-scripts/pass5\
                       test-scripts/fail1   test-scripts/fail2   test-scripts/fail3  test-scripts/fail4  test-scripts/fail5\
                       test-scripts/fail6   test-scripts/fail7   test-scripts/fail8  test-scripts/fail9  test-scripts/fail10\
                       test-scripts/fail11  test-scripts/fail12  test-scripts/fail13 test-scripts/fail14 test-scripts/fail15\
                       test-scripts/fail16  test-scripts/fail17  test-scripts/fail18 test-scripts/fail19 test-scripts/fail20\
                       test-scripts/fail21  test-scripts/fail22  test-scripts/fail23 test-scripts/fail24 test-scripts/fail25\
                       test-scripts/fail26  test-scripts/fail27  test-scripts/fail28 test-scripts/fail29 test-scripts/fail30\
                       test-scripts/fail31  test-scripts/fail32  test-scripts/fail33
XFAIL_TESTS          = test-scripts/error1  test-scripts/ce-fichier-n-existe-pas\
                                            test-scripts/fail2   test-scripts/fail3  test-scripts/fail4  test-scripts/fail5\
                       test-scripts/fail6   test-scripts/fail7   test-scripts/fail8  test-scripts/fail9  test-scripts/fail10\
                       test-scripts/fail11  test-scripts/fail12  test-scripts/fail13 test-scripts/fail14 test-scripts/fail15\
                       test-scripts/fail16  test-scripts/fail17                      test-scripts/fail19 test-scripts/fail20\
                       test-scripts/fail21  test-scripts/fail22  test-scripts/fail23 test-scripts/fail24 test-scripts/fail25\
                       test-scripts/fail26  test-scripts/fail27  test-scripts/fail28 test-scripts/fail29 test-scripts/fail30\
                       test-scripts/fail31  test-scripts/fail32  test-scripts/fail33

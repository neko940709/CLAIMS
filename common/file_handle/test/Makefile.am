AM_CPPFLAGS= -fPIC -fpermissive -DTHERON_XS\
-I${HADOOP_HOME}/src/c++/libhdfs\
-I${JAVA_HOME}/include\
-I${JAVA_HOME}/include/linux \
-I${GTEST_HOME}/include

AM_LDFLAGS=-lc -lm -lrt -lconfig++ -lpthread -lxs

if OPT_TCMALLOC
AM_CPPFLAGS+=-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
AM_LDFLAGS+=-ltcmalloc 
endif

LDADD = ../libcommon.a 


noinst_LIBRARIES=libtest.a

libtest_a_SOURCES = 	\
	disk_file_handle_imp_test.h		hdfs_file_handle_imp_test.h 


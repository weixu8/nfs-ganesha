AM_CFLAGS                     = $(FSAL_CFLAGS) $(SEC_CFLAGS)


noinst_LTLIBRARIES          = libmfslasync2.la

libmfslasync2_la_SOURCES = mfsl_async2.c            \
                           mfsl_async2_init.c       \
                           mfsl_async2_read_conf.c  \
                           mfsl_async2_dispatcher.c \
                           mfsl_async2_filler.c     \
                           mfsl_async2_synclet.c    \
                           mfsl_async2_unlink.c     \
                           mfsl_async2_link.c       \
                           mfsl_async2_rename.c     \
                           mfsl_async2_setattrs.c   \
                           mfsl_async2_mkdir.c		\
                           mfsl_async2_create.c

libmfslasync2_la_LIBADD = $(FSAL_LIB)

#check_PROGRAMS              = test_mfsl_template
#test_mfsl_template_SOURCES  = test_fsal.c 
#test_mfsl_template_LDADD    = $(FSAL_LIB) $(FSAL_LDFLAGS) -lpthread


new: clean all

doc:
	doxygen ./doxygen.conf
	rep=`grep OUTPUT_DIRECTORY doxygen.conf | grep share  | awk -F '=' '{print $$2;}'` ; cd $$rep/latex ; make


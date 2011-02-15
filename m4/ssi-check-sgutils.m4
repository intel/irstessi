AC_DEFUN([SSI_CHECK_SGUTILS], [
	SGUTILS_LDFLAGS=NONE
	AC_CHECK_LIB(sgutils2, sg_ll_inquiry,
	        [SGUTILS_LDFLAGS=-lsgutils2], [])
	if test "x$SGUTILS_LDFLAGS" = "xNONE"; then
	    AC_CHECK_LIB(sgutils, sg_ll_inquiry,
	            [SGUTILS_LDFLAGS=-lsgutils], [])
	fi
	if test "x$SGUTILS_LDFLAGS" = "xNONE"; then
	    AC_MSG_ERROR([**Error**: sg3_utils library is required to build this package!])
	fi
	AC_SUBST(SGUTILS_LDFLAGS)
])

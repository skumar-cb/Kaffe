# Makefile.frag for kaffevm Interpreter
#
# Copyright (c) 1996, 1997
#	Transvirtual Technologies, Inc.  All rights reserved.
#
# See the file "license.terms" for information on usage and redistribution 
# of this file. 

ENGINE_CFLAGS=	-DINTERPRETER -I$(srcdir)/intrp  

ENGINE_OBJECTS= machine-intrp$(OBJEXT)

machine.o:	machine-intrp.c kaffe.intrp.def

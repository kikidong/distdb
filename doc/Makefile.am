

	
html-local:clean-local Doxyfile
	doxygen Doxyfile

#Doxyfile:Doxyfile.in
#	$(top_builddir)/config.status $@	
	
clean-local:
	${RM} -r html latex
	
all-am:	html
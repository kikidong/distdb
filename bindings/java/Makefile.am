

distdb.h : distdb.class	
	${JAVAH} -classpath .  distdb
	@touch $@

distdb.class: distdb.java
	${JAVAC} $(srcdir)/distdb.java -d .

all-am: distdb.class distdb.h
 
clean-local:
	${RM} distdb.class distdb.h	

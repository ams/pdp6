DC==200
UTC==210
UTS==214

DACI==04010

SL==220000
RD==300

LOC	0
ENTRY:
	CONO	DC,DACI
	CONO	UTC,SL+RD
	MOVE	17,[ IOWD 200,300 ]
	CONSO	DC,1000
	 JRST	.-1
	BLKI	DC,17
	 JRST	DONE
	JRST	.-4
DONE:	CONO	DC,0
	HALT	300

END ENTRY

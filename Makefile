contact_driver: contact_driver.o contact.o pds.o
	gcc -o contact_driver contact_driver.o contact.o pds.o

contact_driver.o: contact_driver.c
	gcc -c contact_driver.c

contact.o: contact.c contact.h
	gcc -c contact.c

pds.o: pds.c pds.h
	gcc -c pds.c

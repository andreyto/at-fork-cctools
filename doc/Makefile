include ../config.mk

SOURCES=${shell ls man/*.m4}

HTMLFILES = ${SOURCES:%.m4=%.html}
TXTFILES = ${SOURCES:%.m4=%.txt}
PDFFILES = ${SOURCES:%.m4=%.pdf}
MANFILES = ${SOURCES:%.m4=%.1}
ZIPFILES = ${SOURCES:%.m4=%.1.gz}

all: ${CCTOOLS_DOCTARGETS}

htmlpages: ${HTMLFILES}

manpages: ${ZIPFILES}

apipages: api/html/index.html api/html/work_queue_perl.html api/html/work_queue_task_perl.html

api/html/index.html:
	mkdir -p api/html
	doxygen cctools.doxygen.config

api/html/work_queue_perl.html:
	mkdir -p api/html
	pod2html ../work_queue/src/perl/Work_Queue.pm > $@
	@rm -f pod2htmd.tmp

api/html/work_queue_task_perl.html:
	mkdir -p api/html
	pod2html ../work_queue/src/perl/Work_Queue/Task.pm > $@
	@rm -f pod2htmd.tmp

%.html: %.m4
	m4 -DHTML ${CCTOOLS_M4_ARGS} $< > $@

%.1: %.m4
	m4 -DMAN ${CCTOOLS_M4_ARGS} $< > $@

%.txt: %.1
	nroff -man $< > $@

%.gz: %
	gzip < $< > $@

install: all
	mkdir -p $(CCTOOLS_INSTALL_DIR)/doc
	mkdir -p $(CCTOOLS_INSTALL_DIR)/doc/man
	mkdir -p $(CCTOOLS_INSTALL_DIR)/doc/images
	mkdir -p $(CCTOOLS_INSTALL_DIR)/share/man/man1
	cp *.html manual.css $(CCTOOLS_INSTALL_DIR)/doc
	if [ -f man/chirp.html ]; then cp man/*.html $(CCTOOLS_INSTALL_DIR)/doc/man; fi
	if [ -f man/chirp.1.gz ]; then cp man/*.1.gz $(CCTOOLS_INSTALL_DIR)/share/man/man1; fi
	if [ -d api ]; then cp -rp api $(CCTOOLS_INSTALL_DIR)/doc; fi
	if [ -d images ]; then cp images/* $(CCTOOLS_INSTALL_DIR)/doc/images; fi

test:

clean:
	rm -rf api $(HTMLFILES) $(MANFILES) $(ZIPFILES) *~

.PHONY: all clean install test

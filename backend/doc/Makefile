all:
	@echo generation htmlhelp
	@#get rid of garbled lines in window section
	@sed -i 's/WINDOWS/x/' html/index.hhp
	@cd html; ../../../3rdparty/chmcmd index.hhp 2>&1 >/dev/null
	@chmod 0644 html/TimeTagger-1.0-api.chm
	@mv html/TimeTagger-1.0-api.chm TimeTagger-1.0-api.chm
	@rm -f html/index.hh*
	
	@echo creating pdf manual
	@make -C latex >/dev/null 2>&1 >/dev/null
	@mv latex/refman.pdf TimeTagger-1.0-api.pdf
	
clean:
	rm -Rf html latex *.pdf *.chm
	
dist-clean: clean


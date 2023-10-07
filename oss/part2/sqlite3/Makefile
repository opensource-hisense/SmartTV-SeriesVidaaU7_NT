.PHONY: all install clean

all install clean: 
	make -C ./amalgamation_3_40_0 $@
	
notice:
	cp -rf NOTICE.txt $(OSS_NOTICE_OUTPUT)/sqlite3_NOTICE.txt
	
info:
	cp -rf OSS_INFO.txt $(OSS_INFO_OUTPUT)/sqlite3_INFO.txt

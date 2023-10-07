include openbox_inc.mak


#all: bash coreutils findutils gawk grep gzip inetutils module_init_tools ncurses procps sed tar util-linux net-tools iputils udhcp #psmisc
all: bash 

bash:                                     
	cd ${BASH_SRC_PATH}; \
	tar -zxvf bash-3.2.57.tar.gz; \
	cd bash-3.2.57; \
	patch -p0 < ../bash-3.2-patches/bash43-048-mtk ; \
	patch -p0 < ../bash-3.2-patches/bash44-006-mtk ; \
	patch -p2 < ../bash-3.2-patches/bash32-mtk-patch-001 ; \
	patch -p2 < ../bash-3.2-patches/bash-mtk-builtins_Makefile.in-patch 

coreutils:
	cd ${COREUTIL_SRC_PATH}; \
	tar -zxvf coreutils-6.9.tar.gz; \
	cp -rf coreutils-6.9_patch/* coreutils-6.9/
	
findutils:
	cd ${FINDUTIL_SRC_PATH}; \
	tar -zxvf findutils-4.2.31.tar.gz

gawk:
	cd ${GAWK_SRC_PATH}; \
	tar -zxvf gawk-3.1.5.tar.gz

grep:
	cd ${GREP_SRC_PATH}; \
	tar -zxvf grep-2.5.1a.tar.gz; \
	cp -rf grep-2.5.1a_patch/* grep-2.5.1a/
	
	
gzip:
	cd ${GZIP_SRC_PATH}; \
	tar -zxvf gzip-1.3.12.tar.gz; \
	patch -d gzip-1.3.12 -re -p1 < ./gzip-1.3.12_patch/gzip.mtk.patch ; \
	patch -d gzip-1.3.12 -re -p1 < ./gzip-1.3.12_patch/CVE-2010-0001.patch
	
inetutils:
	cd ${INETUTIL_SRC_PATH}; \
	tar -zxvf inetutils-1.4.2.tar.gz; \
	cp -rf inetutils-1.4.2_patch/* inetutils-1.4.2/
	
module_init_tools:
	cd ${MODULE_INIT_SRC_PATH}; \
	tar -zxvf module-init-tools-3.12.tar.gz; \
	if [ -d module-init-tools-3.12_patch ]; then \
		cp -rf module-init-tools-3.12_patch/* module-init-tools-3.12/; \
	fi
	
ncurses:
	cd ${NCURSES_SRC_PATH}; \
	tar -zxvf ncurses-5.7.tar.gz
	
procps:
	if [ "$(PROCPS_VER)" == "procps-3.2.8"  ]; then \
		echo PROCPS_VER=procps-3.2.8; \
		cd ${PROCPS_SRC_PATH}; \
		tar -zxvf procps-3.2.8.tar.gz; \
		cp -rf procps-3.2.8_patch/* procps-3.2.8/;\
	fi
	
	if [ "$(PROCPS_VER)" == "procps-ng-3.3.15" ]; then \
		echo PROCPS_VER=procps-ng-3.3.15;\
		cd ${PROCPS_SRC_PATH}; \
		xz -d -k -f procps-ng-3.3.15.tar.xz; \
		tar -xvf procps-ng-3.3.15.tar; \
	fi
	
psmisc:
	cd ${PSMISC_SRC_PATH}; \
	tar -zxvf psmisc-22.13.tar.gz

sed:                                     
	cd ${SED_SRC_PATH}; \
	tar -zxvf sed-4.1.5.tar.gz; \
	cp -rf sed-4.1.5_patch/* sed-4.1.5/
	
tar:                                     
	cd ${TAR_SRC_PATH}; \
	tar -zxvf tar-1.17.tar.gz; \
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/tar-1.17.mtk.patch; \
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2010-0624-rtapelib-overflow.patch; \
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2016-6321.patch; \
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2018-20482.patch; \
	patch -d tar-1.17 -re -p1 < ./tar-1.17_patch/CVE-2019-9923.patch

util-linux-ng:
	cd ${UTIL_LINUX_NG_SRC_PATH}; \
	tar -zxvf util-linux-ng-2.18.tar.gz; \
	cp -rf util-linux-ng-2.18_patch/* util-linux-ng-2.18/
	
util-linux:
	cd ${UTIL_LINUX_SRC_PATH}; \
	tar -zxvf util-linux-2.28.2.tar.gz; \
	cp -rf util-linux-2.28.2_patch/* util-linux-2.28.2/

net-tools:                                     
	cd ${NET_TOOLS_SRC_PATH}; \
	tar -zxvf net-tools-1.60.tar.gz; \
	cp -rf net-tools-1.60_patch/* net-tools-1.60/

iputils:                                     
	cd ${IPUTILS_SRC_PATH}; \
	tar -zxvf iputils-s20101006.tar.gz; \
	cp -rf iputils-s20101006_patch/* iputils-s20101006/

udhcp:
	cd ${UDHCP_SRC_PATH}; \
	tar -zxvf udhcp-install.tar.gz; \
	patch -d  v0.0 -re -p1 < v0.0_patch/udhcp.makefile.patch; \
	patch -d  v0.0 -re -p1 < v0.0_patch/udhcpc.v0.0_CVE-2011-2716.patch


clean:
	rm -rf $(BASH_SRC_BUILD_PATH)
	rm -rf $(COREUTIL_SRC_BUILD_PATH)
	rm -rf $(FINDUTIL_SRC_BUILD_PATH)
	rm -rf $(GAWK_SRC_BUILD_PATH)
	rm -rf $(GREP_SRC_BUILD_PATH)
	rm -rf $(GZIP_SRC_BUILD_PATH)
	rm -rf $(INETUTIL_SRC_BUILD_PATH)
	rm -rf $(MODULE_INIT_SRC_BUILD_PATH)
	rm -rf $(NCURSES_SRC_BUILD_PATH)
	rm -rf $(PROCPS_SRC_BUILD_PATH)
	rm -rf $(PSMISC_SRC_BUILD_PATH)
	rm -rf $(SED_SRC_BUILD_PATH)
	rm -rf $(TAR_SRC_BUILD_PATH)
	rm -rf $(UTIL_LINUX_NG_SRC_BUILD_PATH)
	rm -rf $(NET_TOOLS_SRC_BUILD_PATH)
	rm -rf $(IPUTILS_SRC_BUILD_PATH)
	rm -rf $(UDHCP_SRC_BUILD_PATH)


The makefile is default for toolchain 5.5.0 and NCT4.
We can set NCT5_ENABLE=true if branch uses NCT5 or toolchain=10.2.1 if branch has 10.2.1 toolchain
Example steps to update version:
1.Change version in oss_version.mak under oss\source\mak\
2.Put new tar in oss\source\libxml2\
3.Execute dockerq make NCT5_ENABLE=true
4.Execute dockerq make NCT5_ENABLE=true install
5.Execute dockerq make NCT5_ENABLE=true clean

If the upgrade of libxml2 fails, please see the following
During the upgrade process, the version of automake does not match. the solution is as follows:
1. Find the corresponding IT personnel to upgrade the automake version
2. If you upgrade the version of automake, you encounter an error "Couldn't find pkg. m4 from pkg config" during the build process.
The problem is that most. m4 files are in the/usr/share/aclcal/directory, but in fact, the default aclocal path of configure is /usr/local/share/acloca,
so you need to copy the *. m4 files under /usr/share/aclocal/to the usr/local/share/aclocal/directory
3. If the version of automake is still reported to be mismatched after upgrading, please remove dockerq and use the make command directly
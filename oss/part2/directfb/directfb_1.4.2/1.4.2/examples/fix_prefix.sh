#!/bin/sh

perl -pi -e 's|^datarootdir = \${prefix}|datarootdir = ..|' ./src/df_neo/Makefile
perl -pi -e 's|^datarootdir = \${prefix}|datarootdir = ..|' ./src/spacedream/Makefile
perl -pi -e 's|^datarootdir = \${prefix}|datarootdir = ..|' ./src/df_knuckles/Makefile
perl -pi -e 's|^datarootdir = \${prefix}|datarootdir = ..|' ./src/Makefile

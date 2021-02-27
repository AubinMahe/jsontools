#!/bin/bash

git clean -fdX
mkdir -p m4
autoreconf --install
if [ $? -eq 0 ] ; then
   mkdir BUILD
   cd BUILD
   ../configure
   if [ $? -eq 0 ] ; then
      make dist
      make check
   fi
fi

#!/bin/sh

echo "Removing Old Things"
rm *.py
rm *.pm
rm *.java
rm *.cs

echo "Python"
swig -python rlib.i
cp rlib_wrap.c ../bindings/python/python.c
cp rlib.py ../bindings/interfaces/rlib.py

echo "Perl"
swig -perl rlib.i
cp rlib_wrap.c ../bindings/perl/perl.c
cp rlib.pm ../bindings/interfaces/rlib.pm

echo "Java"
swig -java rlib.i
cp rlib_wrap.c ../bindings/java/java.c
cp *.java ../bindings/interfaces/


echo "C Sharp"
swig -csharp rlib.i
cp rlib_wrap.c ../bindings/csharp/csharp.c
sed -e '/DllImport/s/\"rlib\"/\"rlibcsharp\"/g' rlibPINVOKE.cs > foo
cp foo rlibPINVOKE.cs
cp *.cs ../bindings/interfaces/

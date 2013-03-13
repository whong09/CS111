#! /bin/bash

yes happytest | head -n 9140 > test/largefile.txt

./ospfscrash 112
cp test/largefile.txt test/largefilecp.txt

./ospfscrash -1
yes gotcutoff | head -n 410 > test/newfile.txt

./ospfscrash 2
cp test/newfile.txt test/newfilecp.txt

./ospfscrash 3
cp test/newfile.txt test/newfilecp2.txt

./ospfscrash 7
echo smile >> test/subdir/smile.txt
ln test/subdir/smile.txt test/smile.txt
rm test/subdir/smile.txt

./ospfscrash -1
ls -la test


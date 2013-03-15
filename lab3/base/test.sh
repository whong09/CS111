#! /bin/bash

yes happytest | head -n 9140 > test/largefile.txt
yes importantinfo | head -n 1000 > test/important.txt

./ospfscrash 112
cp test/largefile.txt test/largefilecp.txt

./ospfscrash -1
yes gotcutoff | head -n 410 > test/newfile.txt

./ospfscrash 2
cp test/newfile.txt test/newfilecp.txt

./ospfscrash 3
cp test/newfile.txt test/newfilecp2.txt

./ospfscrash 5
echo more important >> test/important.txt
ln test/important.txt test/subdir/important.txt
rm test/important.txt

./ospfscrash -1
ls -la test
rm test/important.txt

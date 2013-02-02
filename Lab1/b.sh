# redirection
#echo this is the redirection > redir.txt
echo lala overwriting the redirection > redir.txt
sort -r < redir.txt > sorted.txt

#pipe
echo first | echo second | echo third
ls | sort -r | cat
who | sort 
echo first > redir.txt | echo second

#sub
(echo shelly works)
(echo first line in newshell; echo second line)
(echo 1 && (echo 2; echo 7) || echo 3 | cat)
(echo 1
echo 2)

#and/or
echo or || echo and && echo what
echo a || echo b;
cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!

#clean
rm redir.txt
rm sorted.txt
ls

# exit status check
false && echo 1;
true && false;
true | false;
(false)
false;

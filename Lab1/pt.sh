# redirection
echo this is the redirection > redir.txt
echo lala overwriting the redirection > redir.txt
sort -r < redir.txt > sorted.txt

#pipe
echo first | echo second | echo third
ls | sort -r | cat
who | sort 

#sub
(echo shelly works)
(echo first line in newshell; echo second line)
(echo 1 && (echo 2; echo 7) || echo 3 | cat)

#and/or
echo or || echo and && echo what
echo a || echo b;

#clean
rm redir.txt
rm sorted.txt
ls

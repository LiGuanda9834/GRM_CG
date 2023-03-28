make clean
make -j
valgrind --log-file="./log/valgrind_report.log" --tool=memcheck --leak-check=yes --track-origins=yes ./bin/WTA_CG $1 $2 $3 $4 $5 $6 $7


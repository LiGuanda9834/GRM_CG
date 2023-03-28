make -j
result_name="./results/WTA_${1}.out"
rm -rf ${result_name}
./bin/WTA_CG $1 $2 $3 


dir="/share/home/liguanda/WTA-Problem/Code/WTA_CG/cluster_test/test_random"
rm -rf ${dir}/*.out
rm -rf ${dir}/*.err
rm -rf ${dir}/run.sh
rm -rf ${dir}/results.txt
./script.sh
bsub -J psclp -q batch -R "span[ptile=36]" -n 180 -e %J.err -o %J.out "mpirun ~/cmipwork/check/bin/lsf ${dir}/run.sh"
./Results_compare.sh
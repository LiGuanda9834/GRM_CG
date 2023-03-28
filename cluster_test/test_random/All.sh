dir="/share/home/liguanda/WTA-Problem/Code/WTA_CG/cluster_test/test_random"
rm -rf ${dir}/*.out
rm -rf ${dir}/*.err
rm -rf ${dir}/run.sh
rm -rf ${dir}/results.txt
./script.sh
chmod +x "${dir}/run.sh"
./run.sh
./Results_compare.sh
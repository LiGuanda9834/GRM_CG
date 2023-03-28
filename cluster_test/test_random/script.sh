test_size=(5 10 20 30 40 50 60)
#
random_seed=(0 1 2 3 4 5 6 7 8 9)
#
dir="/share/home/liguanda/WTA-Problem/Code/WTA_CG"
test_dir="${dir}/cluster_test/test_random"
runfile="${test_dir}/run.sh"
rm -rf ${runfile}
for size in ${test_size[@]}
do
   for seed in ${random_seed[@]}
   do   
      echo "size : ${size}, seed : ${seed}"
      name=WTA_CG
      #name=$(basename ${file} .txt)
      #num=$(cut $(echo ${name}) | cut -c 4-)
      #echo ${num}
      echo "${dir}/bin/WTA_CG ${seed} ${size} ${size} > $test_dir/${name}_${size}_${seed}.out 2>$test_dir/${name}_${size}_${seed}.err" >> ${runfile}
      #echo ${name}
   done
done

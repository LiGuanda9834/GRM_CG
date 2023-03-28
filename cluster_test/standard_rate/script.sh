target_num=(5 10 20 30 40 50 60 70 80)
#
wt_rate=(4)
#4 8
dir="/share/home/liguanda/WTA-Problem/Code/WTA_CG"
test_dir="${dir}/cluster_test/standard_rate"
runfile="${test_dir}/run.sh"
rm -rf ${runfile}
rm -rf ${test_dir}/*.out
rm -rf ${test_dir}/*.err
rm -rf ${test_dir}/*.outc
rm -rf ${test_dir}/*.errc
for t_num in ${target_num[@]}
do
   for wt_r in ${wt_rate[@]}
   do
      w_num=$((t_num * wt_r))
      echo "t : ${t_num}, w : ${w_num}"
      name=WTA_CG
      seed=0
      #name=$(basename ${file} .txt)
      #num=$(cut $(echo ${name}) | cut -c 4-)
      #echo ${num}
      echo "${dir}/bin/WTA_CG ${seed} ${t_num} ${w_num} > $test_dir/${name}_${t_num}_${w_num}.out 2>$test_dir/${name}_${t_num}_${w_num}.err" >> ${runfile}
      #echo ${name}
   done
done
chmod +x ${runfile}

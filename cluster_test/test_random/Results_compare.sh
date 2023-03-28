
dir="/share/home/liguanda/WTA-Problem/Code/WTA_CG/cluster_test/test_random"
runfile="${dir}/run.sh"
results_file="${dir}/results.txt"
rm -rf ${results_file}




for file_name in ${dir}/*.out
do
   echo ${file_name}

   CG_FINISHED_LINE=`grep -no "Column Generation finished" ${file_name} | cut -d: -f1`
   NUM_ROWS=`wc -l ${file_name} | cut -d' ' -f1`
   echo ${NUM_ROWS}
   echo ${CG_FINISHED_LINE} 

   FROM_END=$((NUM_ROWS - CG_FINISHED_LINE))
   OPTIMAL_LINE=$((${FROM_END} + 12))

   echo ${OPTIMAL_LINE}

   # Optimal value


   out_file_name=$(basename ${file_name} .out)
   out_number=${out_file_name/WTA_CG_/}
   out_num_1=${out_number/_*/}
   out_num_2=${out_number#*_}
   echo "size : ${out_num_1}, seed : ${out_num_2}" >> ${results_file}
   tail -n ${FROM_END} ${file_name} | head -n 1 >> ${results_file}
   tail -n ${OPTIMAL_LINE} ${file_name} | head -n 5 >> ${results_file}
   echo " ">> ${results_file}
   #name=$(basename ${file} .txt)
   #num=$(cut $(echo ${name}) | cut -c 4-)
   #echo ${num}
   #echo "./bin/WTA_CG ${size} > $dir/${name}_${size}.out 2>$dir/${name}_${size}.err" >> ${runfile}
   #echo ${name}
done

test_size=(5 10 20 30 40 50 60 70 80 90 100 200)
dir="/share/home/liguanda/WTA-Problem/Code/WTA_CG/cluster_test"
runfile="${dir}/run.sh"
results_file="${dir}/results.txt"
rm -rf ${results_file}
for size in ${test_size[@]}
do
   echo >> ${results_file}
   echo ${size}
   file_name="${dir}/WTA_CG_${size}.out"
   echo ${file_name}
   tail -n 2 ${file_name} | head -n 1 >> ${results_file}
   tail -n 10 ${file_name} | head -n 1 >> ${results_file}
   tail -n 22 ${file_name} | head -n 5 >> ${results_file}
   #name=$(basename ${file} .txt)
   #num=$(cut $(echo ${name}) | cut -c 4-)
   #echo ${num}
   #echo "./bin/WTA_CG ${size} > $dir/${name}_${size}.out 2>$dir/${name}_${size}.err" >> ${runfile}
   #echo ${name}
done

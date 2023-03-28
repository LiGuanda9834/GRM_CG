test_size=(5 10 20 30 40 50 60 70 80 90 100 200)
dir="/share/home/liguanda/WTA-Problem/Code/WTA_CG/cluster_test"
runfile="${dir}/run.sh"
rm -rf ${runfile}
for size in ${test_size[@]}
do
   echo ${size}
   name=WTA_CG
   #name=$(basename ${file} .txt)
   #num=$(cut $(echo ${name}) | cut -c 4-)
   #echo ${num}
   echo "./bin/WTA_CG ${size} > $dir/${name}_${size}.out 2>$dir/${name}_${size}.err" >> ${runfile}
   #echo ${name}
done

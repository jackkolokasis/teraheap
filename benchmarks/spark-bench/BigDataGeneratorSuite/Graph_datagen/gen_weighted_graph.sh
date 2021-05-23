#!/bin/bash
##
# Function: Generate weighted graph data
#
# To Run: ./gen_weighted_graph.sh
#
# Output in ./ DIR
#
# By: Mingzijian, mingzijian@ict.ac.cn
##



echo "Generating weighted graph data"
read -p "Please Enter The Iterations of GenGragh: " I
 
./gen_kronecker_graph -o:./temp/AMR_gen_1_$I.txt -m:"0.8815 0.4795; 0.4507 0.2752" -i:$I
./gen_kronecker_graph -o:./temp/AMR_gen_2_$I.txt -m:"0.9209 0.4049; 0.4556 0.2645" -i:$I
./gen_kronecker_graph -o:./temp/AMR_gen_3_$I.txt -m:"0.9278 0.4362; 0.4924 0.2587" -i:$I
./gen_kronecker_graph -o:./temp/AMR_gen_4_$I.txt -m:"0.9687 0.4585; 0.4548 0.2524" -i:$I
./gen_kronecker_graph -o:./temp/AMR_gen_5_$I.txt -m:"0.9495 0.5297; 0.4147 0.2416" -i:$I

sed 1,4d -i ./temp/AMR_gen_1_$I.txt
sed 1,4d -i ./temp/AMR_gen_2_$I.txt
sed 1,4d -i ./temp/AMR_gen_3_$I.txt
sed 1,4d -i ./temp/AMR_gen_4_$I.txt
sed 1,4d -i ./temp/AMR_gen_5_$I.txt

python give_weighted.py ./temp/AMR_gen_1_$I.txt 1 ./temp/AMR_gen_1_score_$I.txt
python give_weighted.py ./temp/AMR_gen_2_$I.txt 2 ./temp/AMR_gen_2_score_$I.txt
python give_weighted.py ./temp/AMR_gen_3_$I.txt 3 ./temp/AMR_gen_3_score_$I.txt
python give_weighted.py ./temp/AMR_gen_4_$I.txt 4 ./temp/AMR_gen_4_score_$I.txt
python give_weighted.py ./temp/AMR_gen_5_$I.txt 5 ./temp/AMR_gen_5_score_$I.txt


cat ./temp/AMR_gen_1_score_$I.txt ./temp/AMR_gen_2_score_$I.txt ./temp/AMR_gen_3_score_$I.txt ./temp/AMR_gen_4_score_$I.txt ./temp/AMR_gen_5_score_$I.txt > ./temp/AMR_gen_unsorted_$I.txt

sort ./temp/AMR_gen_unsorted_$I.txt > ./temp/AMR_gen_sorted_$I.txt

python reduce.py ./temp/AMR_gen_sorted_$I.txt ./AMR_gen_final_$I.txt

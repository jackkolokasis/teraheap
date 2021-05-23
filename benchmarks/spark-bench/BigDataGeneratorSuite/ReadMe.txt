========================================================================
  BDGS : Big Data Generator Suite
	http://prof.ict.ac.cn/BigDataBench/
========================================================================

Big Data Generator Suite (BDGS) is a suite of generators, which can efficiently 
generate scalable big data while employing data models derived from real data 
to preserve data veracity. The tool includes 8 specific generators based on a 
variety of real-world data sets from different internet services domains, 
covering three representative data types (structured, semi-structured and 
unstructured) and three data sources (text, graph, and table data).
Most programs in BDGS is written in C++ and for more information you can read 
our paper: BDGS: A Scalable Big Data Generator Suite in Big Data Benchmarking 
from http://arxiv.org/abs/1401.5465 .

/////////////////////////////////////////////////////////////////////////////

Download:
  http://prof.ict.ac.cn/BigDataBench/#downloads

  Text_datagen: 
        text data generators for Wikipedia article and Amazon Movie Review.
  Graph_datagen: 
        graph data generators for Amazon(weighted and un-weighted), Facebook and Google graph.
  Table_datagen:
        table data generdators for E-com and Personal Resume.

Some of programs expect that GSL(http://www.gnu.org/software/gsl/) and 
PSSH(https://code.google.com/p/parallel-ssh/) are installed and accessible 
-- paths are in the system PATH variable or they reside in the working directory.

/////////////////////////////////////////////////////////////////////////////

To compile from the command line, execute:
  make all

Execution examples:
  Wikipedia generator:
        We have 3 trained models:lda_wiki1w, wiki_1w5 and wiki_noSW_90_Sampling
	To Run£º Text_datagen/gen_text_data.sh wiki_noSW_90_Sampling 10 100 10000 gen_data/
	
  Amazon movie review generator :
        We have 2 trained models for each score,such as amazonMR1 and AMR1_noSW_95_Sampling
	To Run: Text_datagen/gen_text_data.sh AMR1_noSW_95_Sampling 10 100 10000 gen_data/
	
  Amazon un-weighted graph:
	To Run: Graph_datagen/gen_weighted_graph.sh
  Amazon weighted graph:
    To Run: Graph_datagen/gen_kronecker_graph -o:amazon_gen_16.txt -m:"0.9532 0.5502; 0.4439 0.2511" -i:16
  Facebook graph:
    To Run: Graph_datagen/gen_kronecker_graph -o:facebook_g_16.txt -m:"0.9999 0.5887; 0.6254 0.3676" -i:16
  Google graph :
    To Run: Graph_datagen/gen_kronecker_graph -o:google_g_16.txt -m:"0.8305 0.5573; 0.4638 0.3021" -i:16

  E-com table :
        We us PDGF for generate E-com table
	To Run: Table_datagen/e-com/run.sh
  Personal Resume :
	To Run: Table_datagen/personal_generator/gen_resume.sh 100 10 gen_data/
  
  Run in parallel£º
	    mkdir /mnt/raid/BigDataGeneratorSuite in every node
		Configure Non password login and the host: parallel_ex/conf_hosts
	To Run:
        parallel_ex/deploy_ex.sh
		parallel_ex/run_personalResumeGen_10T.sh

/////////////////////////////////////////////////////////////////////////////

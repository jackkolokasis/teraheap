#include <stdio.h>
#include <math.h>
#include <time.h> 
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_math.h>

using namespace std;

int main (int argc,char* argv[])
{
    // count time
    //time_t rawtime;
    //time (&rawtime);
    //cout<<"The start time is: "<<rawtime<<endl;


    if(argc !=5){
        cout<<"file missed! "<<endl;
        return -1;       
    }

//*************Init*************
    //
    char* modeldirname = argv[1];      
    string linesof_gendata_s = argv[2];
    string ex_wordsof_file_s = argv[3];
    string seed_s = argv[4];
    int seed = atoi(seed_s.c_str());
    int lines_of_gendata = atoi(linesof_gendata_s.c_str());
    int ex_words_of_file = atoi(ex_wordsof_file_s.c_str());
    cout<<"The lines of gen data is:"<<lines_of_gendata<<endl;
    cout<<"The expectation words of each file is:"<<ex_words_of_file<<endl;


    char* alpha_temp1="/mnt/raid/BigDataGeneratorSuite/Text_datagen/";
    char* alpha_temp2="/final.other";
    char* alphafile = new char[strlen(alpha_temp1)+strlen(modeldirname)+strlen(alpha_temp2)+1];
    strcpy(alphafile, alpha_temp1);
    strcat(alphafile, modeldirname);
    strcat(alphafile, alpha_temp2);
        
    char* beta_temp1="/mnt/raid/BigDataGeneratorSuite/Text_datagen/";
    char* beta_temp2="/final.beta";
    char* betafile = new char[strlen(beta_temp1)+strlen(modeldirname)+strlen(beta_temp2)+1];
    strcpy(betafile, beta_temp1);
    strcat(betafile, modeldirname);
    strcat(betafile, beta_temp2);

    char* voca_temp1="/mnt/raid/BigDataGeneratorSuite/Text_datagen/";//seed_data
    char* voca_temp2="/";
    char* voca_temp3=".voca";
    char* vocafile = new char[strlen(voca_temp1)+strlen(modeldirname)+strlen(voca_temp2)+strlen(modeldirname)+strlen(voca_temp3)+1];
    strcpy(vocafile,voca_temp1);
    strcat(vocafile, modeldirname);
    strcat(vocafile, voca_temp2);
    strcat(vocafile, modeldirname);
    strcat(vocafile, voca_temp3);

    cout<<"alphafile is:"<<alphafile<<endl;
    cout<<"betafile is:"<<betafile<<endl;
    cout<<"vocafile is:"<<vocafile<<endl;

    //init parametres
    ifstream parametres(alphafile);
    string line;
    getline(parametres,line);
    string::size_type index = line.find_first_of(" ",0);    
    string topics = line.substr(index+1);
    int topics_num =atoi(topics.c_str());
    cout<<"topic numbers is:"<<topics_num<<endl;

    getline(parametres,line);
    index = line.find_first_of(" ",0);
    string terms = line.substr(index+1);
    int terms_num =atoi(terms.c_str());
    //int temp =terms_num;
    //terms_num =5000;
    cout<<"term numbers is:"<<terms_num<<endl;

    getline(parametres,line);
    index = line.find_first_of(" ",0);
    string alpha_string = line.substr(index+1);
    double alpha =atof(alpha_string.c_str());
    cout<<"alpha numbers is:"<<alpha<<endl;

    //init beta
    ifstream lda_data(betafile);
    double beta[topics_num][terms_num];
    //double temp2;
    for(int j=0;j<topics_num;j++){
        for(int i=0;i<terms_num;i++){
            lda_data >> beta[j][i];
            beta[j][i] = exp(beta[j][i]);
            //cout<<beta[j][i]<<" ";
        }
        //cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
      //  for(int i=5000;i<temp;i++){
      //      lda_data >> temp2;
      //  }
    }
    //cout<<line<<endl;
    //init vocabulary
    ifstream dictionary(vocafile);
    string voca[terms_num];
    for(int i=0;i<terms_num;i++){
        getline(dictionary,line);
        voca[i]=line;
        //cout<<voca[i]<<" ";
    }

//*************Generate Data*************
    //init
    const gsl_rng_type *T;    //定义type变量
    gsl_rng *r;
    //gsl_rng *r1;
    //gsl_rng *r3;

    //gl_rng_env_setup();   //读取环境变量GSL_RNG_TYPE>    和GSL_RNG_SEED的值，并把他们分别赋给gsl_rng_default和gs    l_rng_default_seed
    //T = gsl_rng_default;
    T = gsl_rng_ranlxs0;    //设随机数生成器类型是 ranlxs0
    gsl_rng_default_seed = ((unsigned long)(time(NULL))+seed);    //设seed值为当前时间+seed
    //cout<<(unsigned long)(time(NULL)+1)<<endl; 
    r=gsl_rng_alloc(T);    //产生以T为类型的随机数生成器实例


    double theta[topics_num];
    double theta_sum=0;
    for(int i=0;i<topics_num;i++){
        theta[i]=gsl_ran_gamma(r,alpha,1);
        theta_sum+=theta[i];
        //cout<<theta[i]<<endl;
    }
    //cout<<theta_sum<<endl;

    for(int i=0;i<topics_num;i++){
        theta[i]=theta[i]/theta_sum;
    }


    // run

    int length;//文档词数
    unsigned int cats[topics_num];//存放每个主题词要生成的词的数量
    for(int size_i=0; size_i<lines_of_gendata;size_i++){
        //length=ex_words_of_file/2+abs(int(gsl_ran_ugaussian(r)*ex_words_of_file/2));
        //T = gsl_rng_ranlxs1;    //设随机数生成器类型是 ranlxs1
        gsl_rng_default_seed++;    //改变seed值
        //cout<<gsl_rng_default_seed<<endl;
        //r=gsl_rng_alloc(T);    //产生以T为类型的随机数生成器实例
        length=gsl_ran_poisson(r,ex_words_of_file);
        //cats=np.random.multinomial(length,theta);

        //T = gsl_rng_ranlxs1;    //设随机数生成器类型是 ranlxs1
        gsl_rng_default_seed++;    //改变seed值
        //cout<<gsl_rng_default_seed<<endl;
        //r=gsl_rng_alloc(T);    //产生以T为类型的随机数生成器实例
        //cout<<length<<endl;
        gsl_ran_multinomial(r,topics_num,length, &theta[0], &cats[0]);

        /*
        for(int i=0;i<topics_num;i++){
            printf("%d ",cats[i]);
        }
        */

        //unsigned int N=1000;//N次试验
        //int K=terms_num;//词项的个数
        //double p[terms_num];//每一个单位的概率
        unsigned int words_count[topics_num][terms_num];//存放词数量输出结果
        unsigned int words_sum[terms_num];//所有主题词数加和

        //set words_sum=0

        for(int i=0;i<terms_num;i++){
            words_sum[i]=0;
        }

        for(int i=0;i<topics_num;i++){
            //T = gsl_rng_ranlxs1;    //设随机数生成器类型是 ranlxs1
            gsl_rng_default_seed++;    //改变seed值
            //cout<<gsl_rng_default_seed<<endl;
            //r=gsl_rng_alloc(T);    //产生以T为类型的随机数生成器实例
            gsl_ran_multinomial(r,terms_num,cats[i], &beta[i][0], &words_count[i][0]);
            for (int j=0;j<terms_num;j++){
                words_sum[j]+=words_count[i][j];
                //cout<<words_count[i][j];
            }
        //(const gsl_rng * r, size_t K, unsigned int N, const double p[], unsigned int n[])
        }

        for (int i=terms_num-1;i>=0;i--){
           //cout<<words_sum[i]<<" ";
           for (unsigned int j=0;j<words_sum[i];j++){
               cout<<voca[i]<<" ";
           }
           
        }

        cout<<endl;
    }

    //time (&rawtime);
    //cout<<"The end time is: "<<rawtime<<endl;
   
    gsl_rng_free(r);
    return 0;
}

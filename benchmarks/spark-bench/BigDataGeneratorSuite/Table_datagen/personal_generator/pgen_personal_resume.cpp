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

#define P 0.8
#define RANGE_firstname 58381
#define RANGE_address 1573
#define RANGE_family_names 1056
#define RANGE_hometown 7339
#define RANGE_institute 9393
#define RANGE_interest 118057
#define RANGE_tittle 4934
#define RANGE_school 2199
#define RANGE_papers_source 143485
#define RANGE_publication 300000
//1002039

using namespace std;

int main (int argc,char* argv[])
{
    // count time
    //time_t rawtime;
    //time (&rawtime);
    //cout<<"The start time is: "<<rawtime<<endl;


    
    if(argc !=3){
        cout<<"input missed! "<<endl;
        return -1;       
    }
    

    string str_Person = argv[1];
    int num_Person = atoi(str_Person.c_str());

    string seed_s = argv[2];
    int seed = atoi(seed_s.c_str());


//*************Init*************
    //
    //
    ifstream if_firstname("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/names.dict");
    ifstream if_familyname("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/family_names.dict");
    ifstream if_address("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/depart_address.dict");
    ifstream if_hometown("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/hometown.dict");
    ifstream if_interest("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/interest.dict");
    ifstream if_tittle("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/tittle.dict");
    ifstream if_institute("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/institute.dict");
    ifstream if_school("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/school.dict");
    ifstream if_papers_source("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/papers_source.dict");
    ifstream if_publication("/mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/dict/publication.dict");
    
    string word;
    string firstname[RANGE_firstname];
    int i = 0;
    while(i<RANGE_firstname){
        getline(if_firstname,word,'\n');
        firstname[i]=word;
        //cout<<firstname[i]<<endl;
        i++;
    }
    
    string familyname[RANGE_family_names];
    i = 0;
    while(i<RANGE_family_names){
        getline(if_familyname,word,'\n');
        familyname[i]=word;
        //cout<<familyname[i]<<endl;
        i++;
    }
    
    string address[RANGE_address];
    i =0;
    while(i<RANGE_address){
        getline(if_address,word,'\n'); 
        address[i]=word; 
        //cout<<address[i]<<endl;
        i++;
    }      
    
    string hometown[RANGE_hometown];
    i = 0;
    while(i<RANGE_hometown){
        getline(if_hometown,word,'\n'); 
        hometown[i]=word;
        //cout<<hometown[i]<<endl;
        i++;
    } 
    
    string interest[RANGE_interest];
    i = 0;
    while(i<RANGE_interest){
        getline(if_interest,word,'\n');                 
        interest[i]=word;                                
        //cout<<interest[i]<<endl;                                        
        i++;                           
    }    
    
    string tittle[RANGE_tittle];
    i = 0;
    while(i<RANGE_tittle){
        getline(if_tittle,word,'\n');                 
        tittle[i]=word;                                
        //cout<<tittle[i]<<endl;                                       
        i++;                           
    }    
    
    string institute[RANGE_institute];
    i = 0;
    while(i<RANGE_institute){
        getline(if_institute,word,'\n');                 
        institute[i]=word;                                
        //cout<<institute[i]<<endl;                                        
        i++;                            
    }


    
    string school[RANGE_school];
    i = 0;
    
    while(i<RANGE_school){
        getline(if_school,word,'\n');
        school[i]=word;
        //cout<<school[i]<<endl;
        i++;
    }  


        
    string papers_source[RANGE_papers_source];
    i = 0;
    while(i<RANGE_papers_source){
        getline(if_papers_source,word,'\n');
        papers_source[i]=word;
        //cout<<school[i]<<endl;
        i++;

       
    }   


    string publication[RANGE_publication];
    i = 0;
    while(i<RANGE_publication){
        getline(if_publication,word,'\n');
        publication[i]=word;
        //cout<<school[i]<<endl;
        i++;

    }
    
//*************Generate Data*************

    while(num_Person--){
    const gsl_rng_type *T;    //定义type变量
    gsl_rng *r; 
    //gsl_rng *r1;  
    //gsl_rng *r3;
    //gl_rng_env_setup();   
    //读取环境变量GSL_RNG_TYPE>    和GSL_RNG_SEED的值，并把他们分别赋给gsl_rng_default和gs    l_rng_default_seed                        
    //T = gsl_rng_default;

    T = gsl_rng_ranlxs0;    //设随机数生成器类型是 ranlxs0
    gsl_rng_default_seed = ((unsigned long)(time(NULL))+seed-num_Person);    //设seed值为当前时间+seed

    //cout<<(unsigned long)(time(NULL)+1)<<endl; 
    r=gsl_rng_alloc(T);    //产生以T为类型的随机数生成器实例

    int Num_familyname = int(gsl_rng_uniform(r)*RANGE_family_names);
    int Num_firstname = int(gsl_rng_uniform(r)*RANGE_firstname);
    cout<<familyname[Num_familyname]<<firstname[Num_firstname]<<"|";

    char words[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};

    //const int numbers[9] = {1,2,3,4,5,6,7,8,9};

    int output_year;
    int output_month;
    int output_day;

    int birth_flag = 0;

    //Email
    if(gsl_ran_bernoulli(r,P)==1){
    
        int Num_Email = int(gsl_rng_uniform(r)*10+3);
        int Num_address = int(gsl_rng_uniform(r)*RANGE_address);
        while(Num_Email--){
            
            //int outputEmail = gsl_rng_uniform(r)*25+1;
            int outputEmail = (rand() % (25-0+1))+ 0;
            //cout<<outputEmail<<endl;
            cout<<words[outputEmail];
        }

        cout<<"@"<<address[Num_address]<<"|";

        //address
        if(gsl_ran_bernoulli(r,P)==1){
            cout<<address[Num_address]<<"|";
        }     
    
    }

    //Tel
    if(gsl_ran_bernoulli(r,P)==1){
        //int Num_Tel = 8;
        int output_Tel = (rand() % (99999999-10000000+1))+ 10000000;
        //int output_Tel = gsl_rng_uniform(r)*9;
        //while(Num_Tel--){
        //    cout<<output_Tel;
        //}
        cout<<output_Tel<<"|";
    }   
    
    //date of birth
    if(gsl_ran_bernoulli(r,P)==1){

        output_year = (rand() % (1999-1930+1))+ 1930;
        output_month = (rand() % (12-1+1))+ 1;
        output_day = (rand() % (31-1+1))+ 1;
        cout<<output_year<<"-"<<output_month<<"-"<<output_day<<"|";
   
        birth_flag = 1;
    }   

    //homeplace 
    if(gsl_ran_bernoulli(r,P)==1){
        int Num_homeplace = int(gsl_rng_uniform(r)*RANGE_hometown);
        cout<<hometown[Num_homeplace]<<"|";        
    }   

    //institute
    if(gsl_ran_bernoulli(r,P)==1){
        int Num_institute = int(gsl_rng_uniform(r)*RANGE_institute);
        cout<<institute[Num_institute]<<"|";                                  
    }   


    //tittle
    if(gsl_ran_bernoulli(r,P)==1){
        int Num_tittle = int(gsl_rng_uniform(r)*RANGE_tittle);
        cout<<tittle[Num_tittle]<<"|"; 
    }   

    //research interest
    if(gsl_ran_bernoulli(r,P)==1){
        int Num_interest = int(gsl_rng_uniform(r)*RANGE_interest);
        cout<<interest[Num_interest]<<"|";
    }   

    //experience
    if(gsl_ran_bernoulli(r,P)==1){
        
        if(gsl_ran_bernoulli(r,P)==1){
            
            if(birth_flag==0){
                output_year = (rand() % (1999-1930+1))+ 1930;
                birth_flag=1;
            }
            
            output_year = (rand() % (24+1))+ output_year;
            output_month = (rand() % (12-1+1))+ 1;
            output_day = (rand() % (31-1+1))+ 1;
            cout<<output_year<<"-"<<output_month<<"-"<<output_day<<"--";

            cout<<output_year+4<<"-"<<output_month<<"-"<<output_day<<"   ";

            //school
            //
            int Num_school = int(gsl_rng_uniform(r)*RANGE_school);
            cout<<school[Num_school];

            //degree
            cout<<"本科"<<"    ";
                                                    
        }
       
        if(gsl_ran_bernoulli(r,P)==1){

            
            if(birth_flag==0){
                output_year = (rand() % (1999-1930+1))+ 1930;
                birth_flag=1;
            }
            
            output_year = (rand() % (4+1))+ output_year+4;
            output_month = (rand() % (12-1+1))+ 1;
            output_day = (rand() % (31-1+1))+ 1;
            cout<<output_year<<"-"<<output_month<<"-"<<output_day<<"--";

            
            output_year = (rand() % (5+1))+ output_year;
            output_month = (rand() % (12-1+1))+ 1;
            output_day = (rand() % (31-1+1))+ 1;
            cout<<output_year+3<<"-"<<output_month<<"-"<<output_day<<"   ";
 
            //school
            int Num_school = int(gsl_rng_uniform(r)*RANGE_school);
            cout<<school[Num_school];
            //degree
            //
            if(gsl_ran_bernoulli(r,P)==1){
                cout<<"硕士研究生";
            }
            else{
                cout<<"博士研究生";
            }



        }
        cout<<"|";                                     
    
    }
   
    //publication
    //
    if(gsl_ran_bernoulli(r,P)==1){
        int Num_publication = int(gsl_rng_uniform(r)*RANGE_publication);
        int count = Num_publication%10;
        for(int p=0;p<count;p++){
         
            //cout<<"著作情况：";
            cout<<p+1<<".";
            Num_publication = int(gsl_rng_uniform(r)*RANGE_publication);
            cout<<publication[Num_publication]<<", ";
            int Num_papers_source = int(gsl_rng_uniform(r)*RANGE_papers_source);
            cout<<"刊物："<<papers_source[Num_papers_source]<<";";
        }
        cout<<"|";
    
    }

   
    cout<<endl;
    gsl_rng_free(r);


}

return 0;
}

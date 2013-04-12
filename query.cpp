// A branch prediction optimizer
// Implemented in the spirit of
//      Ross, Kenneth A. "Selection conditions in main memory." ACM Transactions on Database Systems (TODS) 29.1 (2004): 132-161.
// Date: Apr 11, 2013
// Author: Le Chang, Yu Qiao
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include "query.h"
using namespace std;

struct config_t{
        int r;
        int t;
        int l;
        int m;
        int a;
        int f;
};

void usage()
{
        cout << "Usage: query query.txt config.txt" << endl;
}

void parse_input(string line, float * selectivity, int & n)
{
    n = 0;
    for (int i = 0 ; i < MAXN ; ++i)
	selectivity[i] = 0.0f;	
    while (line.length() > 0){
                size_t space_index = line.find(' ');
                if (space_index == string::npos)                //reaches the end of line
                        space_index = line.length();
                string value_str = line.substr(0,space_index);
                float value = atof(value_str.c_str());
                selectivity[n++] = value;
                line.erase(0,space_index+1);
        }
}

string to_string(int x)
{
        char buffer[1024];
        snprintf(buffer,1024,"%d",x);
        return string(buffer);
}

string to_string(float x)
{
        char buffer[1024];
        snprintf(buffer,1024,"%.2f",x);
        return string(buffer);
}

//TODO: trim
void read_config(char * config_filename, struct config_t & config)
{
        ifstream config_input(config_filename);
        if (!config_input){
                cout << "Error reading config file!" << endl;
                exit(-1);
        }
        string line;
        while (getline(config_input,line)){
                size_t equal_index = line.find('=');
                if (equal_index ==  string::npos)
                        continue;
                int rval = atoi(line.substr(equal_index + 1).c_str());
                string lval = line.substr(0,equal_index - 1);
                switch (lval[0]){
                        case 'r': config.r = rval; break;
                        case 't': config.t = rval; break;
                        case 'l': config.l = rval; break;
                        case 'm': config.m = rval; break;
                        case 'a': config.a = rval; break;
                        case 'f': config.f = rval; break;
                        default: break;
                }
        }
        config_input.close();
}
struct sub_sol{
        int num_ele;
        float product_sel;
        float cost;
        bool is_no_branch;
        int left_sol_index;
        int right_sol_index;
		sub_sol(){
			num_ele=0;
			product_sel=1.0f;
			cost=0;
			is_no_branch=false;
			left_sol_index=0;
			right_sol_index=0;
		}
};
inline float c_metric(const sub_sol &sol,const config_t &config)
{
	return  (sol.product_sel-1)/sol.num_ele*config.r+(sol.num_ele-1)*config.l+config.f*sol.num_ele+config.t;
}
inline float fcost(const sub_sol &sol,const config_t &config)
{
		return  sol.num_ele*config.r+(sol.num_ele-1)*config.l+config.f*sol.num_ele+config.t;
}
inline float min(float a, float b){ return a>b?b:a;}
/*identify the &-term in the right child that dominate the left by tree traversal*/
bool d_metric(const sub_sol &sol_j,const sub_sol &sol_i,int i,const sub_sol * sol, const config_t &config){
	int r_index = i;
	int l_index= 0;
	while(sol[r_index].right_sol_index){
		r_index = sol_i.right_sol_index;
	    l_index= sol_i.left_sol_index;
		return !(sol_j.product_sel > sol[l_index].product_sel && fcost(sol_j,config) > fcost(sol[l_index],config));
	}
	return (sol_j.product_sel > sol[r_index].product_sel && fcost(sol_j,config) > fcost(sol[r_index],config));
}

string pri(int i,int k) {
	int bits=k;
	bool f=true;
	string ot="";
	while(i){
		if(i&1){
			if(f){
				ot = "t"+to_string(bits)+"[o"+to_string(bits)+"[i]]"+ot;
				f=false;
			}
			else{
				ot = "t"+to_string(bits)+"[o"+to_string(bits)+"[i]] & "+ot;
			}
		  }
		 	i=i>>1;
			bits=bits-1;
		}
	return ot;
}
void reconstruct(sub_sol * sol,int _size,int k){
	int  no_branch=0;
	string branch="";
	string n_branch="";
	int l_index=_size;
	int r_index=_size;
	bool f=true;
	while(sol[r_index].right_sol_index){
		l_index=sol[r_index].left_sol_index;
		r_index=sol[r_index].right_sol_index;
		if(sol[l_index].is_no_branch )
			no_branch |= l_index;
		else
		{ 
			if(f){
			branch ="("+ pri(l_index,k)+")"+branch;
			f=false;
			}
			else 
			{
				branch ="("+pri(l_index,k)+")"+" && "+branch;
			}
		}
	}
      if(sol[r_index].is_no_branch)
			no_branch |= r_index;
		else{
			branch += pri(l_index,k);
	        n_branch=pri(no_branch,k);
		}
		cout<<"if("+branch+"){"<<endl;
		if(n_branch==""){
			cout<<"answer[j++]=i"<<endl;
			cout<<"}";
		}
		else{
			cout<<"answer[j]=i"<<endl;
		        cout<<"("+n_branch+")"<<endl;
		}
	return;
}

void query_opt(const float * selectivity,int k, const struct config_t &config)
{	cout<< k<<endl;
        if(!(selectivity||k)) return;
        int _size = 1 << k;
	cout<<"the size:"<<_size<<endl;
        sub_sol * sol = new sub_sol[_size];
        /*without branching AND*/
        for (int i=1;i<_size;i++){
                int bit = k-1;
		int size = i;
                while( size > 0){
                     if(size&1){
                                sol[i].num_ele+=1;
                                sol[i].product_sel*=selectivity[bit];
                                }
                                size=size>>1;
                                bit=bit-1;
				}
			cout<<sol[i].num_ele<<endl;
			cout<<sol[i].product_sel<<endl;
                     if(config.a>(config.m * min(sol[i].product_sel,1-sol[i].product_sel)+sol[i].product_sel *config.a + config.t))
                                {     
									sol[i].is_no_branch = false;
                                    sol[i].cost = sol[i].num_ele*config.r+(sol[i].num_ele-1)*config.l+config.f*sol[i].num_ele+config.m * min(sol[i].product_sel,1-sol[i].product_sel)+sol[i].product_sel *config.a + config.t;
							  }
					 else       {	sol[i].is_no_branch = true;
                                   sol[i].cost = sol[i].num_ele*config.r+(sol[i].num_ele-1)*config.l+config.f*sol[i].num_ele+config.a;
						    }
			cout<<"statistics:"<<i<<" "<<sol[i].num_ele<<" "<<sol[i].product_sel<<" "<<sol[i].cost<<" "<<sol[i].is_no_branch<<endl;
			}
	
			/*With branching AND*/
				for (int i=1;i<_size;i++){ /*i is the index of right child s*/
					for (int j=1;j<_size;j++){ /*j is the index of left child s' */
						if((i&j)>0||(sol[j].left_sol_index>0)) continue;
						int size_S = i|j;
						if((sol[j].product_sel >= sol[sol[i].left_sol_index].product_sel)&&(c_metric(sol[j],config)>c_metric(sol[i],config))) {;}
				      else if(sol[j].product_sel<=0.5 && d_metric(sol[j],sol[i],i,sol,config))
							{;}
						else 		{
								  float new_cost=fcost(sol[j],config) +config.m*min(sol[j].product_sel,1-sol[j].product_sel)+sol[j].product_sel*sol[i].cost;
								  if(new_cost<sol[size_S].cost)
									  sol[size_S].left_sol_index=j;
								      sol[size_S].right_sol_index=i;
									  sol[size_S].cost=new_cost;
									cout<<"new cost replaced\n";
									cout<<j<<" "<<i<<" "<<new_cost<<"\n";
						}
                     }
				}
				 std::cout<<"========================================="<<endl;
				 for (int i=0;i<k;i++) 
					 cout<<selectivity[i]<<" ";
				 cout<<endl;
			     cout<<"-----------------------------------------"<<endl;
			     reconstruct(sol,_size,k);
				 cout<<"cost: "<<sol[_size-1].cost<<endl;
				delete [] sol;
}

int main(int argc, char * * argv)
{
        if (argc != 3){
                usage();
                return -1;
        }

        struct config_t config;
        read_config(argv[2],config);
//      cout << config.r << " " << config.t << " " << config.l << " " << config.m << " " << config.a << " " << config.f << endl;

        ifstream input(argv[1]);
        if (!input)     {
                cout << "Error reading input file!" << endl;
                return -1;
        }

        float selectivity[MAXN];                        //selectivity for n terms
        int n;

        string line;
        while (getline(input,line)){
                parse_input(line,selectivity,n);                //each line is a set of selectivities
//              for (int j = 0 ; j < n ; ++j)
//                      cout << selectivity[j] << " ";
//              cout << endl;
                query_opt(selectivity,n,config);                                //output the solution for each line
        }
        input.close();
        return 0;
}

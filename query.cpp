// A branch prediction optimizer
// Implemented in the spirit of
//      Ross, Kenneth A. "Selection conditions in main memory." ACM Transactions on Database Systems (TODS) 29.1 (2004): 132-161.
// Date: Apr 11, 2013
// Author: Le Chang, Yu Qiao
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "query.h"
using namespace std;
#define min(a,b) a>b?b:a
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
        memset(selectivity,0.0f,MAXN * sizeof(float));
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
			num_ele=1;
			product_sel=1;
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

/*identify the &-term in the right child that dominate the left by tree traversal*/
bool d_metric(const sub_sol &sol_j,const sub_sol &sol_i,int i,const sub_sol * sol, const config_t &config){
	int r_index = i;
	int l_index= 0;
	while(sol[r_index].right_sol_index){
		int r_index = sol_i.right_sol_index;
	    int l_index= sol_i.left_sol_index;
		if(sol_j.product_sel > sol[l_index].product_sel &&
			fcost(sol_j,config) > fcost(sol[l_index],config) )
			return false;
	}
	if(sol_j.product_sel > sol[r_index].product_sel &&
			fcost(sol_j,config) > fcost(sol[r_index],config) )
	return true;
}

void query_opt(const float * selectivity,int k, const struct config_t &config)
{
        if(selectivity||k) return;
        int _size = 1 << k;
        sub_sol * sol = new sub_sol[_size];
        /*without branching AND*/
        for (int i=1;i<_size;i++){
                int bit = k-1;
                while( _size > 0){
                     if(_size&1){
                                sol[_size].num_ele+=1;
                                sol[_size].product_sel*=selectivity[bit];
                                }
                                _size=_size>>1;
                                bit=bit-1;
				}
                     if(config.a>config.m * min(sol[_size].product_sel,1-sol[_size].product_sel)+sol[_size].product_sel *config.a + config.t)
                                {     
									sol[_size].is_no_branch = false;
                                    sol[_size].cost = sol[_size].num_ele*config.r+(sol[_size].num_ele-1)*config.l+config.f*sol[_size].num_ele+config.m * min(sol[_size].product_sel,1-sol[_size].product_sel)+sol[_size].product_sel *config.a + config.t;
							  }
					 else       {	sol[_size].is_no_branch = true;
                                   sol[_size].cost = sol[_size].num_ele*config.r+(sol[_size].num_ele-1)*config.l+config.f*sol[_size].num_ele+config.a;
						    }
			}	
			/*With branching AND*/
				for (int i=1;i<_size;i++){ /*i is the index of right child s*/
					for (int j=i+1;j<_size;j++){ /*j is the index of left child s' */
						if(i&j>0) continue;
						int size_S = i|j;
						if(!((sol[j].product_sel >= sol[sol[i].left_sol_index].product_sel)&&c_metric(sol[j],config)>c_metric(sol[i],config)
							||(sol[j].product_sel<=0.5 && d_metric(sol[j],sol[i],i,sol,config)))){
								  float new_cost=fcost(sol[j],config) +config.m*min(sol[j].product_sel,1-sol[j].product_sel)+sol[j].product_sel*sol[i].cost;
								  if(new_cost<sol[size_S].cost)
									  sol[size_S].left_sol_index=j;
								      sol[size_S].right_sol_index=i;
									  sol[size_S].cost=new_cost;
						}
                     }
				}
				reconstruct(sol,_size);
}
void reconstruct(sub_sol * sol,int _size){
	string no_branch;
	string branch;
	int l_index=_size;
	int r_index=_size;
	while(sol[r_index].right_sol_index){
		l_index=sol[r_index].left_sol_index;
		r_index=sol[r_index].right_sol_index;
		if(sol[l_index].is_no_branch )
			no_branch = no_branch;
		else
			branch = branch;
	}
      if(sol[r_index].is_no_branch)
			no_branch = no_branch;
		else
			branch = branch;
	return;
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


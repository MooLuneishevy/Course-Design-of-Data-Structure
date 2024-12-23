#include<iostream>
#include<cstring>
#include<ctime>
#include<algorithm>
#include<cmath>
#include"jsoncpp/json.h"
using namespace std;
struct mcts_node{
	int x;
	int y;
	double value;
	double n;
	int son[81];
	int avail[81];
	int avail_action;
	int nson;
	int father;
};
mcts_node node_list[500000];
int start=clock();
int threshold=(int)(0.98*(double)CLOCKS_PER_SEC);
int board[9][9]={0};
int avail_len[2];
int tmp_avail_len[2];
int avail_board[2][81];
int tmp_avail_board[2][81];
int depth_limit=6;
int new_x=-1;
int new_y=-1;
int node_cnt=0;
int enm_col=-1;
int in_board(int x,int y){
	return x>=0 && y>=0 && x<9 && y<9;
}
int dfs_find_liberty(int x,int y,int col){
	if(!in_board(x,y))
		return 0;
	if(board[x][y]==-col)
		return 0;
	else if(board[x][y]==0)
		return 1;
	else{
		board[x][y]=-col;
		int res=dfs_find_liberty(x,y-1,col) || dfs_find_liberty(x,y+1,col) || dfs_find_liberty(x-1,y,col) || dfs_find_liberty(x+1,y,col);
		board[x][y]=col;
		return res;
	}
}
int avail_posi(int x,int y,int col){
	if(board[x][y]!=0 || !in_board(x,y))
		return 0;
	int res=1;
	board[x][y]=col;
	if(!dfs_find_liberty(x,y,col) || (x<8 && board[x+1][y]==-col && !dfs_find_liberty(x+1,y,-col)) || (x>0 && board[x-1][y]==-col && !dfs_find_liberty(x-1,y,-col)) || (y<8 && board[x][y+1]==-col && !dfs_find_liberty(x,y+1,-col)) || (y>0 && board[x][y-1]==-col && !dfs_find_liberty(x,y-1,-col)))
		res=0;
	board[x][y]=0;
	return res;
}
void set_avail(){
	avail_len[0]=0;
	avail_len[1]=0;
	for(int i=0;i<9;i++){
		for(int j=0;j<9;j++){
			if(avail_posi(i,j,1))
				avail_board[1][avail_len[1]++]=i*9+j;
			if(avail_posi(i,j,-1))
				avail_board[0][avail_len[0]++]=i*9+j;
		}
	}
}
double judge_value(int col){
	// value function here
}
double UCB(int current){
	return node_list[current].value/node_list[current].n+2.0*sqrt(log(node_list[node_list[current].father].n)/node_list[current].n);
}
int select(int current,int&col){
    int decision=0;
    if(node_list[current].nson==0){
		int color=col==1?1:0;
		set_avail();
        memcpy(node_list[current].avail,avail_board[color],sizeof(node_list[current].avail));
        node_list[current].avail_action=avail_len[color];
    }
    if(node_list[current].avail_action!=0){
        decision=rand()%node_list[current].avail_action;
        if(decision!=-1){
            node_cnt++;
            int chess=node_list[current].avail[decision];
            node_list[current].avail[decision]=node_list[current].avail[--node_list[current].avail_action];
            node_list[current].son[node_list[current].nson]=node_cnt;
            node_list[current].nson++;
			node_list[node_cnt].father=current;
            node_list[node_cnt].x=chess/9;
            node_list[node_cnt].y=chess%9;
            return node_cnt;
        }
    }
    double max_value=-10000;
	decision=-current;
    for(int i=0;i<node_list[current].nson;i++){
        int child=node_list[current].son[i];
        double value=UCB(child);
        if(value>max_value){
            max_value=value;
            decision=child;
        }
    }
    return decision;
}
int expansion(int current,int&col){
	int dep=0;
	while(dep<depth_limit){
		current=select(current,col);
        if(current<0)
            return -current;
		dep++;
		board[node_list[current].x][node_list[current].y]=col;
		col=-col;
	}
	return current;
}
void mcts(){
	int col=-1;
	int f=-1;
	node_list[0].x=-1;
	node_list[0].y=-1;
	node_list[0].father=-1;
	set_avail();
	memcpy(tmp_avail_board,avail_board,sizeof(avail_board));
	memcpy(tmp_avail_len,avail_len,sizeof(avail_len));
	while(clock()-start<threshold){
		int choose=0;
		int tmp=select(choose,col);
		for(int i=0;i<10;i++){
			choose=tmp;
			board[node_list[choose].x][node_list[choose].y]=col;
            col=-col;
			choose=expansion(choose,col);
			double value=judge_value(-col);
			f=node_list[choose].father;
			while(f!=-1){
				node_list[choose].value+=value;
				node_list[choose].n++;
				board[node_list[choose].x][node_list[choose].y]=0;
				value=-value;
				choose=f;
				f=node_list[choose].father;
			}
			node_list[0].n++;
			col=-1;
			memcpy(avail_board,tmp_avail_board,sizeof(avail_board));
			memcpy(avail_len,tmp_avail_len,sizeof(avail_len));
		}
	}
    double max=-10000;
    int select=0;
	for(int i=0;i<node_list[0].nson;i++){
		int m=node_list[0].son[i];
		if(node_list[m].n!=0){
			double score=node_list[m].value/node_list[m].n;
			if(score>max){
				select=m;
				max=score;
			}
		}
	}
	new_x=node_list[select].x;
	new_y=node_list[select].y;
}
int count_step(){
	int step=0;
	for(int i=0;i<9;i++){
		for(int j=0;j<9;j++){
			if(avail_posi(i,j,-1))
				step++;
		}
	}
	return step;
}
void find_depth_limit(int step){
	if(step>49)
		depth_limit=7;
	else if(step>36)
		depth_limit=9;
	else if(step>25)
		depth_limit=11;
	else
		depth_limit=81;
}
int enumeration(int is_depth_1){
	for(int i=0;i<9;i++){
		for(int j=0;j<9;j++){
			if(avail_posi(i,j,enm_col)){
				if(is_depth_1 && new_x==-1){
					new_x=i;
					new_y=j;
				}
				board[i][j]=enm_col;
				enm_col=-enm_col;
				int res=enumeration(0);
				if(res==0){
					if(is_depth_1){
						new_x=i;
						new_y=j;
					}
					board[i][j]=0;
					enm_col=-enm_col;
					return 1;
				}
				board[i][j]=0;
				enm_col=-enm_col;
			}
		}
	}
	return 0;
}
void selectway(){
	int step=count_step();
	if(step>=10){
		find_depth_limit(step);
		mcts();
	}
	else
		enumeration(1);
}
int main(){
    srand((unsigned)time(0));
    srand(clock());
    string str;
    int x;
	int y;
    getline(cin,str);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str,input);
    int turnID=input["responses"].size();
    for(int i=0;i<turnID;i++){
        x=input["requests"][i]["x"].asInt();
        y=input["requests"][i]["y"].asInt();
        if(x!=-1)
            board[x][y]=1;
        x=input["responses"][i]["x"].asInt();
        y=input["responses"][i]["y"].asInt();
        if(x!=-1)
            board[x][y]=-1;
    }
    x=input["requests"][turnID]["x"].asInt();
    y=input["requests"][turnID]["y"].asInt();
    if(x!=-1)
        board[x][y]=1;
    Json::Value ret;
    Json::Value action;
    selectway();
    action["x"]=new_x;
    action["y"]=new_y;
    ret["response"]=action;
    Json::FastWriter writer;
    cout<<writer.write(ret)<<endl;
    return 0;
}
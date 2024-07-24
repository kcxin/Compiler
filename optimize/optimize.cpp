#include <bits/stdc++.h>
using namespace std;
#include "macro.h"

extern formula fms[N];
extern int nxtst;

struct block{
    map<piis,int> graph;    //DAG图
    vector<formula> fms;    //新生成的四元式
}blocks[N];
int nxtbl=1;    //指向下一个块的编号
vector<pii> blocks_divide;  //块在原四元式的左右区间
int fmsnum = 0; //划分块优化后的所有四元式总和

map<string,string> tmpid;   //重新给块内tmp编号
vector< set<int> > D,loop;  //必经节点集和循环内的块编号

/** 查找循环时用的图 **/
int head[N],tail[N];
int cnt;
struct edge{int v,next;}e[N]; //边集
void add(int u,int v,int* __ht) //存正向边或存反向边
{
    e[++cnt].v=v;
    e[cnt].next=__ht[u];
    __ht[u]=cnt;
}
void adde(int u,int v)  //添加边
{
    add(u,v,head);
    add(v,u,tail);
}

/** 识别临时变量 和 用于方便打印临时变量的函数 **/
extern bool istmp(string s);
extern string tmpPrefixTo_t(string s);

/** 打印 **/
void printBlocks()  //打印每个块
{
    for(int i=1;i<nxtbl;i++){
        cout<<"第"<<i<<"块"<<endl;
        int j=1;
        for(auto ite:blocks[i].fms)
            cout<<setw(3)<<j++<<setw(6)<<ite.op<<setw(4)<<tmpPrefixTo_t(ite.a)
                <<setw(4)<<tmpPrefixTo_t(ite.b)<<setw(4)<<tmpPrefixTo_t(ite.c)<<endl;
    }
    cout<<nxtst-1<<" => "<<fmsnum<<endl<<endl;
}
void printNewfms()
{
    for(int i=1,j=1;i<nxtbl;i++){
        for(auto ite:blocks[i].fms)
            cout<<setw(3)<<j++<<setw(6)<<ite.op<<setw(4)<<tmpPrefixTo_t(ite.a)
                <<setw(4)<<tmpPrefixTo_t(ite.b)<<setw(4)<<tmpPrefixTo_t(ite.c)<<endl;
    }
    cout<<endl;
}
void printDom()  //打印必经节点集
{
    for(int i=1;i<nxtbl;i++){
        cout<<"D["<<i<<"]:";
        for(auto ite:D[i])
            cout<<ite<<ends;
        cout<<endl;
    }
    cout<<endl;
}
void printLoop()  //打印每个循环
{
    int i=0;
    for(auto l:loop){
        cout<<"第"<<++i<<"个循环:";
        for(auto c:l) cout<<c<<ends;
        cout<<endl;
    }
    cout<<endl;
}

/** 局部优化 **/
template<typename T>    //模板类，对同类型的a和b做算术运算
    string operate(const string& op, const T& a, const T& b) {
    if(op=="+") return to_string(a+b);
    else if(op=="-") return to_string(a-b);
    else if(op=="*") return to_string(a*b);
    else if(op=="@") return to_string(-a);
    else if(op=="=d"||op=="d") return to_string(a);
}
void initBlock(int l,int r,int cur) //对块建图并生成新中间代码,cur为当前块编号
{
    auto& dst = blocks[nxtbl].fms;
    auto& graph = blocks[nxtbl++].graph;

    unordered_map<string,int> node;
    string v[N];               //节点值
    set<string> symbols[N];     //
    int nxtnd=1;

    bool isleaf[N];     //是否为叶子节点，初始化为1
    memset(isleaf,1,sizeof(isleaf));
    bool vis[N];    //一个节点只能复写传播一次
    memset(vis,0,sizeof(vis));

    auto insertNode = [&](int pos,string s){    //将s插入位置为pos的节点
        if(v[pos].empty()) v[pos] = s;
        else{
            if(istmp(v[pos])) swap(s,v[pos]);
            symbols[pos].insert(s);
        }
        node[s] = pos;
    };
    auto newNode = [&](string& s){  //返回s是否为数字，s为空修改s为0并返回1，s不存在则生成新节点
        if(s.empty()){
            s="0";
            return 1;
        }
        if(!node.count(s)) insertNode(nxtnd++,s);
        return isdigit(v[node[s]].front());
    };
    auto deleteNodes = [&](string op,string B,string C){    //删除新生成的节点
        node.erase(B);
        v[--nxtnd].clear();
        symbols[nxtnd].clear();
        if(op=="+"||op=="-"||op=="*"){
            node.erase(C);
            v[--nxtnd].clear();
            symbols[nxtnd].clear();
        }
    };

    auto emit = [&](formula f){    //生成四元式
        string& a=f.a ,&b=f.b ,&c=f.c;
        auto newId = [&](string& s){
            if(istmp(s)){
                string id = s.substr(1),newid;
                if(!tmpid.count(id)) tmpid[id] = to_string(tmpid.size()+1);
                s = string(1,tmpprefix) + tmpid[id];
            }
        };
        newId(a),newId(b),newId(c);
        dst.push_back(f);
        fmsnum++;
    };
    auto insertNodeFms = [&](int x){    //加入节点生成的四元式，复写传播
        if(!vis[x]){
            for(auto ite:symbols[x])
                if(v[x] != ite && !istmp(ite)) emit(formula("=",v[x],"",ite));
            vis[x]=1;
        }
    };

    insertNode(0,"0");
    for(int i=l;i<=r;i++){
        string A=fms[i].c,B=fms[i].a,C=fms[i].b,op=fms[i].op,P;
        bool isexist = !node.count(B) && !node.count(C);//B和C是否是此四元式新构造出来的节点
        bool flagtmp = newNode(B);
        bool isdig = (newNode(C)) && flagtmp && (op !="itf" && op !="fti");
        if(op.front()=='j') break;//留给下面处理
        if(op=="="){
            insertNode(node[B],A);
            continue;
        }
        if(isdig){
            B = v[node[B]],C = v[node[C]];//取数
            P = (B.find('.')!=string::npos)||(C.find('.')!=string::npos)?
                            operate(op,stof(B),stof(C)):operate(op,stoi(B),stoi(C));
            if(isexist) deleteNodes(op,B,C);
        }
        else{
            if( graph.count({{node[B],node[C]},op}) ){
                int foo = graph[ { { node[B],node[C] },op } ];
                insertNode(foo,A);
                continue;
            }
            else if( (op=="+" || op=="*") && graph.count({{node[C],node[B]},op}) ){
                int foo = graph[ { { node[C],node[B] },op } ];
                insertNode(foo,A);
                continue;
            }
        }
        if(node.count(A)){
            if(!isleaf[node[A]]) symbols[node[A]].erase(A);
        }
        else{
            if(isdig) newNode(P);
            else newNode(A);
        }
        if(isdig) insertNode(node[P],A);
        else{
            graph[{{node[B],node[C]},op}] = node[A];
            isleaf[node[A]]=0;
        }
    }

    v[0]="";
    vector< map<piis,int>::iterator > gtmp;
    for(auto ite = graph.begin();ite!=graph.end();ite++) gtmp.push_back(ite);
    sort(gtmp.begin(), gtmp.end(), [](auto a,auto b){return a->sc < b->sc;});
    for(auto g:gtmp){
        int a = g->ft.ft.ft, b = g->ft.ft.sc, c = g->sc;
        string op = g->ft.sc;
        insertNodeFms(a);
        insertNodeFms(b);
        emit(formula(op,v[a],v[b],v[c]));
        insertNodeFms(c);
    }
    for(int i=1;i<nxtnd;i++)
        if(!vis[i]) insertNodeFms(i);
    if(fms[r].op.front()=='j'){
        int pos = stoi(fms[r].c);
        if(pos==nxtst) fms[r].c = to_string(blocks_divide.size()+1);
        else fms[r].c = to_string( lower_bound(blocks_divide.begin(),blocks_divide.end()
                                                ,make_pair(pos,pos))-blocks_divide.begin()+1 );
        emit(formula(fms[r].op,v[node[fms[r].a]],v[node[fms[r].b]],fms[r].c));
        adde(cur,stoi(fms[r].c));
        if(fms[r].op!="jump") adde(cur,cur+1);
    }
    else adde(cur,cur+1);
}
void divideBlocks() //分块
{
    bool isentry[N];
    memset(isentry,0,sizeof(isentry));
    isentry[1] = 1; //入口语句是程序的第一个语句
    bool isexit[N];
    memset(isexit,0,sizeof(isexit));
    isexit[nxtst-1] = 1; //出口语句是程序的第一个语句
    for(int i=nxtst-1;i;i--)
        if(fms[i].op.front()=='j'){ //如果是转移语句
            isentry[stoi(fms[i].c)] = 1;    //入口语句是跳转的目标语句
            isexit[i] = 1;       //出口语句是跳转语句
            isexit[stoi(fms[i].c)-1] = 1;  //出口语句是跳转目标语句的前一个语句（词法序）
            if(fms[i].op!="jump"){  //如果是条件语句
                isentry[i+1] = 1;   //入口语句是条件跳转的下一条语句
            }
        }
    for(int i=1,l;i<nxtst;i++){ //划分左右区间
        if(isentry[i]) l=i;
        if(isexit[i]) blocks_divide.push_back(make_pair(l,i));
    }
    int i=1;
    for(auto block:blocks_divide) initBlock(block.ft,block.sc,i++);
}
void renewFms() //将每个块四元式再穿成串
{
    int i=1,j=1;
    for(i=1,j=1;i<nxtbl;i++){
        blocks_divide[i]={j,j+blocks[i].fms.size()-1};
        j += blocks[i].fms.size();
    }
    nxtst = j;
    for(i=1,j=1;i<nxtbl;i++){
        for(auto &ite:blocks[i].fms)
            if(ite.op.front()=='j')//改为跳转到块的开头四元式
                ite.c = (ite.c!=to_string(nxtbl))?to_string(blocks_divide[stoi(ite.c)].ft):to_string(nxtst);
    }
}

void initDom() //初始化必经节点集
{
    D.resize(nxtbl+1);
    D[1].insert(1);
    int cur=1;
    bool flag = 1;
    set<int> tmp;
    for(int i=1;i<nxtbl;i++) tmp.insert(i);
    for(int i=2;i<nxtbl;i++) D[i] = tmp;
    while(1){
        if(++cur == nxtbl){
            if(flag) break;
            cur = 2,flag = 1;
        }
        set<int> newset,tmp1;

        int i=tail[cur];
        tmp1 = D[e[i].v];
        i=e[i].next;
        for(;i;i=e[i].next){
            int v = e[i].v;
            set<int> tmp2;
            set_intersection(tmp1.begin(),tmp1.end(),
                                D[v].begin(),D[v].end(),
                                inserter(tmp2, tmp2.begin()));
            tmp1 = tmp2;
        }
        newset = tmp1;
        newset.insert(cur);
        if(newset != D[cur]) flag = 0;
        D[cur] = newset;
    }
}
void findLoop() //查找循环
{
    vector<pii> forwedge;
    loop.reserve(nxtbl);
    for(int i=1;i<nxtbl;i++){
        for(int j=head[i];j;j=e[j].next){
            int v = e[j].v;
            if(D[i].count(v)){
                forwedge.push_back({i,v});
            }
        }
    }
    cout<<"回边:"<<endl;for(pii eg:forwedge) cout<<eg.ft<<ends<<eg.sc<<endl;

    for(pii eg:forwedge){
        set<int> newloop;
        queue<int> q;
        newloop.insert(eg.ft),newloop.insert(eg.sc);
        q.push(eg.ft);
        while(!q.empty()){
            int n=q.front();
            q.pop();
            for(int i=tail[n];i;i=e[i].next){
                int m = e[i].v;
                if(n != eg.sc && !newloop.count(m)){
                    newloop.insert(m);
                    q.push(m);
                }
            }
        }
        loop.push_back(newloop);
    }
}

void optimize() //里面就调了调函数
{
    divideBlocks();
    cout<<"优化后";
    printBlocks();
    renewFms();
    printNewfms();

    initDom();
    printDom();
    findLoop();
    printLoop();
}

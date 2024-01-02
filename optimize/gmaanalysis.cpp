#include <bits/stdc++.h>
using namespace std;
#include "macro.h"

struct comp{//grammar按get排序
    bool operator ()(const grammar &a, const grammar &b){
        if(!a.isrepro()&&b.isrepro()) return 1;//a<b
        if(a.isrepro()&&!b.isrepro()) return 0;//a>b
        if(a.isrepro()&&b.isrepro()) return a.id<b.id;
        return
            a.get()!=b.get()?a.get()<b.get()
            :a.id!=b.id?a.id<b.id:
            a.pointpos<b.pointpos;
    }
};
struct State{   //判重排序
    multiset<pii> rules;
    State(set<grammar,comp> b){
        for(auto r:b) rules.insert(make_pair(r.id,r.pointpos));
    }
    bool operator <(State b)const{ return rules < b.rules; }
};

vector<grammar> rulearray;  //规则集
int num[128];       //符号编号
char alpha[128];    //编号对应的符号
int Vnsize;         //非终结符编号个数
int Vsize=1;        //符号编号个数
int Statesize=1;    //状态编号
set<char> First[N]; //First集
int GOACTION[N][60];
extern vector< piis > words[N];
extern priority_queue< pii,vector<pii>,greater<pii> > Errors;

/**打印**/
void printLexString();
void printfms();
void printRule(grammar r)
{
    cout<<r.id<<':';
    r.right.insert(r.pointpos==-1?0:r.pointpos,"·");
    cout<<r.left<<" -> "<<r.right<<" | ";
    for(auto c:r.symbols) cout<<c<<ends;
    cout<<endl;
}
void printState(set<grammar,comp> s,int cnt)
{
    cout<<"State"<<cnt-1<<':';
    for(auto r:s) printRule(r);
}
void printFIRST()
{
    freopen("output/First.txt","w",stdout);
    for(int i=1;i<Vsize;i++){   //打印FIRST集
        cout<<"First("<<alpha[i]<<"):";
        for(auto s:First[alpha[i]]) cout<<s<<ends;
        cout<<endl;
    }

    fclose(stdout);//关闭文件
    freopen(dst.c_str(),"w",stdout);
    cout.clear();
}
void printGOTO(set<grammar,comp> I[])
{
    freopen("output/Goto.txt","w",stdout);

    for(int i=0;i<Statesize;i++){
        printState(I[i],i+1);
        cout<<"含边:";
        for(int j=0;j<128;j++){
            if(num[j]&&GOACTION[i][num[j]]){
                cout<<alpha[num[j]]<<ends;
                if(GOACTION[i][num[j]]==acc) cout<<"acc";
                else if(GOACTION[i][num[j]]>0) cout<<"S"<<GOACTION[i][num[j]];
                else cout<<"r"<<-GOACTION[i][num[j]];
                cout<<endl;
            }
        }
        cout<<endl;
    }

    fclose(stdout);//关闭文件
    freopen(dst.c_str(),"w",stdout);
    cout.clear();
}
void printStack(vector<int>& stack_states,vector<char>& stack_symbols,vector<piis>& stack_place)
{
    cout<<"状态栈：";
    for(auto v:stack_states) cout<<setw(4)<<v<<ends ;
    cout<<endl<<"符号栈：";
    for(auto v:stack_symbols) cout<<setw(4)<<alpha[v]<<ends ;cout<<endl;
    cout<<endl<<"位置栈：";
    for(auto v:stack_place) cout<<setw(4)<<v.ft.ft<<ends ;cout<<endl<<'\t';
    for(auto v:stack_place) cout<<setw(4)<<v.ft.sc<<ends ;cout<<endl<<'\t';
    for(auto v:stack_place) cout<<setw(4)<<v.sc<<ends ;cout<<endl;
}
/**初始化**/
void initRuleSet()
{
    freopen("input/grammar.txt","r",stdin);

    string l,r;
    rulearray.push_back(grammar("Z","Y",set<char>({'#'})));//0号规则

    num['Z']=Vsize,alpha[Vsize++]='Z';
    while(cin>>l>>r){
        rulearray.push_back(grammar(l,r,set<char>({'#'})));
        if(!num[l[0]]) num[l[0]]=Vsize,alpha[Vsize++]=l[0];//加入非终结符编号
    }
    Vnsize=Vsize;//非终结符编号个数

    num['`']=-1,num['i']=-1,num['d']=-1,num['#']=-1;
    //错误符号，标识符，数字
    for(int i=0;i<128;i++) //加入终结符编号
        if(num[i]==-1) num[i]=Vsize,alpha[Vsize++]=(char)i;

    sort(++rulearray.begin(),rulearray.end());
    for(int i=0;i<rulearray.size();i++){
        rulearray[i].id=i;//规则编号
    }
    fclose(stdin);//关闭文件
    freopen("CON","r",stdin);
    cin.clear();

//printRuleSet
    freopen("output/RuleSet.txt","w",stdout);
    for(auto r:rulearray) printRule(r);

    fclose(stdout);//关闭文件
    freopen("CON","w",stdout);
    cout.clear();
    //printRuleSet
}
void initFirst()
{
    bitset<N> dp[N];
    for(auto rule:rulearray) dp[num[rule.left[0]]][num[rule.right[0]]] = 1;//建可达矩阵
    for(int i=Vnsize;i<Vsize;i++)  dp[i][i]=1;      //Vt

    for(int k=1;k<Vnsize;k++)  //非终结符求传递闭包
        for(int i=1;i<Vnsize;i++)
            if(dp[i][k]) dp[i]|=dp[k];
                                    //V入First集
    for(int i=1;i<Vsize;i++)      //V
        for(int j=Vnsize;j<Vsize;j++)    //Vt
            if(dp[i][j]) First[alpha[i]].insert(alpha[j]);

    printFIRST();
}
/**打表**/
set<grammar,comp> clousure(queue<grammar> &I)
{
    set<grammar,comp> res;
    auto cmp = [](const grammar &a, const grammar &b){return a.get()<b.get();};//get比较
    while(!I.empty()){
        auto rule=I.front();
        I.pop();
        char v=rule.get();
        if(!rule.isrepro()){//下一个是非终结符
            set<char> newsymbols= rule.pointpos+1!=rule.right.size() ?
                                First[rule.right[rule.pointpos+1]]:rule.symbols;//根据后跟是否为空

            auto ite=res.find(rule);
            if( ite != res.end() ){    //规则已存在
                set<char>& s1=rule.symbols,s2=(*ite).symbols;
                bool flag=includes(s2.begin(),s2.end(),s1.begin(),s1.end());

                if( flag && (s1.size()<=s2.size()) ) continue;//存在包含关系且之前规则个数更多
                else{//更新以前规则
                    set<char> tmp;
                    set_union(s1.begin(),s1.end(),
                               s2.begin(),s2.end(),
                                inserter(tmp, tmp.begin()));
                    (set<char>&) ite->symbols = tmp;
                }
            }else res.insert( rule );  //加入这个规则的项目

            auto V_range=equal_range(++rulearray.begin(),rulearray.end(),v);
            for(auto i=V_range.ft;i!=V_range.sc;i++){//从规则表里加入新的项目，待处理
                I.push( grammar(*i,newsymbols) );
            }

        }else res.insert(rule);
    }
    return res;

}
void initGoto()
{

    set<grammar,comp> I[N];//状态，每个状态是集合
    multimap<State,int> isexsit;//状态存在
    char u,v;
    int isrep,repeatedI,pos;

    auto mergable = [](auto range,auto I,auto newI,int& repeatedI){ //俩个状态能否合并
        for(auto ite=range.ft;ite!=range.sc;ite++){//遍历找到的每个相同的情况
            int flag=1;     //合并标记 0/1/2 不能/能/完全相同
            repeatedI=ite->sc;
            for(auto i=I[ite->sc].begin(),j=newI.begin();j!=newI.end();i++,j++){//遍历规则的符号集
                set<char> result;
                set_intersection(i->symbols.begin(),i->symbols.end(),
                                j->symbols.begin(),j->symbols.end(),
                                inserter(result, result.begin()));

                if(result.size()==i->symbols.size()&&   //完全相同，进行下一规则
                   result.size()==j->symbols.size()) flag=2; //规则完全相同
                else if(!result.empty()){ flag=0;break;}  //规则交集不为空，状态不能合并
            }
            if( flag == 1 ) return 1;   //规则交集都为空，状态能合并
            else if(flag == 2) return 2;   //只要存在规则完全相同的，就不再判断
        }
        return 0;
    };


    queue< set<grammar,comp> > Q;
    queue<grammar> q;
    q.push({grammar(rulearray[0],set<char>({'#'}))});
    I[0]=clousure(q);
    Q.push(I[0]);
    while(!Q.empty()){ //遍历每个状态
        auto cur = Q.front();
        Q.pop();
        u=(*cur.begin()).get();   //划分前一个项目
        auto j=cur.begin(),k=cur.begin();//指针初始化

        State tmp=State(cur);   //找当前状态在状态集的位置
        auto range=isexsit.equal_range(tmp);
        for(auto ite=range.ft;ite!=range.sc;ite++){
            if(I[ite->sc] == cur){
                pos = ite->sc;
                break;
            }
        }

        while(1){   //快慢指针
            auto rule=*j;
            v=rule.get();   //划分后一类项目
            if(!rule.isrepro() && v==u){//项目rule过边u
                if(j==k) k++;   //防止j超过k
                j++;
                q.push(grammar(rule,rule.symbols));//GO（I,u）
            }

            if(rule.isrepro() || k==cur.end() || v!=u){
                set<grammar,comp> newI;
                if(q.empty()) break;
                newI=clousure(q);//求clousure
                repeatedI = 1;

                State newState=State(newI);
                auto range=isexsit.equal_range(newState);
                if(range.ft!=range.sc){  //重复
                    if( isrep=mergable(range,I,newI,repeatedI) ){//1或2
                        if(isrep == 2)  {if(!GOACTION[pos][num[u]]) GOACTION[pos][num[u]]=repeatedI;}//优先规约
                        else{
                            for(auto i=I[repeatedI].begin(),j=newI.begin();j!=newI.end();i++,j++){
                                    //newI合并到I[tmp]
                                set<char> s1=(*i).symbols,s2=(*j).symbols,tmp;
                                set_union(s1.begin(),s1.end(),
                                                s2.begin(),s2.end(),inserter(tmp, tmp.begin()));
                                    //更新GOACTION
                                if(j->isrepro())
                                    for(char c:j->symbols) GOACTION[repeatedI][num[c]]=-(*i).id;
                                const_cast<grammar&>(*i).symbols=tmp;
                            }
                        }
                    }
                }
                if(range.ft==range.sc || !isrep ){ //加入状态 不重或都不能合并
                    isexsit.insert(make_pair(newState,Statesize));
                    if(!GOACTION[pos][num[u]]) GOACTION[pos][num[u]]=Statesize;//优先规约

                    for(grammar rule:newI){
                        if(rule.isrepro())
                            for(char c:rule.symbols)
                                GOACTION[Statesize][num[c]]=-rule.id;
                    }

                    I[Statesize++]=newI;  //保存状态，范围加一
                }

                if(range.ft==range.sc || isrep != 2) Q.push(newI);
                if(rule.isrepro()) break;   //剪枝
                if(k!=cur.end()) k++;
                else break;
                u=v;
                while(!q.empty()) q.pop();
            }
        }
    }
    GOACTION[0][num['#']]=GOACTION[1][num['#']]=acc;

    printGOTO(I);
}
/**语法分析**/
int generateCode(vector<piis>& stack_place,int ruleid);
bool gmaanalysis()
{
    initRuleSet();
    initFirst();
    initGoto();
    int flag=0;    //是否出错标志
    int line=1,next=0;
    int X;
    char a;
    auto getNextChar=[&](){
        while( next == words[line].size() || words[line].empty() ) line++,next=0;
        return words[line][next].ft.sc;
    };
    printLexString();

    vector<int> stack_states;
    vector<char> stack_symbols;
    vector<piis> stack_place;
    stack_states.push_back(0),stack_symbols.push_back(num['#']);    //语法分析
    stack_place.push_back(npos);    //记录在Words入口地址
    while(1){
            printStack(stack_states,stack_symbols,stack_place);
        X=stack_states.back();  //状态栈栈顶
        a=num[getNextChar()];

        if(GOACTION[X][a] == acc) break;
        else if(GOACTION[X][a]>0){  //移进
            stack_states.push_back(GOACTION[X][a]);
            stack_symbols.push_back(a);
            stack_place.push_back({{line,next},""});
            next++;
        }
        else if(GOACTION[X][a]<0){  //规约
            string r=rulearray[-GOACTION[X][a]].right;
            for(auto ite=r.rbegin();ite!=r.rend();ite++){   //检查能否正确规约
                if(*ite != alpha[stack_symbols.back()]) break;
                stack_symbols.pop_back();
                stack_states.pop_back();
            }

            cout<<"弹出完"<<endl;printStack(stack_states,stack_symbols,stack_place);
            cout<<endl<<"规约成 "<<rulearray[-GOACTION[X][a]].left[0];

            stack_symbols.push_back(num[rulearray[-GOACTION[X][a]].left[0]]);//非终结符入栈
            stack_states.push_back(GOACTION[stack_states.back()][stack_symbols.back()]);//查GO表入栈

            cout<<endl<<"状态栈栈底 "<<stack_states.back();
            cout<<endl<<"符号栈栈底 "<<alpha[stack_symbols.back()];
            cout<<endl<<"状态压入 "<<GOACTION[stack_states.back()][stack_symbols.back()];

            if(!flag){
                flag = generateCode(stack_place,-GOACTION[X][a]);      //生成中间代码
                if(flag) Errors.push({line,flag});     //有错就记录
            }
            cout<<endl<<"规约完"<<endl;printStack(stack_states,stack_symbols,stack_place);
        }
        else{   //出错
            Errors.push({line,4});
            flag=1;      cout<<"往前退栈："<<endl;
            while( !isupper(alpha[stack_symbols.back()]) && alpha[stack_symbols.back()] != '#'){
                stack_symbols.pop_back();
                stack_states.pop_back();
            }           cout<<"往后省略：";
            while( !GOACTION[ stack_states.back() ][ num[getNextChar()] ]
                    && getNextChar() != '#' ) next++;
            if(alpha[stack_symbols.back()] == '#' || getNextChar() == '#') break;
            cout<<endl<<"恐慌后"<<endl;printStack(stack_states,stack_symbols,stack_place);
        }
    }
    if(!flag) cout<<"Yes"<<endl;
    else cout<<"No"<<endl;

    printfms();

    return !flag;
}

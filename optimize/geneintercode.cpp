#include <bits/stdc++.h>
using namespace std;
#include "macro.h"

formula fms[N];
int nxtst=1;    //指向下一条四元式编号
map<string,size_t> place;//查找变量名在符号表的入口地址或临时变量编码
int newtmp=1;   //下一个临时变量的编号
extern vector< piis > words[N];
extern vector<grammar> rulearray;//规则集

/** 识别临时变量 和 用于方便打印临时变量的函数 **/
bool istmp(string s) { return s.front() == tmpprefix; } //s是否为临时变量
string tmpPrefixTo_t(string s){  //返回值将临时变量的前缀修改为 't'
    if(istmp(s)) return "t"+s.substr(1);
    return s;
}

/** 打印 **/
void printRule(grammar r);//打印规则
void printfms()//打印四元式
{
    for(int i=1;i<nxtst;i++)
        cout<<setw(3)<<i<<' '<<setw(6)<<fms[i].op
        <<setw(4)<<tmpPrefixTo_t(fms[i].a)<<setw(4)<<tmpPrefixTo_t(fms[i].b)
        <<setw(4)<<tmpPrefixTo_t(fms[i].c)<<endl;
    cout<<endl;
}

/** 生成中间代码 **/
int merge(int p1,int p2)    //拉链，p2的链尾改为p1
{
    if(!p2) return p1;
    string tmp;
    while(!((tmp = fms[p2].c).empty())) p2=stoi(tmp);
    fms[p2].c = to_string(p1);
    return p2;
}
void backpatch(int p,string t)  //回填，把链首p所连接的每个四元式的第四区段改为t
{
    string& c = fms[p].c;
    if(!c.empty() && p) backpatch(stoi(c),t);
    c = t;
}
int generateCode(vector<piis>& stack_place,int ruleid) //传入位置栈和规则编号
{
    //先退栈存入临时栈，方便后续入栈
    auto sz = rulearray[ruleid].right.size();
    vector<piis> stack_place_tmp(sz);
    for(int i=sz-1;~i;i--) {
        stack_place_tmp[i] = stack_place.back();
        stack_place.pop_back();
    }
    auto ite = --stack_place_tmp.end();//指向临时栈最后一个字符

    //以下是用到的lambda表达式，分配和查找临时空间、变量、临时变量
    auto getWord = [&](pii ite) -> piis& { return words[ite.ft][ite.sc]; }; //根据行列得到语法分析存的单词
    auto getName = [&](pii ite) -> string& { return getWord(ite).name; };   //根据行列得到单词的原始名
    auto getType = [&](pii ite) -> pii& { return getWord(ite).inner; };     //根据行列得到单词的类型

    auto getTmp = [](int pos) -> piis& { return words[0][pos]; };           //根据临时空间位置查找存的内容
    auto getTmpName = [](int pos) -> string& { return words[0][pos].name; };//根据位置查找存的变量名或临时变量名
    auto getNewTmpName = [](){ return words[0].back().sc; };    //返回新生成的临时空间里存的变量名或临时变量名
    auto getNewTmpPlace = [](){ return words[0].size()-1; };    //返回新生成的临时空间地址
    auto newTmp = [&](pii type,string name="") -> piis&{   //生成一个type类型的变量,不传名字为临时变量,返回所存内容
        if(name.empty()) name = string(1,tmpprefix)+to_string(newtmp++);
        place[name] = words[0].size();

        words[0].push_back( make_pair( type, name ) );//默认是编号，需要时为变量名
        return words[0].back();
    };

    auto typeCast = [&](piis& D1,piis& D2,int type = 5){     //将D1和D2转成同型,不同默认转为浮点
        if(D1.var != D2.var){     //类型判断
                newTmp(make_pair(type,0));                  //申请空间
                auto& Dcast = D1.var != type ? D1 : D2;   //判断谁需要转型
                string op = Dcast.var==4 ? "itf" : "fti";//转型为另一种类型
                fms[nxtst++]=formula( op, getTmpName(Dcast.plc), "", getNewTmpName() );
                Dcast = {{type,getNewTmpPlace()},Dcast.codebegin};
            }
    };
    auto varAssign = [&](){     //变量赋值
        auto i = getName((ite-3)->pos);
        auto itmp = getTmp(place[i]);
        piis D=*(ite-1);
        if(place[i]){   //如果声明则生成赋值语句
            typeCast(itmp,D,itmp.var);  //转型
            fms[nxtst++]=formula( "=", getTmpName(D.plc), "", i );
        }
        else return 6;
    };


    /*
        id、d 行-列-原字符，B 类型-简化字-空，D 类型-变量地址-codebegin, L 上个变量地址-变量地址-变量名
        E 真链-假链-codebegin   S C T K 链-空-codebegin     W 空-空-codebegin
    */
    switch (ruleid)
    {
        case 33:{          //S -> A
            stack_place.push_back({{0,0},ite->codebegin});
            break;
        }
        case 34:{          //S -> B;
            stack_place.push_back({{0,0},(ite-1)->codebegin});
            break;
        }
        case 2:{         //变量赋值语句 A -> i=D;
            stack_place.push_back({{0,0},(ite-1)->codebegin});
            varAssign();
            break;
        }
        case 23:{        //多个变量命名 L -> L,i
            auto i = getWord(ite->pos);
            piis L;
            if(!place[i.name]) L = newTmp({(ite-2)->plc, getNewTmpPlace()+1}, i.name);//没声明则新增变量
            else return 5;
            stack_place.push_back(L);
            break;
        }
        case 24:{        //变量命名   L -> i
            auto i = getWord(ite->pos);
            piis L;
            if(!place[i.name]) L = newTmp({0, getNewTmpPlace()+1}, i.name);    //没声明则新增变量
            else return 5;
            stack_place.push_back(L);
            break;
        }
        case 3: {       //变量声明语句 B -> BL
            auto B = *(ite-1), L = *ite;
            do{
                getType({0,L.plc}) = B.inner;
                L = getTmp(L.var);
            }while(L != npos);
            stack_place.push_back(B);
            break;
        }
        case 4:case 5:{ //类型确定 B -> l | B -> z
            stack_place.push_back({getType(ite->pos),to_string(nxtst)});
            break;
        }


        case 7:{         //D -> (D)
            stack_place.push_back(*(ite-1));
            break;
        }
        case 8:{        //D -> - D
            auto Dneg = newTmp(ite->inner);
            fms[nxtst++]=formula( "@", getTmpName(ite->plc), "", getNewTmpName() );
            stack_place.push_back({{Dneg.var,getNewTmpPlace()},ite->codebegin});
            break;
        }
        case 9:case 10:{//D -> D*D  |  D -> D+D
            auto D1=*(ite-2),D2=*ite;
            typeCast(D1,D2);
            auto Dres = newTmp(D1.inner);     //乘除
            fms[nxtst++]=formula( getName((ite-1)->pos), getTmpName(D1.plc)
                                 , getTmpName(D2.plc), getNewTmpName() );
            stack_place.push_back({{D1.var,getNewTmpPlace()},D1.codebegin});

            break;
        }
        case 11:{        //字面量赋值   D -> d
            auto d = newTmp(getType(ite->pos));
            stack_place.push_back({{d.var,place[d.name]},d.name});

            fms[nxtst++]=formula( "=d", getName(ite->pos), "", getNewTmpName() );
            break;
        }
        case 12:{       //使用变量 D -> i
            auto i = getName(ite->pos);
            auto itmp = getTmp(place[i]);
            piis D;
            if(place[i])  D = {{itmp.var,place[i]}, to_string(nxtst)};    //有声明则取到变量地址
            else return 6;
            stack_place.push_back(D);
            break;
        }


        case 13:{       //E -> !E
            swap(ite->tc,ite->fc);
            break;
        }
        case 14:{       //E -> (E)
            stack_place.push_back(*(ite-1));
            break;
        }
        case 15:{       //E -> E and E
            auto E1 = *(ite-2), E2 = *ite;
            backpatch(E1.tc,E2.codebegin);
            stack_place.push_back({ { E2.tc, merge(E1.fc,E2.fc) }, E1.codebegin });
            break;
        }
        case 16:{       //E -> E or E
            auto E1 = *(ite-2), E2 = *ite;
            backpatch(E1.fc,E2.codebegin);
            stack_place.push_back({ { merge(E1.tc,E2.tc), E2.fc }, E1.codebegin });
            break;
        }
        case 17:case 18:case 19:{       //E -> dRd | E -> iRd |  E -> iRi
            auto dti = [&](auto ite){   //字面量转变量
                piis D = getWord(ite->pos);
                if(isdigit(D.name.front())){
                    newTmp(D.inner);
                    fms[nxtst++]=formula( "=d", D.name, "", getNewTmpName() );
                    D.name = getNewTmpName();
                }else D.var=getTmp(place[D.name]).var;
                D = {{D.var,place[D.name]},D.name};
                return D;
            };
            auto D1 = dti(ite-2),D2 = dti(ite);
            typeCast(D1,D2);
            int code_begin = nxtst;
            fms[nxtst++]=formula( "j"+(ite-1)->name, D1.name, D2.name, "" );//真链
            fms[nxtst++]=formula( "jump", "", "", "" );          //假链
            stack_place.push_back({{nxtst-2,nxtst-1},to_string(code_begin)});
            break;
        }
        case 25:case 26:case 27:case 28:case 29:case 30:case 31:case 32:{   //R -> rop
            stack_place.push_back({{0,0},getName(ite->pos)});
            break;
        }


        case 6:{        //if单分支 C -> pE
            auto& E = *ite;
            backpatch(E.tc,to_string(nxtst));
            stack_place.push_back({{E.fc,0},E.codebegin});
            break;
        }
        case 35:{       //S -> C{S}
            stack_place.push_back({ { merge((ite-3)->chain,(ite-1)->chain), 0 }, (ite-3)->codebegin });
            break;
        }
        case 40:{       //if双分支 T -> C{S}e
            stack_place.push_back({ { merge(nxtst,(ite-2)->chain), 0 }, (ite-4)->codebegin});
            fms[nxtst++]=formula( "jump", "", "", "" );
            backpatch((ite-4)->chain,to_string(nxtst));
            break;
        }
        case 39:{       //S -> T{S}
            stack_place.push_back({ { merge((ite-3)->chain,(ite-1)->chain), 0 }, (ite-3)->codebegin});
            break;
        }


        case 37:{       //while循环 S -> K{S}
            backpatch((ite-1)->chain,to_string(nxtst));
            fms[nxtst++]=formula( "jump", "", "", (ite-3)->codebegin );
            stack_place.push_back({ { (ite-3)->chain, 0 }, (ite-3)->codebegin});
            break;
        }
        case 22:{       //K -> WE
            backpatch(ite->tc,to_string(nxtst));
            stack_place.push_back({ { ite->fc, 0 }, (ite-1)->codebegin});
            break;
        }
        case 41:{       //W -> w
            stack_place.push_back({ { 0, 0 }, to_string(nxtst)});
            break;
        }


        case 20:{        //F -> $(A
            stack_place.push_back({ { 0, 0 }, ite->codebegin});
            break;
        }
        case 21:{        //G -> FE;i=D)
            varAssign();    //同变量赋值语句 A -> i=D;
            fms[nxtst++]=formula( "jump", "", "", (ite-5)->codebegin );
            backpatch((ite-5)->tc,to_string(nxtst));
            stack_place.push_back({{ (ite-5)->fc, stoi((ite-1)->codebegin) }, (ite-6)->codebegin});
            break;
        }
        case 36:{        //S -> G{S}
            backpatch((ite-1)->chain,to_string(nxtst));
            fms[nxtst++]=formula( "jump", "", "", to_string((ite-3)->prevbegin) );
            backpatch((ite-3)->chain,to_string(nxtst));
            stack_place.push_back({ { 0, 0 }, (ite-3)->codebegin});
            break;
        }


        case 38:{       //S -> SS
            auto S1 = *(ite-1),S2 = *ite;
            backpatch((ite-1)->chain,ite->codebegin);
            stack_place.push_back({ { ite->chain, 0 }, (ite-1)->codebegin});
            break;
        }
        case 42:{       //Y -> zm(){S}
            backpatch((ite-1)->chain,to_string(nxtst));
            stack_place.push_back(npos);
            break;
        }
        default:{
            stack_place.push_back(npos);
            break;
        }
    }
    return 0;
}

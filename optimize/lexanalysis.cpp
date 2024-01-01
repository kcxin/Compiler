#include <bits/stdc++.h>
using namespace std;
#include "macro.h"

unordered_map<string,pic > typemap_pspic;   //字符类别表 <种类，简化字符>
vector<string> errorarray;                  //错误表

vector< piis > words[N];        //N行单词的 < <种类，简化字符> , 原字符 >
priority_queue< pii,vector<pii>,greater<pii> > Errors;     //行数，错误类别 --按值查找，保持原来顺序

extern int num[];   //符号编号
int line=1;         //行号

char getNextChar(bool blankflag,bool lineflag)//读不读空白符，行数增不增加
{
    char cur;
    do{cur=cin.get();}
        while((cur=='\t' || cur==' '|| lineflag && (cur=='\n'&&line++))
                            && !blankflag && cur!=EOF);
    return cur;
}
/**初始化**/
void initTypeMap()
{
    freopen("input/type.txt","r",stdin);
    int type[]={0,1,3,4,5,6};
    int len[]={0,9,14,1,1,8};
    string s;char c;
    for(int i=1;i<=5;i++) {
        for(int j=1;j<=len[i];j++){
            cin>>s;
            c=getNextChar(0,0);//简化字符
            typemap_pspic[s]=make_pair(type[i],c);
            num[c]=-1;//标记终结符，语法分析用到
        }
    }

    fclose(stdin);//关闭文件
    freopen("CON","r",stdin);
    cin.clear();
}
void initErrorArray()
{
    string filename="input/error.txt";
    string tmp;
    fstream file;
    file.open(filename.c_str());
    errorarray.push_back("");
    while(getline(file,tmp)) errorarray.push_back(tmp);
    file.close();
}
/**词法分析**/
void lexAnalysis()
{
    char cur;
    string word;
    pic type;//类别 简化符号
    int state,errortype;
    bool blankflag,ignflag,intflag;
    size_t base,digit;//判断常数越界，实现自动截取

    auto initState=[&](){
        type=make_pair(0,'`');
        state=0;
        ignflag=0;//不读标记
        blankflag=0;//读不读空格的标记 0不读，1读
        intflag=0;//读取是否为整型标志
        errortype=0;//0为无错
        base=1;
        digit=0;
        word.clear();
    };
    auto fallback=[&](){//回退
        cin.putback(cur);
        ignflag=1;//不读且不存
    };
    auto getType=[&](){
        if(state==ErrorState) return make_pair(0,'`');
        if(isdigit(word[0]))
            if(intflag) return make_pair(4,'d');
            else return make_pair(5,'d');
        return typemap_pspic[word].ft?
                    typemap_pspic[word]:make_pair(2,'i');//关键字或标识符
    };

    initState();
    initTypeMap();

    freopen(src.c_str(),"r",stdin);
    while((cur=getNextChar(blankflag,1))!=EOF){
        switch(state)
        {
            case 0:
                switch(cur)
                {
                    case '+':case '-':case '*':case '/':
                    case '(':case ')':case '{':case '}':
                    case '.':case ',':case ';':case ':':
                        state=FinishState;
                        break;
                    case '>':case '<':case '!':case '=':
                        state=1;
                        break;
                    case '&':
                        state=2;
                        break;
                    case '|':
                        state=3;
                        break;
                    default:
                        if(isdigit(cur)) state=4,intflag=1;
                        else if(isalpha(cur)) state=6;
                        else state=ErrorState,errortype=1;//未知字符
                }
                break;
            case 1:
                if(cur!='=') fallback();
                state=FinishState;
                break;
            case 2:
                if(cur=='&') {state=FinishState;}
                else fallback(),state=ErrorState,errortype=2;//'&&'或'||'输入有误
                break;
            case 3:
                if(cur=='|') {state=FinishState;}
                else fallback(),state=ErrorState,errortype=2;
                break;
            case 4:
                if(isdigit(cur)) state=4;
                else if(cur=='.'){
                    if(isdigit(cin.peek())) state=5;//检查小数位
                    else state=ErrorState,errortype=3;//小数位为空
                    intflag=0;
                }
                else{
                    fallback();
                    state=FinishState;
                }
                break;
            case 5:
                if(isdigit(cur)) state=5;
                else state=FinishState;
                break;
            case 6:
                if(isalnum(cur)){
                    state=6;
                }
                else{
                    fallback();
                    state=FinishState;
                }
                break;
            default:break;

        }
        switch(state)
        {
            case ErrorState:
                Errors.push(make_pair(line,errortype));
            case FinishState:
                if(!ignflag) word.push_back(cur);//忽视，不读入
                type=getType();
                if(intflag){
                    for(auto ite=word.rbegin();ite!=word.rend();ite++){
                        digit+=base*(*ite-'0');
                        base*=10;
                    }
                    words[line].push_back(make_pair(type,to_string(digit)));
                }
                else words[line].push_back(make_pair(type,word));
                initState();
                break;
            default:
                blankflag=1;
                word.push_back(cur);

                break;
        }
    }
    words[line].push_back(make_pair(make_pair(1,'#'),"#"));
    words[0].push_back(make_pair(make_pair(0,0),""));

    fclose(stdin);//关闭文件
    freopen("CON","r",stdin);
    cin.clear();
}

/**打印**/
bool printErrors()
{
    if(Errors.empty()) return 0;
    initErrorArray();
    for(;!Errors.empty();Errors.pop()){
        auto ite=Errors.top();
        cout<<"[ERROR]第"<<ite.ft<<"行:["<<ite.sc<<"]";
//        printf("[ERROR]第%d行:[%d]",ite.ft,ite.sc);
        cout<<ends<<errorarray[ite.sc]<<endl;
    }
    return 1;
}
void printWords()
{
    cout<<"\n  Type        |  Words\n";
    cout<<"\n---------------------------------\n";
    string translation[7]={"错误单词","保留字  ","标识符  ",
        "运算符  "," 整型   "," 浮点   ","其他界符"};
    for(int i=1;i<=line;i++){
        for(auto word:words[i]){
            cout<<"| "<<translation[word.ft.ft]<<"    |  ";
            cout<<word.sc<<"    "<<(char)word.ft.sc<<endl;
        }
    }
}
void printLexString()
{
    for(int i=1;i<=line;i++)
        for(auto word:words[i]) cout<<(char)word.ft.sc;
    cout<<endl;
}

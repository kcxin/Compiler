#ifndef MACRO_H_INCLUDED
#define MACRO_H_INCLUDED

#define ft first
#define sc second
#define FinishState 0x3f3f3f3f
#define ErrorState 0x3f3f3f3f+1
#define acc 0x3f3f3f3f+3

#define var ft.ft
#define pos ft
#define inner ft
#define plc ft.sc
#define name sc

#define chain ft.ft
#define prevbegin ft.sc
#define codebegin sc
#define tc ft.ft
#define fc ft.sc

#define tmpprefix '~'
typedef pair<int,int> pii;
typedef pair<int,char> pic;
typedef pair< pii,string > piis;
typedef pair< pii,int > piii;
typedef pair< pic,string > pics;
typedef pair<piis,int> piisi;
const int N=1e3;
const auto npos = make_pair(make_pair(0,0),string(""));

extern string src,dst;
struct grammar{ //语法产生式
    string left,right;//左部 右部
    int id; //规则编号
    set<char> symbols; //标识符
    int pointpos;

    grammar(string l,string r,set<char> s):left(l),right(r),
        symbols(s),pointpos(-1){}

    grammar(grammar b,set<char> s):left(b.left),right(b.right),id(b.id),
        symbols(s),pointpos(b.pointpos+1){}

    bool operator <(const grammar b)const{  //按左部
        return left!=b.left?left<b.left:
                right!=b.right?right<b.right:
                 symbols!=b.symbols?symbols<b.symbols:
                  pointpos!=b.pointpos?pointpos<b.pointpos:0;
    }
    friend bool operator <(const grammar a,const char b){   //按左部比较，查找区间时用
        return a.left[0]<b;
    }
    friend bool operator <(const char a,const grammar b){   //按左部比较，查找区间时用
        return a<b.left[0];
    }
    bool operator ==(const grammar b)const{
        return left==b.left&&right==b.right&&symbols==b.symbols&&pointpos==b.pointpos;
    }
    bool isrepro()const{ return pointpos == right.size(); } //是否是规约项目
    char get()const{ return right[pointpos]; }              //取pos位置字符
};
struct formula{ //四元式
    string op;
    string a,b;
    string c;
    formula(string op,string a,string b,string c):op(op),a(a),b(b),c(c){};
    formula(){};
};

#endif // MACRO_H_INCLUDED

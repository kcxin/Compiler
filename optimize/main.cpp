#include <bits/stdc++.h>
using namespace std;
#include "Functions.h"

string src,dst;
int main()
{
    ios::sync_with_stdio(0);
    cin.tie(0),cout.tie(0);

//    string n;cin>>n;
//    string src = "input/code"+n+".txt";   //选择第n个代码文件

    src = "input/code1.txt";    //指定代码文件
//    dst = "CON";              //输出到命令行
    dst = "output/result.txt";  //输出到文件

    freopen(dst.c_str(),"w",stdout);
    lexAnalysis();
    printWords();
//    printLexString();

    if( gmaanalysis() ) optimize();


    printErrors();

    return 0;
}

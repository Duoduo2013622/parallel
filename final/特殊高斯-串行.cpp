#include<iostream>
#include<fstream>
#include<stream>
#include <ctime>
#include <ratio>
#include <chrono>
using namespace std;
const int colnmu=85401;//列数
const int line_e=756; //被消元行数
int byte_num=(colnmu-1)/32+1;   //每个实例中的byte型数组数
class bit_matrix{
public:
    int mycolnmu;    //首项
    int *mybyte;    
    bit_matrix(){    //初始化
        mycolnmu=-1;
        mybyte = new int[byte_num];
        for(int i=0;i<byte_num;i++)
            mybyte[i]=0; 
    }
    bool isnull(){  //判断当前行是否为空行
        if(mycolnmu==-1)return 1;
        return 0;
    }   
    void insert(int x){ //数据读入
        if(mycolnmu==-1)mycolnmu=x;
        int a=x/32,b=x%32;
        mybyte[a]|=(1<<b);
    }
    void doxor(bit_matrix b){  //两行做异或操作，由于结果留在本实例中，只有被消元行能执行这一操作,且异或操作后要更新首项
        for(int i=0;i<byte_num;i++)
            mybyte[i]^=b.mybyte[i];
        for(int i=byte_num-1;i>=0;i--)
            for(int j=31;j>=0;j--)
                if((mybyte[i]&(1<<j))!=0){
                    mycolnmu=i*32+j;
                    return;
                }
        mycolnmu=-1;  
    }
};
bit_matrix *eliminer=new bit_matrix[colnmu];
bit_matrix *eline=new bit_matrix[line_e];
void indata(){
    ifstream ifs;
    ifs.open("eliminer1.txt");  //消元子
    string temp;
    while(getline(ifs,temp)){
        istringstream s(temp);
        int x;
        int T=0;
        while(s>>x){
            if(!T)T=x;    //第一个读入元素代表行号
            eliminer[T].insert(x);
        }
    }
    ifs.close();
    ifstream ifs2;
    ifs2.open("eline1.txt");     //被消元行,读入方式与消元子不同
    int T=0;
    while(getline(ifs2,temp)){
        istringstream s(temp);
        int x;
        while(s>>x){
            eline[T].insert(x);
        }
        T++;
    }
    ifs2.close();
}
void t_guass_simple(){  //消元
    for(int i=0;i<line_e;i++){
        while(!eline[i].isnull()){  //只要被消元行非空，循环处理
            int tcolnmu = eline[i].mycolnmu;  //被消元行的首项
            if(!eliminer[tcolnmu].isnull())    //如果存在对应消元子
                eline[i].doxor(eliminer[tcolnmu]);
            else{
                eliminer[tcolnmu]=eline[i];    //由于被消元行升格为消元子后不参与后续处理，可以直接用=来浅拷贝
                break;
            }
        }
    }
}
void printres(){ //打印结果
    for(int i=0;i<line_e;i++){
        if(eline[i].isnull()){puts("");continue;}   //空行的特殊情况
        for(int j=byte_num-1;j>=0;j--){
            for(int k=31;k>=0;k--)
                if((eline[i].mybyte[j]&(1<<k))!=0){     //一个错误调了半小时，谨记当首位为1时>>不等于除法！
                    printf("%d ",j*32+k);
                }
                }
        puts("");
    }
}
int main(){
    indata();
    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    t_guass_simple();
     high_resolution_clock::time_point t2 = high_resolution_clock::now();
    std::cout<<"serial: "<<duration_cast<duration<double>>(t2-t1).count()<<std::endl;
    //printres();
    system("pause");
    return 0;
}
#include<iostream>
using namespace std;
#define MAX_SIZE 100
struct SqList{
   int data[MAX_SIZE];
   int length;
};

int insertorder(SqList&L,int x){
    if(L.length >= MAX_SIZE){
        return 0;
    }
    int i = L.length - 1;
    while(i >= 0 && L.data[i] > x){
        L.data[i + 1] = L.data[i];
        i--;
    }
    L.data[i + 1] = x;
    L.length++;
    return 1;
}
int main(){
    SqList L;
    L.length = 7;
    for(int i = 0; i < L.length; i++){
        L.data[i] = i*5;
    }
    cout<<"当前顺序表元素为："<<endl;
    for(int i = 0;i<L.length;++i){
        cout<<L.data[i]<<" ";
    }
    cout<<endl;
    int x;
    cout<<"要插入的元素：";
    cin>>x;
    insertorder(L, x);
    cout<<"插入后的顺序表为：";
    for(int i = 0; i < L.length; i++){
        cout<<L.data[i]<<" ";
    }
    cout<<endl;
   return 0;
}

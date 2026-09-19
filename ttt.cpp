#include<iostream>
#include<vector>
#include<ctime>
#include<cstdlib>
#include<string>
using namespace std;
template <class T>
void myswap(T &a,T &b){
    T temp = a;
    a = b;
    b = temp;
}
template <typename T,typename Cmp>
void mysort(vector<T> &v,int n,Cmp cmp){
    for(int i=0;i<n-1;i++){
        for(int j=0;j<n-i-1;j++){
            if(cmp(v[j],v[j+1])){
                myswap(v[j],v[j+1]);
            }
        }
    }
} 

class person{
    public:
        string name;
        int age;
    public:
    person(string n,int a):name(n),age(a){}
    void getinfo(){
        cout<<"Name: "<<name<<", Age: "<<age<<endl;
    }

};


int main(){
    srand(time(nullptr));
    vector<person> v;
    v.push_back(person("Alice",30));
    v.push_back(person("Bob",25));
    v.push_back(person("Charlie",35));
    cout<<"Before sorting:"<<endl;
    for(auto &p:v){
        p.getinfo();
    }
    mysort(v, v.size(),[](const person &a, const person &b) {
        return a.age > b.age; 
    });
    cout<<"After sorting:"<<endl;
    for(auto &p:v){
        p.getinfo();
    }
    vector<char>c;
    int n;
    cout<<"Enter the number of characters to sort: ";
    cin>>n;
    if(n<=0){
        cout<<"Invalid number of characters."<<endl;
        return 1;
    }
    for(int i=0;i<n;i++){
        c.push_back('a'+(rand()%26));
    }
    cout<<"Before sorting:"<<endl;
    for(const auto &ch:c){
        cout<<ch<<" ";
    }
    cout<<endl;
    mysort(c,c.size(),[](const char &a, const char &b) {
        return a > b; 
    });
    cout<<"After sorting:"<<endl;
    for(const auto &ch:c){
        cout<<ch<<" ";
    }
    cout<<endl;
    
    
    return 0;
}

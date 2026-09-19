#include<iostream>
using namespace std;
class Myvector{
    public:
    int* data;
    int capacity;//最大容量
    int size;//当前已储存的元素个数

    Myvector(){
        this->capacity=10;
        this->size=0;
        data=new int[capacity];
    }
    Myvector(const Myvector& v){
        this->capacity = v.capacity;
        this->size = v.size;    
        data = new int[capacity];
        for(int i = 0; i < size; i++){
            data[i] = v.data[i];
        }
    }
    ~Myvector(){
        delete[] data;
    }
    Myvector& operator=(const Myvector& v){
        Myvector temp(v);
        swap(this->data, temp.data);
        swap(this->capacity, temp.capacity);
        swap(this->size, temp.size);
        return *this;
    }

    public:
    void push_back(int val){
        if (capacity == 0) capacity = 1;
        if(size == capacity){
            capacity *= 2;
            int* newdata = new int[capacity];
            for(int i = 0; i < size; i++){
                newdata[i] = data[i];
            }
            delete[] data;
            data = newdata;
        }
        data[size] = val;
        size++;
    }
    int getsize() const{
        return size;
    }   
    int getcapacity() const{
        return capacity;
    }
    int& at(int index){
        if(index < 0 || index >= size){
            throw out_of_range("Index out of range");
        }
        return data[index];
    }
    int& operator[](int index){
        return data[index];
    }
};

int main() {
    
    Myvector v1;
    for(int i = 0; i < 15; i++){
        v1.push_back(i);
    }
    for(int i = 0; i < v1.getsize(); i++){
        cout << v1[i] << " ";
    }
    cout << endl;


    return 0;
}
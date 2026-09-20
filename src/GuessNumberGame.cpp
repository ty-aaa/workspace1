#include<iostream>
#include<ctime>
#include<cstdlib>
#include <limits>
#include <cctype>
using namespace std;

class GuessNumberGame{
    public:
        
        
        void run(){
            srand(static_cast<unsigned>(time(0)));
            bool flag = true;
            cout<<"want to play a game? y or n"<<endl;
            string t;
            cin>>t;
            clearInput();


            if(t=="y"||t=="Y"){
                flag = true;
                while(flag){
                    bool valid = false;
                    while(!valid){
                        cout<<"choose gamemode:easy,medium,hard or sd (self-define):"<<endl;
                    
                        cin>>t;
                        clearInput();

                        valid = SetDiff(t);
                    }
                    playoneround();
                    flag = askagain();
                    
                }
            }else{
                cout<<"Bye!"<<endl;
            }
        }
    private:
        int maxnum = 500,maxtry = 10,target = 0,tries = 0;
        void setNandT(){
            cout << "Enter max number and max tries: ";
            cin >> maxnum >> maxtry;
            clearInput();
        }
        
        void clearInput() {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        bool SetDiff(string s){
            for (char &c : s) {
                c = tolower(static_cast<unsigned char>(c));
            }   
           
            if (s == "e" || s == "easy") {
            // 简单模式
                maxnum = 100;
                maxtry = 15;
            } else if (s == "m" || s == "medium") {
            // 中等模式
                maxnum =  500;
                maxtry = 10;
            } else if (s == "h" || s == "hard") {
            // 困难模式
                maxnum = 1000;
                maxtry = 7;
            } else if(s=="sd"){
            //自定义模式
                setNandT();
            }else{
                cout<<"no such mode"<<endl;
                return false;
            }
            return true;
        }
        void playoneround(){
            tries = 0;
            target = rand()%maxnum+1;
            bool win = false;
            cout<<"game start"<<endl;
            cout<<"please enter a number bt 1 and "<<maxnum<<endl;
            while(tries<maxtry){
                ++tries;
                int n = -1;
                cin>>n;
                clearInput();
                if(n<1||n>maxnum){
                    cout<<"please enter the number whthin the range"<<endl;
                    cout<<maxtry-tries<<" times left"<<endl;
                    continue;
                }
                if(n<target){
                    cout<<"too small,enter again"<<endl;
                    cout<<maxtry-tries<<" times left"<<endl;
                }else if(n>target){
                    cout<<"too big,enter again"<<endl;
                    cout<<maxtry-tries<<" times left"<<endl;
                }else{
                    cout<<"congratulations! you guessed the number right by "<<tries<<" times"<<endl;
                    win = true;
                    break;
                }
               
                    
                
            }
            if(!win)cout<<"unfortunately,the game is over,the target is "<<target<<endl;

        }
        bool askagain(){
            
            cout<<"again or end? enter y or n"<<endl;
            string s;
            cin>>s;
            clearInput();
            return (s=="y"||s=="Y");
             
        }
    
};
int main(){
    GuessNumberGame g;
    g.run();
return 0;

}
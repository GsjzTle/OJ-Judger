#include<bits/stdc++.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/resource.h>
#include<sys/socket.h>
#include<arpa/inet.h>
using namespace std;

std::chrono::system_clock::time_point begin_time;
std::chrono::system_clock::time_point end_time;

const int NAME_LEN = 15;

string strip(string s){
    string t = "";
    int n = s.size();
    if(!n) return "";
    int i = 0;
    for(i = 0 ; i < n ; i ++){
        if(s[i] == '\n' || s[i] == ' ' || s[i] == '\t') continue ;
        else break ;
    }
    int j = n - 1;
    for(j = n - 1 ; j >= 0 ; j --){
        if(s[j] == '\n' || s[j] == ' ' || s[j] == '\t') j --;
        else break ;
    }
    for(int k = i ; k <= j ; k ++) t += s[k];
    return t;
}
void Kill(int sig){
    abort();
}
class Judger{
    private :
        string judgername;
        string data_in;
        string data_out;
        int timelimit;
        int memorylimit;
        string language;
        string code;
    public :
        Judger(int t_limit, int m_limit, string judge, string din , string dout, string la, string c){
            this->judgername = judge;
            this->data_in = din;
            this->data_out = dout;
            this->timelimit = t_limit;
            this->memorylimit = m_limit;
            this->language = la;
            this->code = c;
            fstream fout(judgername + ".cpp", ios::out);
            fout << c;
            fout.close();
            /*
                fstream fin("ac.cpp" , ios::in);
                string code = "";
                while(!fin.eof()){
                    string tmp = "";
                    getline(fin, tmp);
                    tmp += '\n';
                    code += tmp;
                }
                fin.close();
            */
        }
        pair<bool, string> compiledCpp(){
            string name = this->judgername;
            string cmd = "timeout 10 g++ " + name + ".cpp -fmax-errors=3 -o " + name + " -O2 -std=c++11 2> " + name + "_ce.txt";
            int res = system(cmd.c_str());
            if(!res) return make_pair(true, "");
            fstream fin(name + "_ce.txt", ios::in);
            if(!fin) return make_pair(false, "");
            string msg = "";
            while(!fin.eof()){
                string tmp;
                getline(fin, tmp);
                tmp += '\n';
                msg += tmp;
            }
            fin.close();
            return make_pair(false, msg);
        }
        string runCpp(){
            pid_t pid_runcpp = vfork();
            if(pid_runcpp == 0) {
                rlimit limit;
                limit.rlim_cur = limit.rlim_max = 1;
                setrlimit(RLIMIT_CPU, &limit);
                freopen(data_in.c_str(), "r" , stdin);
                freopen((judgername + ".out").c_str(), "w", stdout);
                char* argv[] = {(char*)"" , nullptr};
                begin_time = std::chrono::system_clock::now();
                signal(SIGALRM, Kill);  // 设置一个定时器如果程序在规定时间（真实时间而不是cpu调用时间）内还没结束，就kill它
                alarm(3);
                int res = execvp(("./" + judgername).c_str(), argv);
                return "运行失败！";
            } else{
                int status_run = 0;
                struct rusage use;
                int wait_pid = wait4(pid_runcpp, &status_run, 0, &use);
                end_time = std::chrono::system_clock::now();
                int tot_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - begin_time).count();
                cout << "运行时间 = " << tot_time << "ms" << '\n';
                std::ifstream input1(data_out);
                std::ifstream input2(judgername + ".out");
                string res = "";
                if(WIFEXITED(status_run)) {
                    while(1){
                        string ans = "" , u_out = "";
                        getline(input1 , ans);
                        getline(input2 , u_out);
                        ans = strip(ans);
                        u_out = strip(u_out);
                        if(ans == "" && u_out == "") {
                            res = "AC";
                            break;
                        }
                        if(ans != u_out){
                            res = "WA";
                            break;
                        }
                    }
                }
                if(WIFSIGNALED(status_run)) {
                    switch WTERMSIG(status_run) {
                    case SIGXCPU:   // TLE
                        res = "TLE";
                        break;
                    case SIGKILL:   // TLE
                        res = "TLE";
                        break;
                    case SIGXFSZ:   // OLE
                        res = "OLE";
                        break;
                    default:        // RE
                        res = "RE";
                        break;
                    }
                }
                return res;
            }
        }
        string GO(){
            if(language == "C++"){
                pair<bool , string>res = this->compiledCpp();
                if(res.first){
                    return this->runCpp();
                }
                else{
                    return "CE";
                /*
                    if(res.second == "") return "编译超时！或许您使用了过大的空间\n";
                    else return res.second;
                */
                }
            }
            return "Other languages are not supported for the time being";
        }
};
signed main(){

    int s_fd = socket(AF_INET , SOCK_STREAM , 0);

    struct sockaddr_in s_addr , c_addr;
    socklen_t c_len = sizeof(c_addr);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(9926);
    s_addr.sin_addr.s_addr = inet_addr("192.168.0.115");

    int res = bind(s_fd, (struct sockaddr*)&s_addr, sizeof(s_addr));
    listen(s_fd, 128); // 将套接字变为监听套接字、同时设置连接上限 接待连接请求
    perror("");
    int cfd = accept(s_fd , (struct sockaddr*)&c_addr , &c_len); // 连接套接字、通信套接字
    while(1){
        char buf[BUFSIZ * 2];
        int len = read(cfd, buf, sizeof(buf)); // 阻塞函数
        if(!len) break;
        buf[strlen(buf)] = '\0';
        string info = "";
        vector<string>rec;
        for(int i = 0 ; i < len ; i ++){
            if(i + 2 < len && buf[i] == '|' && buf[i + 1] == '#' && buf[i + 2] == ')'){
                rec.push_back(info);
                info = "";
                i += 2;
            }
            else info += buf[i];
        }
        rec.push_back(info);
        if(rec.size() != 7) {
            cout << "data error!\n";
            continue ;
        }
        for(auto i : rec) cout << i << '\n';
        Judger judger(stoi(rec[0]) , stoi(rec[1]), rec[2] + "_" + rec[3] + "_" + rec[4], "data1.in", "data1.out", rec[5], rec[6]);
        string res = judger.GO();
        cout << res << '\n';
        write(cfd, res.c_str(), strlen(buf));
    }
    close(cfd);
    return 0;
}
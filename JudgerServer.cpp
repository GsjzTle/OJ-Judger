#include<bits/stdc++.h>
#include <mariadb/mysql.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
using namespace std;

class MySql{
	private:
		MYSQL *conn;
		MYSQL_RES *res;   //结果集
		string ip; //数据库服务器地址
		string user;         //用户名
		string pwd;    //密码
		string db;    //数据库名
		unsigned int port;	
	public:
		MySql(){
			this->ip = "localhost"; this->user = "root"; this->pwd = "root"; this->db = "OJ"; this->port = 3306;
			conn = mysql_init(NULL);
			if(!conn){
				cout << "数据库初始化失败\n";
				exit(1);
			}
			cout << "数据库初始化成功\n";
		}
		MySql(string ip, string user, string pwd, string db, int port){
			this->ip = ip; this->user = user; this->pwd = pwd; this->db = db; this->port = port;
			conn = mysql_init(NULL);
			if(!conn){
				cout << "数据库初始化失败\n";
				exit(1);
			}
			cout << "数据库初始化成功\n";
		}
		~MySql(){
			mysql_close(this->conn);
		}
		void mysql_connect(){
			if(!mysql_real_connect(conn, ip.c_str(), user.c_str(), pwd.c_str(), db.c_str(), port, NULL, 0)){
				cout << "数据库连接失败\n";
				exit(1);
			}
			cout << "数据库连接成功\n";
		}
		vector<vector<string>> query(string sql){		
			vector<vector<string>>rows;
			if(!sql.size()) return rows;
			int tmp = mysql_query(this->conn, sql.c_str());          //执行sql语句
    		if(tmp) return rows;							// 返回值为 0 表示执行成功，否则表示执行失败
			this->res = mysql_store_result(this->conn);    //将查询结果装进MYSQL_RES
    		if(this->res == nullptr) return rows;
			int rows_num = mysql_num_rows(this->res);      //获取结果行数
			int cels_num = mysql_num_fields(this->res);     //获取结果列数 
			while(rows_num --){
				MYSQL_ROW cel = mysql_fetch_row(this->res);  //从结果集中获取一行
				vector<string>row;
				for(int i = 0 ; i < cels_num ; i ++) row.push_back(cel[i]);
				rows.push_back(row);
			}
			mysql_free_result(res);                       //查询完要记得释放
			return rows;
		}
		bool update(string sql){
			return mysql_query(this->conn, sql.c_str());          //执行sql语句
		}
		bool updateById(int id, string field, string result){
			string sql = "update user set " + field + " = '" + result + "' where id = " + to_string(id) + ";"; 
			return mysql_query(this->conn, sql.c_str());
		}

};
MySql my;
queue<vector<string>>que;
pthread_mutex_t mut;
void TCP(){
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in s_addr;
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(9926);
    s_addr.sin_addr.s_addr = inet_addr("192.168.0.115");
    while(connect(fd, (struct sockaddr*)&s_addr, sizeof(s_addr))){
		perror("");
		cout << "连接评测机失败，正在尝试重新连接\n";
		sleep(1);
	}
	cout << "连接评测机成功\n";
	while(1){
		sleep(2);
		while(que.size()){
			pthread_mutex_lock(&mut);
			vector<string>vec = que.front();
			que.pop();
			string info = "";
			for(int i = 0 ; i < vec.size() ; i ++){
				info += vec[i];
				if(i != vec.size() - 1) info += "|#)";
			}
			write(fd, info.c_str(), info.size());
			char buf[BUFSIZ];
			read(fd, buf, sizeof(buf));
			buf[strlen(buf)] = '\0';
			cout << buf << '\n';
			string res = buf;
			cout << res << '\n';
			if(res == "AC") res = "2";
			else if(res == "WA") res = "3";
			else if(res == "TLE") res = "4";
			else if(res == "MLE") res = "5";
			else if(res == "RE") res = "6";
			else if(res == "CE") res = "7";
			string sql = "update judger_status set result = " + res + " where id = " + vec[2] + ";"; 
			cout << sql << '\n';
			my.update(sql);
			pthread_mutex_unlock(&mut);
		}
	}
}
void getStatus(){
	while(1){
		sleep(3);
		pthread_mutex_lock(&mut);
		vector<vector<string>>rows = my.query("select id, pid, uid, language, code from judger_status where result = 0 order by submittime asc;");
		for(auto &i : rows) {
			vector<vector<string>>tm_limit = my.query("select time, memory from problem_problem where id = " + i[1]+ ";");
			vector<string>task; // time、memory、pid、uid、language、code 
			for(auto j : tm_limit[0]) task.push_back(j);
			for(auto j : i) task.push_back(j);
			que.push(task);
			for(auto j : task) cout << j << '\n';
		}
		my.update("update judger_status set result = 1 where result = 0");
		pthread_mutex_unlock(&mut);

	}
}
signed main()
{
	my.mysql_connect();
	pthread_mutex_init(&mut, nullptr);
	thread t1(getStatus);
	thread t2(TCP);
	t1.join();
	t2.join();
	pthread_mutex_destroy(&mut);
    return 0;
}
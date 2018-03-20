#include "stdafx.h"
#include <sstream>
#include <string>

#include <iostream>
#include <fstream>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")



struct getRecive {
	double X1;
	double X2;
	double X3;
};

struct Response {
	int type;
	double x1, x2, x3;
};

#define DEFAULT_PORT 5557
#define CONNECTION_QUEUE 100
#define _WIN32_WINNT 0x501
#define M_PI (3.141592653589793)
#define M_2PI (2.*M_PI)

using namespace std;


bool parse_cmd(int argc, char* argv[], char* host, short* port);
void handle_connection(SOCKET, sockaddr_in*);
void error_msg(const char*);
void exit_handler();
getRecive geterRecive(char* reciv);
int pos(char *s, char *c, int n);
int getX(char *sX, char *reciv);
Response Cubic(getRecive&);


SOCKET server_socket;
int main(int argc, char* argv[])
{

	SetConsoleOutputCP(65001);
	atexit(exit_handler);
	short port = DEFAULT_PORT;
	char host[128] = "";
	bool parse_cmd_result = parse_cmd(argc, argv, host, &port);

	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws)) {
		error_msg("Error init of WinSock2");
		return -1;
	}

	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket <= 0) {
		error_msg("Can't create socket");
		return -1;
	}

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	if (parse_cmd && strlen(host) > 0) {
		server_addr.sin_addr.s_addr = inet_addr(host);
	}
	else {
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	
	//Bind socket to the address on the server
	if (bind(server_socket, (sockaddr*)&server_addr, sizeof(sockaddr))) {
		char err_msg[128] = "";
		sprintf(err_msg, "Can't bind socket to the port %d", port);
		error_msg(err_msg);
		return -1;
	}

	if (listen(server_socket, CONNECTION_QUEUE)) {
		error_msg("Error listening socket");
		return -1;
	}

	printf("Server running at the port %d\n", port);

	while (true)
	{
		sockaddr_in incom_addr;
		memset(&incom_addr, 0, sizeof(incom_addr));
		int len = sizeof(incom_addr);
		//SOCKET socket = accept(server_socket, (sockaddr*)&incom_addr, &len);
		//if (socket <= 0) {
		//	error_msg("Can't accept connection");
		//	return -1;
		//}

		//
		SOCKET socket = accept(server_socket, NULL, NULL);
		if (socket == INVALID_SOCKET) {
			cerr << "accept failed: " << WSAGetLastError() << "\n";
			closesocket(server_socket);
			WSACleanup();
			return 1;
		}
		handle_connection(socket, &incom_addr);
	}
	closesocket(server_socket);

    return 0;
}

bool parse_cmd(int argc, char* argv[], char* host, short* port)
{
	if (argc < 2) {
		return false;
	}

	char all_args[256];
	memset(all_args, 0, sizeof all_args);
	
	for (int i = 1; i < argc; ++i) {
		strcat(all_args, argv[i]);
		strcat(all_args, " ");
	}

	
	const int count_vars = 3;
	const int host_buf_sz = 128;
	int tmp_ports[count_vars] = { -1, -1, -1 };
	char tmp_hosts[count_vars][host_buf_sz];
	for (int i = 0; i < count_vars; ++i) {
		memset(tmp_hosts[i], 0, host_buf_sz);
	}
	char* formats[count_vars] = { "-h %s -p %d", "-p %d -h %s", "-p %d" };
	
	int results[] = {
		sscanf(all_args, formats[0], tmp_hosts[0], &tmp_ports[0]) - 2,
		sscanf(all_args, formats[1], &tmp_ports[1], tmp_hosts[1]) - 2,
		sscanf(all_args, formats[2], &tmp_ports[2]) - 1
	};

	for (int i = 0; i < sizeof(results) / sizeof(int); ++i) {
		if (!results[i]) {
			if (strlen(tmp_hosts[i]) > 0) {
				strcpy(host, tmp_hosts[i]);
			}
			if (tmp_ports[i] > 0) {
				*port = (short)tmp_ports[i];
				return true;
			}
		}
	}

	return false;
	
}

void handle_connection(SOCKET socket, sockaddr_in* addr) {
	

	char* str_in_addr = inet_ntoa(addr->sin_addr);
	const int max_client_buffer_size = 1024;
	char buf[max_client_buffer_size];
	printf("[%s]>>%s\n", str_in_addr, "Establish new connection");
	while (true) {
		char buffer[256] = "";
		int rc = recv(socket, buffer, sizeof(buffer), 0);
		if (rc > 0) {
			printf("[%s]:%s\n", str_in_addr, buffer);
		
			std::stringstream response; // сюда будет записываться ответ клиенту
			std::stringstream response_body; // тело ответа

			string indexHtml; // сюда будем класть считанные строки
			ifstream file("index.html"); // файл из которого читаем (для линукс путь будет выглядеть по другому)

			while (getline(file, indexHtml)) { // пока не достигнут конец файла класть очередную строку в переменную (s)
				response_body << indexHtml;
			}
			file.close();
			//response_body << "<br><br>"<<buffer<< "<br><br>"<<buffer[21] << "<br><br>" << buffer[22];
			getRecive getR;
			getR = geterRecive(buffer);

			if (getR.X1 != 0 && getR.X2 != 0 && getR.X3 != 0) {

				buf[rc] = '\0';
				response_body << "<br><br> x1 = " << getR.X1 << " x2 = " << getR.X2 << " x3 = " << getR.X3;
				Response resp = Cubic(getR);
				switch (resp.type)
				{
				case 2: response_body << "<br>1 real root + complex roots imaginary part is zero:<br>" << resp.x1 << "<br>" << resp.x2 << "<br>" << resp.x3;
					break;
				case 1: response_body << "<br>1 real root + 2 complex:<br>" << resp.x1 << "<br>" << resp.x2 << "<br>" << resp.x3;
					break;
				case 3: response_body << "<br>3 real roots:<br>" << resp.x1 << "<br>" << resp.x2 << "<br>" << resp.x3;
					break;
				default:
					response_body << "err response";
					break;
				}
			}
			

			response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Length: " << response_body.str().length() <<"\r\n" 
				<< "Content-Type: text/html; charset=utf-8\r\n"
				<<"Content-Type: text/html\r\n\r\n"
				<< response_body.str();

			int sc = send(socket, response.str().c_str(), response.str().length(), 0);
			if (rc <= 0)
			{
				printf("Can't send response to client %s", str_in_addr);
				break;
			}		
		}
		else {
			break;
		}
	}
	closesocket(socket);
	printf("[%s]>>%s", str_in_addr, "Close incomming connection");
}

getRecive geterRecive(char reciv[100]) {
	getRecive result;
	//const char * с = reciv;
	char *sX1 = "x1=";
	char *sX2 = "x2=";
	char *sX3 = "x3=";

	result.X1 = getX(sX1, reciv);
	result.X2 = getX(sX2, reciv);
	result.X3 = getX(sX3, reciv);
	cout << "x1 = " << result.X1 << endl;
	cout << "x2 = " << result.X2 << endl;
	cout << "x3 = " << result.X3 << endl;

	return result;
}

int getX(char *sX, char *reciv) {
	int n = 0;
	int resultX = 0;
	char s1 = '&';
	char s2 = ' ';


	for (int i = 1; n != -1; i++)
	{
		n = pos(reciv, sX, i);
		if (n >= 0) {
			n = n + 3;
			if (reciv[n + 1] == '&' || reciv[n + 1] == ' ') {
				resultX = (int)reciv[n] - 48;
			}
			else {
				resultX = ((int)reciv[n] - 48)*10 + ((int)reciv[n+1] - 48);
			}
			/*cout << " X1 = " << resultX << endl;
			cout << n << endl;*/
		}
	};
	return resultX;
}

int pos(char *s, char *c, int n)
{
	int i, j;		// Счетчики для циклов
	int lenC, lenS;	// Длины строк

					//Находим размеры строки исходника и искомого
	for (lenC = 0; c[lenC]; lenC++);
	for (lenS = 0; s[lenS]; lenS++);

	for (i = 0; i <= lenS - lenC; i++) // Пока есть возможность поиска
	{
		for (j = 0; s[i + j] == c[j]; j++); // Проверяем совпадение посимвольно
											// Если посимвольно совпадает по длине искомого
											// Вернем из функции номер ячейки, откуда начинается совпадение
											// Учитывать 0-терминатор  ( '\0' )
		if (j - lenC == 1 && i == lenS - lenC && !(n - 1)) return i;
		if (j == lenC)
			if (n - 1) n--;
			else return i;
	}
	//Иначе вернем -1 как результат отсутствия подстроки
	return -1;
}

Response Cubic(getRecive& rq) {
	double q, r, r2, q3;
	Response res;
	q = (rq.X1*rq.X1 - 3.*rq.X2) / 9.; r = (rq.X1*(2.*rq.X1*rq.X1 - 9.*rq.X2) + 27.*rq.X3) / 54.;
	r2 = r*r; q3 = q*q*q;
	if (r2<q3) {
		double t = acos(r / sqrt(q3));
		rq.X1 /= 3.; q = -2.*sqrt(q);
		res.x1 = q*cos(t / 3.) - rq.X1;
		res.x2 = q*cos((t + M_2PI) / 3.) - rq.X1;
		res.x3 = q*cos((t - M_2PI) / 3.) - rq.X1;
		res.type = 3;
		return res;
	}
	else {
		double aa, bb;
		if (r <= 0.) r = -r;
		aa = -pow(r + sqrt(r2 - q3), 1. / 3.);
		if (aa != 0.) bb = q / aa;
		else bb = 0.;
		rq.X1 /= 3.; q = aa + bb; r = aa - bb;
		res.x1 = q - rq.X1;
		res.x2 = (-0.5)*q - rq.X1;
		res.x3 = (sqrt(3.)*0.5)*fabs(r);
		if (res.x3 == 0.) {
			res.type = 2;
			return res;
		}
		res.type = 1;
		return res;
	}
}

void error_msg(const char* msg) {
	printf("%s\n", msg);
}

void exit_handler()
{
	closesocket(server_socket);
	WSACleanup();
}
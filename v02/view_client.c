/*====================================================================
   TCP Client
   - 원격 파일 보기 프로그럄 클라이언트 
   - 동작 과정 
    . Tx: 파일 이름 전송 
	. Rx: 서버로 부터 파일 내용 수신 후 화면 출력 
=======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100

// cmd type 
#define FILE_REQ 1
#define FILE_RES 2
#define FILE_END 3
#define FILE_END_ACK 4
#define FILE_NOT_FOUND 5 

typedef struct {
	int cmd; 
	int buf_len; // 실제 전송되는 파일 내용의 크기 저장 
	char buf[BUF_SIZE+1];
}PACKET;

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	char fname[BUF_SIZE];
	int str_len;
	socklen_t addr_size;
	//PACKET recv_packet, send_packet;
	PACKET send_packet, recv_packet;	// 순서 변경 
	struct sockaddr_in serv_addr;
	int total_rx_cnt=0, total_rx_bytes=0;

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET, SOCK_STREAM, 0);   
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	// PACKET memory initialize
	memset(&recv_packet, 0, sizeof(PACKET));
	memset(&send_packet, 0, sizeof(PACKET));
	memset(fname, 0, sizeof(char) * BUF_SIZE);
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error!");

	printf("Input file name: ");
	scanf("%s", fname);
	
	/*-----------------------------------------------------------
	              FILE_REQ 
	      Client  ------->  Server
	           (buf: file name)
	*-----------------------------------------------------------*/
	send_packet.cmd = FILE_REQ;
	send_packet.buf_len = strlen(fname);
	strncpy(send_packet.buf, fname, send_packet.buf_len); 
	
	write(sock, (void*)&send_packet, sizeof(PACKET));

	printf("[Tx] cmd: %d, file name: %s\n", send_packet.cmd, send_packet.buf);	

	while(1) {

		read(sock, (void*)&recv_packet, sizeof(PACKET));

		if(recv_packet.cmd == FILE_RES)
		{
			total_rx_cnt++;
			total_rx_bytes += recv_packet.buf_len;
			recv_packet.buf[recv_packet.buf_len] = '\0';	// added by csjung
			printf("%s", recv_packet.buf);
		}
		else if(recv_packet.cmd == FILE_NOT_FOUND)
		{
			printf("[Rx] cmd: %d, %s: File Not Found\n", recv_packet.cmd, fname);
			break;

		}
		else if(recv_packet.cmd == FILE_END)
		{
			total_rx_cnt++;
			total_rx_bytes += recv_packet.buf_len;
			recv_packet.buf[recv_packet.buf_len] = '\0';	// added by csjung
			printf("%s\n", recv_packet.buf);	

			printf("---------------------------\n");
			printf("[Rx] cmd: %d, FILE_END \n", recv_packet.cmd);
			/*-----------------------------------------------------------
					  FILE_END_ACK 
			  Client  ------->  Server
			*-----------------------------------------------------------*/
			send_packet.cmd = FILE_END_ACK;
			write(sock, (void*)&send_packet, sizeof(PACKET));
			printf("[Tx] cmd: %d, FILE_END_ACK\n", send_packet.cmd);
			break;
		}
		else
		{
			printf("[Rx] Invalid cmd: %d\n", recv_packet.cmd);
		}

		memset(&recv_packet, 0, sizeof(PACKET));
		memset(&send_packet, 0, sizeof(PACKET));

	}
	printf("------------------------------------\n");
	printf("Total Rx count: %d, bytes: %d\n", total_rx_cnt, total_rx_bytes);
	printf("TCP Client Socket Close!\n");
	printf("------------------------------------\n");
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

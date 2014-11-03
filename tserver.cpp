#include <iostream>
#include <string>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <regex.h>
#include <glib.h>


#define	BUFF_SIZE		512
#define	SOCKET_BUFF_SIZE	512

// returned character array must be freed with g_free()
char *keyReply(const char *key){
	static char hex[]="0123456789abcdef";
	int l;
	char *cat, *sha1, *base64;

	cat=g_strconcat(key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", NULL);
	sha1=g_compute_checksum_for_string(G_CHECKSUM_SHA1, cat, strlen(cat));
	g_free(cat);

	l=strlen(sha1)/2;
	for(int i=0; i<l; ++i){
		sha1[i]=((strchr(hex, sha1[2*i])-hex)<<4)+
				(strchr(hex, sha1[2*i+1])-hex);
	}
	base64=g_base64_encode((guchar *)sha1, l);
	g_free(sha1);

	return base64;
}

int main(int argc, char *argv[]){
	int			w, s, n, len, i, first=1;
	char			buff[BUFF_SIZE], *key;
	struct sockaddr_in	serv, cli;
	fd_set			fdset;
	regex_t			preg;
	regmatch_t		pmatch[5];
	size_t			nmatch=5;

	if(argc<2){
		fprintf(stderr, "usage: %s port\n", argv[0]);
		return 1;
	}

	if((w=socket(PF_INET, SOCK_STREAM, 0))==(-1)){
		perror("Can't create a socket.\n");
		return 1;
	}

	setsockopt(w, SOL_SOCKET, SO_REUSEADDR, 0, 0);

	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=INADDR_ANY;
	serv.sin_port=htons(atoi(argv[1]));

	if(bind(w, (struct sockaddr *)&serv, sizeof(serv))!=0){
		perror("Can't bind the socket.\n");
		return 1;
	}

	if(listen(w, 0)==-1){
		perror("Can't listen the socket.\n");
		return 1;
	}

	len=sizeof(cli);
	if((s=accept(w, (struct sockaddr *)&cli, (socklen_t *)&len))==(-1)){
		perror("Can't accept a socket.\n");
		return 1;
	}else{
		close(w);
		cli.sin_addr.s_addr=ntohl(cli.sin_addr.s_addr);
		cli.sin_port=ntohs(cli.sin_port);
		printf("connected with %d.%d.%d.%d:%d\n",
			cli.sin_addr.s_addr>>24&0xff,
			cli.sin_addr.s_addr>>16&0xff,
			cli.sin_addr.s_addr>>8&0xff,
			cli.sin_addr.s_addr&0xff,
			cli.sin_port);
	}

	do{
		FD_ZERO(&fdset);
		FD_SET(fileno(stdin), &fdset);
		FD_SET(s, &fdset);
		if(select(FD_SETSIZE, &fdset, NULL, NULL, NULL)){
			if(FD_ISSET(fileno(stdin), &fdset)){
				if(!fgets(buff, BUFF_SIZE, stdin)){
					close(s);
					return 0;
				}
				n=strlen(buff)+1;
				send(s, buff, n, 0);
			}
			if(FD_ISSET(s, &fdset)){
				n=recv(s, buff, BUFF_SIZE, 0);
				if(n>0){
					buff[n]=0;
					if(first){
						first=0;
						i=regcomp(&preg,
							"^Sec-WebSocket-Key: .*==",
							REG_NEWLINE);
						regexec(&preg, buff, nmatch, pmatch, 0);
						regfree(&preg);
						//for(regoff_t j=pmatch[0].rm_so+19; j<pmatch[0].rm_eo; ++j){
						//	key+=buff[j];
						//}
						buff[pmatch[0].rm_eo]=0;
						key=keyReply(&buff[pmatch[0].rm_so+19]);

						char msg[1024];
						sprintf(msg, "HTTP/1.1 101 Switching Protocols\r\n"
						"Upgrade: websocket\r\n"
						"Connection: Upgrade\r\n"
						"Sec-WebSocket-Accept: %s\r\n\r\n", key);
						g_free(key);
						printf("%s", msg);
						send(s, msg, strlen(msg), 0);
					}else{
						char opcode=buff[0]&0xf;
						int size=buff[1]&0x7f;
						int masked=buff[1]&0x80;
						switch(opcode){
							case 0x8: // Close
								close(s);
								return 0;
							case 0x1: // Text
								printf("\nBefore xor");
								for(i=0; i<n; ++i){
									if(i%8==0){
										printf("\n");
									}
									printf("%02x ", (unsigned char)buff[i]);
								}
								if(masked){ // masked?
									for(i=0; i<size; ++i){
										buff[i+6]=buff[i+6] ^ buff[i%4+2];
									}
								}
								printf("\nAfter xor");
								for(i=0; i<n; ++i){
									if(i%8==0){
										printf("\n");
									}
									printf("%02x ", (unsigned char)buff[i]);
								}
								if(false){
									gchar *output;
									gsize bytes_read, bytes_written;
									output=g_convert(&buff[6], size,
										"UTF-8", "UTF-8",
										&bytes_read, &bytes_written, NULL);
									for(i=0; i<size; ++i){
										if(i%8==0){
											printf("\n");
										}
										printf("%02x ", (unsigned char)buff[i+6]);
									}
								}
								printf("\n\n");
								fflush(stdout);
								break;
							case 0x2: // Binary
								printf("\nBinary");
								for(i=0; i<n; ++i){
									if(i%8==0){
										printf("\n");
									}
									printf("%02x ", (unsigned char)buff[i]);
								}
								if(masked){ // masked?
									for(i=0; i<size; ++i){
										buff[i+6]=buff[i+6] ^ buff[i%4+2];
									}
								}
								printf("\nAfter xor");
								for(i=0; i<n; ++i){
									if(i%8==0){
										printf("\n");
									}
									printf("%02x ", (unsigned char)buff[i]);
								}
								printf("\n\n");
								fflush(stdout);
								break;
							case 0x9: // Ping
								buff[0]=buff[0]&0xf0 + 0x9;
								send(s, buff, n, 0);
								break;
							case 0xa: // Pong
								break;
							default:
								break;
						}
					}
				}else if(n==0){
					close(s);
					return 0;
				}else{
					perror("Can't recv from the socket.\n");
					close(s);
					return 1;
				}
			}
		}
	}while(1);

	return 0;
}

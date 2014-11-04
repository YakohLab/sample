class WebSockServer {
public:
  callback(void (*)(char *));
private:
  int w, s;
  WebSockServer(int);
  ~WebSockServer(void);
  char *keyReply(char *);
};

WebSockServer::WebSockServer(int port): w(0), s(0){
	struct sockaddr_in	serv;

	try{
	  if((w=socket(PF_INET, SOCK_STREAM, 0))==(-1)){
	    through Exception;
	  }
	  setsockopt(w, SOL_SOCKET, SO_REUSEADDR, 0, 0);

	  serv.sin_family=AF_INET;
	  serv.sin_addr.s_addr=INADDR_ANY;
	  serv.sin_port=htons(port);

	  if(bind(w, (struct sockaddr *)&serv, sizeof(serv))!=0){
	    through Exception;
	  }
	}
	catch(){
	    through Exception;
	}

}

WebSockServer::~WebSockServer(){
}

char *WebSockServer::keyReply(char *key){
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

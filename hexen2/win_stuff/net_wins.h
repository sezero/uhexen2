// net_wins.h

int  WINS_Init (void);
void WINS_Shutdown (void);
void WINS_Listen (qboolean state);
int  WINS_OpenSocket (int port);
int  WINS_CloseSocket (int socket);
int  WINS_Connect (int socket, struct qsockaddr *addr);
int  WINS_CheckNewConnections (void);
int  WINS_Read (int socket, byte *buf, int len, struct qsockaddr *addr);
int  WINS_Write (int socket, byte *buf, int len, struct qsockaddr *addr);
int  WINS_Broadcast (int socket, byte *buf, int len);
char *WINS_AddrToString (struct qsockaddr *addr);
int  WINS_StringToAddr (char *string, struct qsockaddr *addr);
int  WINS_GetSocketAddr (int socket, struct qsockaddr *addr);
int  WINS_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  WINS_GetAddrFromName (char *name, struct qsockaddr *addr);
int  WINS_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  WINS_GetSocketPort (struct qsockaddr *addr);
int  WINS_SetSocketPort (struct qsockaddr *addr, int port);

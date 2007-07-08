/*
	token.h
	$Id: token.h,v 1.6 2007-07-08 17:01:15 sezero Exp $
*/

#ifndef __TOKEN_H__
#define __TOKEN_H__

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif
#ifndef YES
#define YES	1
#endif
#ifndef NO
#define NO	0
#endif
#define ASCII_SPACE		32
#define ASCII_QUOTE		34
#define ASCII_UNDERSCORE	95
#define EOF_CHARACTER		127
#define MAX_IDENTIFIER_LENGTH	64
#define MAX_QUOTED_LENGTH	1024
#define MAX_FILE_NAME_LENGTH	1024

typedef enum
{
	TK_BAD = -1,
	TK_NONE = 0,
	TK_UNKNOWNCHAR,
	TK_EOF,
	TK_IDENTIFIER,		// VALUE: (char *) tk_String
	TK_STRING,		// VALUE: (char *) tk_String
	TK_INTNUMBER,		// VALUE: (int) tk_IntNumber
	TK_FLOATNUMBER,		// VALUE: (float) tk_FloatNumber
	TK_LPAREN,
	TK_RPAREN,
	TK_LBRACE,
	TK_RBRACE,
	TK_LBRACKET,
	TK_RBRACKET,
	TK_COLON,
	TK_MESH,
	TK_MODEL,
	TK_NODES,
	TK_ROTATION,
	TK_SCALING,
	TK_TRANSLATION,
	TK_POLYGONS,
	TK_POSITION,
	TK_VERTEX,
	TK_VERTICES,
	TK_HRCH,
	TK_SOFTIMAGE,

	TK_C_NAMED,
	TK_OBJECT,
	TK_C_TRI,
	TK_C_VERTICES,
	TK_C_FACES,
	TK_C_VERTEX,
	TK_LIST,
	TK_C_FACE,

	TK_C_HEXEN,
	TK_C_TRIANGLES,
	TK_C_VERSION,
	TK_FACES,
	TK_FACE,
	TK_ORIGIN
} tokenType_t;

void TK_Init(void);
void TK_OpenSource(char *fileName);
void TK_CloseSource(void);
tokenType_t TK_Fetch(void);
void TK_Require(tokenType_t tokType);
void TK_FetchRequire(tokenType_t tokType);
tokenType_t TK_RequireFetch(tokenType_t tokType);
tokenType_t TK_FetchRequireFetch(tokenType_t tokType);
tokenType_t TK_Beyond(tokenType_t tokType);
void TK_BeyondRequire(tokenType_t bTok, tokenType_t rTok);

extern tokenType_t tk_Token;
extern int tk_Line;
extern int tk_IntNumber;
extern float tk_FloatNumber;
extern char *tk_String;
extern char tk_SourceName[MAX_FILE_NAME_LENGTH];

#endif	/* __TOKEN_H__ */


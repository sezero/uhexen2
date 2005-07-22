// cmds.c

#include "defs.h"

static	int		cmd_argc;
static	char	*cmd_argv[MAX_ARGS];
static	char	*cmd_null_string = "";
static	char	*cmd_args = NULL;

static	cmd_function_t	*cmd_functions;		// possible commands to execute

int Cmd_Argc (void)
{
	return cmd_argc;
}

char *Cmd_Argv (int arg)
{
	if (arg >= cmd_argc)
		return cmd_null_string;

	return cmd_argv[arg];	
}

void Cmd_TokenizeString (char *text)
{
	int		i;
	
	// clear the args from the last string
	for (i=0 ; i<cmd_argc ; i++)
		free (cmd_argv[i]);
		
	cmd_argc = 0;
	cmd_args = NULL;
	
	while (1)
	{	// skip whitespace up to a /n
		while (*text && *text <= ' ' && *text != '\n')
		{
			text++;
		}

		if (*text == '\n')
		{	// a newline seperates commands in the buffer
			text++;
			break;
		}

		if (!*text)
			return;

		if (cmd_argc == 1)
			 cmd_args = text;

		text = COM_Parse (text);
		if (!text)
			return;

		if (cmd_argc < MAX_ARGS)
		{
			cmd_argv[cmd_argc] = (char *)malloc (strlen(com_token)+1);
			strcpy (cmd_argv[cmd_argc], com_token);
			cmd_argc++;
		}
	}
}

void Cmd_AddCommand (char *cmd_name, xcommand_t function)
{
	cmd_function_t	*cmd;

	// fail if the command already exists
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!strcmp (cmd_name, cmd->name))
		{
			printf ("Cmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}
	}

	cmd = (cmd_function_t *)malloc(sizeof(cmd_function_t));
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
}

void Cmd_Init()
{
	Cmd_AddCommand("quit",Cmd_Quit_f);
	Cmd_AddCommand("list",Cmd_ServerList_f);
	Cmd_AddCommand("filter",Cmd_Filter_f);
}

void Cmd_ExecuteString (char *text)
{	
	cmd_function_t	*cmd;

	Cmd_TokenizeString (text);

	// execute the command line
	if (!Cmd_Argc())
		return;		// no tokens

	// check functions
	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!strcmp(cmd_argv[0],cmd->name))
		{
			if (cmd->function)
				cmd->function ();
			return;
		}
	}

	printf("Unknown command: %s\n",cmd_argv[0]);
}


void Cbuf_Init (void)
{
	cmd_text.data = cmd_text_buf;
	cmd_text.maxsize = sizeof(cmd_text_buf);
}

void Cbuf_AddText (char *text)
{
	int		l;

	l = strlen (text);

	if (cmd_text.cursize + l >= cmd_text.maxsize)
	{
		printf ("Cbuf_AddText: overflow\n");
		return;
	}

	SZ_Write (&cmd_text, text, strlen (text));
}

void Cbuf_InsertText (char *text)
{
	char	*temp;
	int	templen;

	// copy off any commands still remaining in the exec buffer
	templen = cmd_text.cursize;
	if (templen)
	{
		temp = (char *)malloc (templen);
		memcpy (temp, cmd_text.data, templen);
		SZ_Clear (&cmd_text);
	}
	else
	{
		temp = NULL;	// shut up compiler
	}

	// add the entire text of the file
	Cbuf_AddText (text);
	SZ_Write (&cmd_text, "\n", 1);

	// add the copied off data
	if (templen)
	{
		SZ_Write (&cmd_text, temp, templen);
		free (temp);
	}
}

void Cbuf_Execute (void)
{
	int	i, quotes;
	char	*text;
	char	line[1024];

	while (cmd_text.cursize)
	{
		// find a \n or ; line break
		text = (char *)cmd_text.data;

		quotes = 0;
		for (i=0 ; i< cmd_text.cursize ; i++)
		{
			if (text[i] == '"')
				quotes++;
			if ( !(quotes&1) &&  text[i] == ';')
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
		}

		memcpy (line, text, i);
		line[i] = 0;

// delete the text from the command buffer and move remaining commands down
// this is necessary because commands (exec, alias) can insert data at the
// beginning of the text buffer

		if (i == cmd_text.cursize)
		{
			cmd_text.cursize = 0;
		}
		else
		{
			i++;
			cmd_text.cursize -= i;
			memcpy (text, text+i, cmd_text.cursize);
		}

		// execute the command line
		Cmd_ExecuteString (line);
	}
}

qboolean Cmd_Exists (char *cmd_name)
{
	cmd_function_t	*cmd;

	for (cmd=cmd_functions ; cmd ; cmd=cmd->next)
	{
		if (!strcmp (cmd_name,cmd->name))
			return true;
	}

	return false;
}


//Commands

void Cmd_Quit_f()
{
	printf ("Shutting down.\n");
	SV_Shutdown ();

	Sys_Quit();
}

void Cmd_ServerList_f()
{
	server_t *sv;

	for(sv = sv_list;sv;sv = sv->next)
		printf("%s  %i players\n",NET_AdrToString(sv->ip),sv->players);
}

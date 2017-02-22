/* $Id: s_chpw.c,v 1.8 2009/04/03 21:09:11 hoche Exp $ */

/* primitive to prompt for old and new passwords and change them
  (VMS server) */

/* New cp function by Phil Stracchino, November 16 1994 */

#include "icb.h"

int
s_chpw (ARGV_TCL)
{
	static char *usage = "usage: s_chpw [old] [new] [new]";
	char args[256], old[64], new1[64], new2[64], message[256];
	char * token = NULL;
	int match = 0, tokens = 0, length = 0;

	args[0] = old[0] = new1[0] = new2[0] = (char)0;

	if (argc > 2 || argc < 1)
		TRETURNERR(usage)

/* Seems all the args actually get packed into argv[1], which means that we */
/* have to unpack them manually.  The strtok() function is a good way to do */
/* this.                                                                                                            */

	if (argc == 2 && argv[1][0])
	{
		strcpy( args, argv[1] );
		token = strtok( args, " " );
		if (token != NULL)
		{
			tokens++;
			strcpy( old, token );
			token = (char *)strtok( NULL, " " );
			if (token != NULL)
			{
				tokens++;
				strcpy( new1, token );
				token = (char *)strtok( NULL, " " );
				if (token != NULL)
				{
					tokens++;
					strcpy ( new2, token );
					token = (char *)strtok( NULL, " " );
					if (token != NULL)
					{
						tokens++;
					}
				}
			}
		}
	}

	switch( tokens )
	{
		case 0:
		{
			strcpy( old, (char *)getpass( "Enter old password:" ));
			if (strlen( old ))
				tokens++;
	else
				break;
		}

		case 1:
		{
			do
			{
				strcpy( new1, (char *)getpass( "Enter new password:" ));
				length = strlen( new1 );
				sprintf( message, "Received password %s, length %d", new1, length);
				putl( message, PL_SCR );
				if (length > 14)
					putl( "Password is too long - 14 characters max", PL_SCR );
			}
			while (length > 14);
			if (length > 0)
				tokens++;
	else
				break;
		}

		case 2:
	{
			do
		{
				strcpy( new2, (char *)getpass( "Enter new password again:" ));
				match = strcmp (new1, new2);
				if (match)
					putl( "New password mismatch; try again", PL_SCR );
		}
			while (strlen( new2 ) && match);
			if (strlen( new2 ))
				tokens++;
			else
				break;
		}

		case 3:
		{
			match = !strcmp (new1, new2);
			if (match) {
				sprintf( message, "New password %s accepted", new1 );
				putl( message, PL_SCR );
				sprintf( message, "cp %s %s", old, new1 );
				sendpersonal( "server", message, -1 );
	}
			else
				putl( "New password mismatch; password change aborted.", PL_SCR );
			break;
		}

		default:
		{
			TRETURNERR( usage );
			break;
		}
	}

	if (tokens < 3)
		putl( "Password change cancelled", PL_SCR );

	return(TCL_OK);
}

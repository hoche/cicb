/* $Id: readlineinit.c,v 1.21 2009/04/04 09:08:40 hoche Exp $ */

#include "icb.h"

#include <readline/readline.h>
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif


/*
 * readlineinit - set up for readline()
 */

void
readlineinit(void)
{
	char *editor;
	int handletab();

	rl_initialize ();

	/*
	 * bind the tab key
	 */
	rl_bind_key_in_map('\t', handletab, vi_insertion_keymap);
	rl_bind_key_in_map('\t', handletab, vi_movement_keymap);
	rl_bind_key_in_map('\t', handletab, emacs_standard_keymap);
	rl_inhibit_completion = 1;

	rl_getc_function = getc_or_dispatch;

#ifdef HAVE_READLINE_2
	rl_vi_editing_mode(); /* default to vi, dammit. */
#else
	rl_vi_editing_mode(0,0); /* default to vi, dammit. */
#endif
	gv.editmode = "vi";

	editor = (char *)getenv("EDITOR");
	if (!editor)
		editor = (char *)getenv("VISUAL");

	if (editor != NULL)
		if (strcmp(basename(editor),"emacs")==0)
		{
#ifdef HAVE_READLINE_2
			rl_emacs_editing_mode();
#else
			rl_emacs_editing_mode(0,0);
#endif
			gv.editmode = "emacs";
		}
}

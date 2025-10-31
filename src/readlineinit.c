#include "icb.h"

#include <libgen.h>  // basename
#include <readline/readline.h>

/*
 * readlineinit - set up for readline()
 */

void readlineinit(void) {
    char *editor;
    int handletab();

    rl_initialize();

    /*
     * bind the tab key
     */
    rl_bind_key_in_map('\t', handletab, vi_insertion_keymap);
    rl_bind_key_in_map('\t', handletab, vi_movement_keymap);
    rl_bind_key_in_map('\t', handletab, emacs_standard_keymap);
    rl_inhibit_completion = 1;

    rl_getc_function = getc_or_dispatch;

    rl_vi_editing_mode(0, 0); /* default to vi, dammit. */
    gv.editmode = "vi";

    editor = (char *) getenv("EDITOR");
    if (! editor)
        editor = (char *) getenv("VISUAL");

    if (editor != NULL)
        if (strcmp(basename(editor), "emacs") == 0) {
            rl_emacs_editing_mode(0, 0);
            gv.editmode = "emacs";
        }
}

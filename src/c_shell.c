/* primitive to run a command in a shell */

#include "icb.h"
#include <sys/wait.h>

int
c_shell(ARGV_TCL)
{
    static char *usage = "usage: c_shell [command]";
    int w, pid;
    int shellout = 0;
    char mbuf2[512];
    pid_t status;
    char *shell;

    /* disallow use if restricted mode set */
    if (gv.restricted)
        TRETURNERR("c_shell: no shell commands in restricted mode");

    /* is this a shellout or a command? */
    if (argc == 1 || !*argv[1])
        shellout++;

    /* get shell to use */
    if ((shell = getenv("SHELL")) == NULL)
        shell = "/bin/sh";

    /* announce what we are doing */
    if (shellout) {
        sprintf(mbuf, "[%s]\r\n", shell);
        putl(mbuf, PL_SCR);
    } else {
        sprintf(mbuf, "%s", catargs(&argv[1]));
        sprintf(mbuf2, "[%s]\r\n", mbuf);
        putl(mbuf2, PL_SCR);
    }

    if ((pid = fork()) == 0) {
        if (shellout)
            execlp(shell, shell, NULL);
        else
            execlp(shell, shell, "-c", mbuf, NULL);

        /* exec failed if we make it here */
        sprintf(mbuf, "c_shell: can't run shell \"%s\"", shell);
        putl(mbuf, PL_SL);
        putl(usage, PL_SCR);
        exit(-1);
    }
#if 0
    /* trap signals that might mess ICB */
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
#endif

    /* wait for the child(ren) to die */

    while ((w = wait(&status)) != pid && w != -1) ;

#if 0
    /* restore signal processing */
    signal(SIGINT, handle_intr);
    signal(SIGQUIT, SIG_DFL);
#endif

    /* clean up a little */
    if (shellout) {
        write(1, "\r", 1);
    }

    if (gv.pauseonshell) {
        pauseprompt("[=Hit return to continue=]", 0, '\n', 0, 0);
    } else
        puts("[=returning to icb=]");
    return (TCL_OK);
}

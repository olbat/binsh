/*
 * Copyright (C) 2013 Sarzyniec Luc <contact@olbat.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * see the COPYING file for more informations */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

extern char **environ;

#define BUFF_SIZE 1024
#define SWRITE(F,S) write(F,S,sizeof(S))
/* #define SCRIPT "..." */
#ifndef SHELL
#define SHELL "/bin/sh\x0-s\x0--\x0"
#endif

int main(int argc, char **argv)
{
	int fds[2];
	unsigned int i,j,k;
	size_t len,tmp;
        char **args;
	char *key,*ptr;

	tmp = 0;

	/* Handle the key argument */
	if (argc > 1)
	{
		/* The key is specified from STDIN */
		if (!strncmp(argv[1],"-",2) && !isatty(STDIN_FILENO))
		{
			key = malloc(BUFF_SIZE);

			len = 0;
			ptr = key;
			while ((tmp = read(STDIN_FILENO,ptr,BUFF_SIZE)) > 0)
			{
				len += tmp;
				if (tmp == BUFF_SIZE)
					key = realloc(key,len+BUFF_SIZE);
				ptr = key + len;
			}
		}
		else
		{
			key = argv[1];
			len = strlen(key);
		}
	}
	else
	{
		SWRITE(1,"key missing\n");
		return 1;
	}

	/* Reuse STDIN to pipe the script to the shell */
	pipe(fds);
	close(STDIN_FILENO);
	dup2(fds[0], STDIN_FILENO);

	/* Count the number of arguments in the shell's command */
	j = 0;
	i = sizeof(SHELL);
	while (i--)
		if (!SHELL[i])
			j++;

	/* Allocate some memory for the arguments passed to execve():
	 *   - the shell's command (i.e. ["/bin/bash", "-s", "--"])
	 *   - the arguments actually passed to this program (without the key)
	 *   - the final null argument
	 */
	args = (__typeof__(args)) alloca((j+argc-2)*sizeof(args));

	/* Copy the shell's command to args (split SHELL using '\x0') */
	j=0;
	ptr = SHELL;
	for (i=0; i < (sizeof(SHELL)-1); i++)
	{
		if (!SHELL[i])
		{
			k = 0;
			args[j] = (char *) alloca((SHELL+i)-ptr+1);
			while (*ptr)
				args[j][k++] = *ptr++;
			ptr++;
			args[j++][k] = 0;
		}
	}

	/* Copy the program's arguments to args */
	for (i=j;i<(argc+j-2);i++)
		args[i] = argv[i-j+2];

	/* Set the last element of args to null (necessary for execve()) */
	args[i] = NULL;

	/* Decrypt the script using the key */
	for (i=0;i<(sizeof(SCRIPT)-1);i++)
		write(fds[1],&(char){SCRIPT[i]^key[(i+key[i%len])%len]},1);
	close(fds[1]);

	/* Clean dynamically allocated memory */
	if (tmp)
		free(key);

	/* Run the script using the shell's command specified in SHELL */
	execve(args[0], args, environ);

	perror("execve");
	return 1;
}

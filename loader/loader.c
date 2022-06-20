/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>	//malloc, calloc, free
#include <fcntl.h> //open
#include <signal.h> //sigaction
#include <sys/mman.h> //mmap
#include <unistd.h>	//read, lseek

#include "exec_parser.h"

#define dim_buf 4096 //dimensiunea unei pagini

static so_exec_t *exec; //structura fisierului executabil

//functie ce are ca scop rularea handlerului default
void run_default_handler()
{
	signal(SIGSEGV, NULL);
	raise(SIGSEGV);
}

/*
 *In interiorul strucutrii siginfo_t se gasesc informatii cu privire la adresa
 *de unde provine semnalul, folosind aceasta informatie gasim segmentul unde
 *a avut loc problema.
 */
void signal_action(int sig_no, siginfo_t *sig_info, void *context)
{
	int error_address = (int)sig_info->si_addr;

	if (error_address < exec->base_addr)
		run_default_handler();

	for (int i = 0 ; i < exec->segments_no; i++) {
		if (error_address >= exec->segments[i].vaddr &&
			error_address < exec->segments[i].vaddr + exec->segments[i].mem_size) {
			//verificam daca am alocat deja memorie in campul data al structurii
			if (exec->segments[i].data_size == 0) {
				exec->segments[i].data =
				(void *) calloc(exec->segments[i].mem_size / dim_buf, sizeof(char));
				exec->segments[i].data_size = 1;
			}

			//gasim numarul paginii si adresa inceputului paginii unde se va face maparea
			char *data = (char *)exec->segments[i].data;
			int page_number = (error_address - exec->segments[i].vaddr) / dim_buf;
			int page_address = page_number * dim_buf;

			if (data[page_number] == 1)
				run_default_handler();
			data[page_number] = 1;

			//facem maparea paginii
			char *mapped_address = mmap((void *)(exec->segments[i].vaddr + page_address),
										//adresa unde se doreste maparea
										dim_buf,	//dimensiune
										PROT_WRITE,	//tipul de acces
										MAP_SHARED | MAP_FIXED | MAP_ANON,	//flags
										exec->fd,	//file-descriptor
										exec->segments[i].offset);	//offset

			if (mapped_address == MAP_FAILED)
				return;

			if (page_address < exec->segments[i].file_size) {
				//se pozitioneaza offset-ul
				lseek(exec->fd, exec->segments[i].offset + page_address, SEEK_SET);
				int result_count;
				int difference = exec->segments[i].file_size - page_address;
				//se face citirea
				if (dim_buf < difference)
					result_count = read(exec->fd, (void *) mapped_address, dim_buf);
				else
					result_count = read(exec->fd, (void *) mapped_address, difference);
				if (result_count < 0)
					return;
			}

			//setam permisiunile paginii precum permisiunile segmentului
			mprotect(mapped_address, dim_buf, exec->segments[i].perm);

			return;
		}
	}

	run_default_handler();
}

/*
 *Functie ce se ocupa cu declararea unui nou handler pentru SIGSEGV
 *Declar o noua structura sigaction, o initializez si ii dau sa ruleze
 *functia signal_action in cazul in care se receptioneaza semnalul
 */
int so_init_loader(void)
{
	struct sigaction newact;

	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	newact.sa_sigaction = signal_action;
	sigaction(SIGSEGV, &newact, NULL);
	return -1;
}

/*
 *Functia parseaza informatiile executabilului si le salveaza in structura exec
 *---
 *Apoi incearca sa execute fisierul
 */
int so_execute(char *path, char *argv[])
{
	exec = calloc(1, sizeof(so_exec_t));
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	exec->fd = open(path, O_RDONLY);
	if (exec->fd < 0) {
		perror("open");
		return -1;
	}

	so_start_exec(exec, argv);

	return -1;
}

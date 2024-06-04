#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


typedef struct {
    char *src;
    char *dst;
    int thread_id;
    int num_threads;
	int blocks_per_thread;
    int (*args)[2];
    bool *ready;
} ThreadData;

int main(int argc, char *argv[]) {
    srand(time(NULL));
    setlocale(LC_CTYPE, "");
    initscr(); // Start curses mode

    if (argc != 2) {
        fprintf(stderr, "Wrong number of args\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);

    char *foreground = create_grid();
    char *background = create_grid();

    init_grid(foreground);
	double number = (double)grid_height*grid_width / num_threads;
	int blocks_per_thread = grid_height*grid_width / num_threads;
    bool ready = false;

	if (number - blocks_per_thread > 0.5) {
		blocks_per_thread++;
	}
	int rest =(grid_height*grid_width)- (blocks_per_thread*(num_threads-1));
	if (blocks_per_thread == 0) {
		blocks_per_thread = 1;
		num_threads = grid_height*grid_width;
		rest = 1;
	}


    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
	int row=0;
	int col=-1;

    for (int i = 0; i < num_threads-1; ++i) {
        thread_data[i].src = foreground;
        thread_data[i].dst = background;
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
		thread_data[i].blocks_per_thread = blocks_per_thread;
		thread_data[i].args = malloc(blocks_per_thread * sizeof(int[2]));
		thread_data[i].ready = &ready;
		for (int j = 0; j < blocks_per_thread; j++) {
			if (col == grid_width-1) {
				col = 0;
				row++;
			}
			else {
				col++;
			}
			thread_data[i].args[j][0] = row;
			thread_data[i].args[j][1] = col;
		}
        pthread_create(&threads[i], NULL, update_grid_thread, &thread_data[i]);
    }

	thread_data[num_threads-1].src = foreground;
    thread_data[num_threads-1].dst = background;
    thread_data[num_threads-1].thread_id = num_threads-1;
    thread_data[num_threads-1].num_threads = num_threads;
	thread_data[num_threads-1].blocks_per_thread = rest;
	thread_data[num_threads-1].args = malloc(rest * sizeof(int[2]));
	thread_data[num_threads-1].ready = &ready;
	for (int j = 0; j < rest; j++) {
		if (col == grid_width-1) {
			col = 0;
			row++;
		}
		else {
			col++;
		}
		thread_data[num_threads-1].args[j][0] = row;
		thread_data[num_threads-1].args[j][1] = col;
		if (row == grid_height-1 && col == grid_width-1) {
			break;
		}
	}
	pthread_create(&threads[num_threads-1], NULL, update_grid_thread, &thread_data[num_threads-1]);
	char *tmp;


    while (true) {
		usleep(500000);
        draw_grid(foreground);
        ready = true;
        for (int i = 0; i < num_threads; ++i) {
			thread_data[i].ready = &ready;
		}
        tmp = foreground;
		foreground = background;
		background = tmp;
        for (int i = 0; i < num_threads; ++i) {
            thread_data[i].src = foreground;
            thread_data[i].dst = background;
        }
	}

    endwin(); 
    destroy_grid(foreground);
    destroy_grid(background);

	for (int i = 0; i < num_threads; ++i) {
        free(thread_data[i].args);
    }

    return 0;
}
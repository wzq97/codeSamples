#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>

#define MAXTHREADS      1000

/* definition of a struct board_t to send to each thread */
typedef struct
{
    int m;
    int n;
    int row;    /* current row */
    int col;
    int move_count;
    int next_move;
    char ** board;
}   board_t;

board_t ** dead_end_boards;
int max_squares;    /* max number of squares covered */
int dead_i;         /* count for dead end array */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void print_board( char ** board, int m){
    if(board==NULL || m == 0) return;
    for(int r = 0; r < m; r++){
        if(r == 0 ) printf("THREAD %ld: > %s\n", pthread_self(), board[r]);
        else printf("THREAD %ld:   %s\n", pthread_self(), board[r]);
    }
}

/* Check if a given point lies in the board */
bool in_board( int m, int n, int r, int c ){
    if( r < 0 || c < 0 || r > m-1 || c > n-1 )  return false;
    return true;
}

/* CHeck if a board is full -> full night's tour */
bool is_full( char ** board, int m, int n){
    if( board == NULL ) return false;
    if( m == 0 || n == 0 ) return false;
    for(int r = 0; r < m; r++){
        if( *(board[r])=='\0' ) return false;
        for(int c = 0; c < n; c++){
            if( board[r][c]=='\0' ) return false;
            if( board[r][c] == '.')  return false;
        }
    }
    return true;
}

/* Return the squares covered in this board */
int square_covered( char ** board, int m, int n ){
    if( board == NULL ) return 0;
    int count = 0;
    for(int r = 0; r < m; r++){
        for(int c = 0; c < n; c++){
            if(board[r][c] == 'S')  count++;
        }
    }
    return count;
}

/* update board_t struct: position and move_count.. */
void update_position( board_t * myboard, int new_row, int new_col ){
    if(!in_board(myboard->m,myboard->n, new_row, new_col)) return;
    myboard->row = new_row;
    myboard->col = new_col;
    (myboard->move_count)++;
    (myboard->board)[new_row][new_col] = 'S';
}

/* Calculate the possible moves at the current position */
int possible_moves( board_t * myboard, int * moves ){
    int m = myboard->m;
    int n = myboard->n;
    int r = myboard->row;
    int c = myboard->col;
    int poss_moves = 0;
    for(int i = 0; i < 8; i++){
        moves[i] = 0;
    }
    if( in_board(m,n,r-1,c-2) && (myboard->board)[r-1][c-2]=='.' ){
        poss_moves++;
        moves[0] = 1;
    }
    if( in_board(m,n,r-2,c-1) && (myboard->board)[r-2][c-1]=='.' ){
        poss_moves++;
        moves[1] = 2;
    }
    if( in_board(m,n,r-2,c+1) && (myboard->board)[r-2][c+1]=='.' ){
        poss_moves++;
        moves[2] = 3;
    }
    if( in_board(m,n,r-1,c+2) && (myboard->board)[r-1][c+2]=='.' ){
        poss_moves++;
        moves[3] = 4;
    }
    if( in_board(m,n,r+1,c+2) && (myboard->board)[r+1][c+2]=='.' ){
        poss_moves++;
        moves[4] = 5;
    }
    if( in_board(m,n,r+2,c+1) && (myboard->board)[r+2][c+1]=='.' ){
        poss_moves++;
        moves[5] = 6;
    }
    if( in_board(m,n,r+2,c-1) && (myboard->board)[r+2][c-1]=='.' ){
        poss_moves++;
        moves[6] = 7;
    }
    if( in_board(m,n,r+1,c-2) && (myboard->board)[r+1][c-2]=='.' ){
        poss_moves++;
        moves[7] = 8;
    }
    return poss_moves;
}

void free_board( char ** board, int m ){
    for(int i = 0; i < m; i++){
        free(board[i]);
    }
    free(board);
}

void free_board_t ( board_t * myboard ){
    free_board(myboard->board, myboard->m);
    free(myboard);
}

/* move thread function */
void * move_sonny_thread ( void * arg ){
    
    board_t * myboard = (board_t *) arg;
    int r = myboard->row;
    int c = myboard->col;
    int m = myboard->m;
    int n = myboard->n;
    int poss_moves = 0;
    int nextMove = 0;
    int * squares = malloc( sizeof(int));
    int * returned = malloc( sizeof(int));
    int * max_returned = calloc( 1, sizeof(int));
    int moves[8];
    (myboard->board)[r][c] = 'S';
    /* try to move in 8 directions */
    /* create child if multiple moves are possible */
    while(1){
        poss_moves = 0;     /* reset possible moves */
        r = myboard->row;
        c = myboard->col;
        nextMove = myboard->next_move;
        /* FIRST update for next move*/
        if(nextMove == 1){
            update_position(myboard,r-1,c-2);
        }else if(nextMove == 2){
            update_position(myboard,r-2,c-1);
        }else if(nextMove == 3){
            update_position(myboard,r-2,c+1);
        }else if(nextMove == 4){
            update_position(myboard,r-1,c+2);
        }else if(nextMove == 5){
            update_position(myboard,r+1,c+2);
        }else if(nextMove == 6){
            update_position(myboard,r+2,c+1);
        }else if(nextMove == 7){
            update_position(myboard,r+2,c-1);
        }else if(nextMove == 8){
            update_position(myboard,r+1,c-2);
        }
        /* THEN detect NEXT moves */
        poss_moves = possible_moves(myboard, moves);
        /* Case 1: multiple moves */
        if( poss_moves > 1 ){
            /* create multiple threads */
            printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n", pthread_self(), poss_moves, myboard->move_count);
            pthread_t tid[poss_moves];
            int tmp_poss_moves = poss_moves;
            int start = 0;
            *max_returned = 0;
            /* Make multiple boards */
            for(int t = 0; t < tmp_poss_moves; t++){
                for(int n = start; n < 8; n++){
                    if( moves[n] != 0 ){
                        nextMove = moves[n];
                        start = n+1;
                        moves[n] = 0;
                        break;
                    }
                }
                /* Make a new board */
                board_t * new_board = malloc( sizeof(board_t) );
                new_board->board = calloc( m , sizeof(char *) );
                new_board->row = myboard->row;
                new_board->col = myboard->col;
                new_board->m = m;
                new_board->n = n;
                new_board->move_count = myboard->move_count;
                new_board->next_move = nextMove;
                
                for(int i = 0; i < m; i++){
                    (new_board->board)[i] = malloc( n+1 );
                }
                for(int r = 0; r < m; r++){     /* copy the current board to new board */
                    memcpy((new_board->board)[r], (myboard->board)[r], n * sizeof((myboard->board)[0][0]) );    /* copy current board */
                    (new_board->board)[r][n] = '\0';
                }

                int rc = pthread_create( &tid[t], NULL, move_sonny_thread, (void *) new_board );
                if( rc != 0 ){
                    fprintf(stderr, "MAIN: Could not create thread (%d)\n", rc );
                    return (void *) EXIT_FAILURE;
                }

                #ifdef NO_PARALLEL
                rc = pthread_join( tid[t], (void **)&returned ); /*BLOCKING*/
                if(*max_returned < *returned) max_returned = returned;
                if ( rc != 0 ){
                  fprintf( stderr, "MAIN: Could not join thread (%d)\n", rc );
                }else{
                    printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", pthread_self(), tid[t], *returned);                   
                }
                #endif
            }
            #ifndef NO_PARALLEL 
            nextMove = 0;
            for(int t = 0; t < tmp_poss_moves; t++){
                int rc = pthread_join( tid[t], (void **)&returned ); /*BLOCKING*/
                if(*max_returned < *returned) max_returned = returned;
                if ( rc != 0 ){
                  fprintf( stderr, "MAIN: Could not join thread (%d)\n", rc );
                }else{
                    printf("THREAD %ld: Thread [%ld] joined (return %d)\n", pthread_self(), tid[t], *returned);                   
                }
            }
            #endif
            break;
        }
        /* Case 2: one move possible: update next move */
        else if( poss_moves == 1 ){
            poss_moves = possible_moves(myboard, moves);
            int start = 0;
            for(int n = start; n < 8; n++){
                if( moves[n] != 0 ){
                    nextMove = moves[n];
                    start = n+1;
                    break;
                }
            }
            myboard->next_move = nextMove;
            continue;
        }
        /* Case 3: no move possible, either deadend or full night's tour */
        else if( poss_moves == 0 ){
            nextMove = 0;
            *squares = square_covered((myboard->board), m , n);
            if( is_full((myboard->board), m , n) ){
                /* check if a full night's tour */
                printf("THREAD %ld: Sonny found a full knight's tour!\n", pthread_self());
            }else{  /* Deadend */
                #ifndef NO_PARALLEL 
                pthread_mutex_lock( &mutex );
                #endif
                printf("THREAD %ld: Dead end after move #%d\n", pthread_self(), myboard->move_count );
                if(dead_i!=0 && dead_i%(m*n)==0){   /* resize array if necessary */
                    dead_end_boards = (board_t **) realloc(dead_end_boards, 2*(dead_i+1)*sizeof(board_t *));
                }
                dead_end_boards[dead_i] = (board_t *)myboard;   /* Add to global */
                dead_i++;
                #ifndef NO_PARALLEL 
                pthread_mutex_unlock( &mutex );
                #endif
            }
            if (*squares > max_squares)  max_squares = *squares;
            pthread_exit( squares );
        }
    }
    free(myboard);
    return max_returned;
}

int main(int argc, char* argv[]){

    if( argc != 3 && argc != 4){
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <m> <n> [<x>]\n" );
        return EXIT_FAILURE;
    }
    int x = 0;
    int m = atoi(argv[1]);              /* # of rows */
    int n = atoi(argv[2]);              /* # of columns */
    if(argc == 4) x = atoi(argv[3]);    /* OPTIONAL: Display with at least x squares covered */
    if( m <= 2 || n <= 2 || (x != 0 && x > m*n) ){
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <m> <n> [<x>]\n" );
        return EXIT_FAILURE;
    }
    /* Initialize board0 */
    char ** board = calloc( m , sizeof(char *) );
    if( board == NULL ){
        fprintf( stderr, "ERROR: calloc() failed.\n" );
        return EXIT_FAILURE;
    }
    for(int i = 0; i < m; i++){
        board[i] = malloc( n+1 );
        for(int j = 0; j < n; j++){
            *(board[i]+j) = '.';
        }
        *(board[i]+n) = '\0';
    }

    dead_end_boards = calloc( m*n, sizeof(board_t *));  /* initialize dead_end_boards array */
    max_squares = 0;
    printf( "THREAD %ld: Solving Sonny's knight's tour problem for a %dx%d board\n", pthread_self(), m, n );

    dead_i = 0;
    board[0][0] = 'S';
    board_t * board0 = malloc( sizeof(board_t) );
    board0->board = board;
    board0->row = 0;    /* current row */
    board0->col = 0;    /* current col */
    board0->m = m;
    board0->n = n;
    board0->move_count = 1;
    board0->next_move = 0;

    move_sonny_thread( (void *) board0 );

    printf("THREAD %ld: Best solution(s) found visit %d squares (out of %d)\n", pthread_self(), max_squares, m*n);

    printf("THREAD %ld: Dead end boards:\n", pthread_self());
    /* Print dead end boards and free them */
    for(int j = 0; j < dead_i; j++){
        if(dead_end_boards[j]!= NULL){
            if(square_covered(dead_end_boards[j]->board, m, n) >= x){
                print_board((char **) (dead_end_boards[j])->board, m);
                free_board_t(dead_end_boards[j]);
            }
        }
    }
    free(dead_end_boards);
    free_board(board, m);
    return EXIT_SUCCESS;
}

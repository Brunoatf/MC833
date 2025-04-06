#ifndef MOVIE_H
#define MOVIE_H

#define PORT 3490
#define BACKLOG 10

#define MAX_TITLE_LEN 100
#define MAX_DIRECTOR_LEN 100
#define MAX_GENRES 3
#define MAX_GENRE_LEN 20

struct movie {
    int id;
    char title[MAX_TITLE_LEN];
    char genres[MAX_GENRES][MAX_GENRE_LEN];
    int genre_count;
    char director[MAX_DIRECTOR_LEN];
    int year;
};

struct genre_addition_params {
    int id;
    char genre[MAX_GENRE_LEN];
};

// Operation codes
#define OP_SAVE_MOVIE 1
#define OP_ADD_GENRE 2
#define OP_REMOVE_MOVIE 3
#define OP_LIST_ALL_MOVIES_TITLES_AND_IDS 4
#define OP_LIST_ALL_MOVIES_INFO 5
#define OP_LIST_MOVIE_BY_ID 6
#define OP_LIST_MOVIES_BY_GENRE 7


#endif // MOVIE_H 
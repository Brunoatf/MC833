#ifndef JSON_DB_H
#define JSON_DB_H

#include "movie.h"

#define JSON_FILE "movies.json"
#define MAX_MOVIES 100

int add_movie(struct movie *m);
int load_movies(struct movie movies[], int max);
int save_movies(struct movie movies[], int count);
int remove_movie(int id);
int add_genre(int id, const char *genre);
int get_movie_by_id(int id, struct movie *out);
int list_movies(struct movie out[], int max);
int list_movies_by_genre(const char *genre, struct movie out[], int max);


#endif
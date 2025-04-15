#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_database.h"
#include "cJSON.h"
 #include <unistd.h> // para utilizar a funcao sleep e testar concorrencia

static int generate_new_id(struct movie movies[], int count) {
    /* 
    * Gera um novo ID baseado no maior ID existente + 1.
    */
    int max = 0;
    for (int i = 0; i < count; i++) {
        if (movies[i].id > max) {
            max = movies[i].id;
        }
    }
    return max + 1;
}

int load_movies(struct movie movies[], int max) {
    /* 
    * Carrega os filmes salvos no arquivo JSON para um array de filmes.
    * Retorna a quantidade de filmes carregados.
    */
    FILE *file = fopen(JSON_FILE, "r");

    if (!file) {
        perror("Banco de Dados JSON está faltando.\n");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    if (!json || !cJSON_IsArray(json)) {
        cJSON_Delete(json);
        return 0;
    }

    int count = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, json) {
        if (count >= max) break;
        struct movie *m = &movies[count];
        m->id = cJSON_GetObjectItem(item, "id")->valueint;

        cJSON *title = cJSON_GetObjectItem(item, "title");
        cJSON *director = cJSON_GetObjectItem(item, "director");
        cJSON *year = cJSON_GetObjectItem(item, "year");
        cJSON *genres = cJSON_GetObjectItem(item, "genres");

        strncpy(m->title, title->valuestring, MAX_TITLE_LEN);
        strncpy(m->director, director->valuestring, MAX_DIRECTOR_LEN);
        m->year = year->valueint;
        m->genre_count = cJSON_GetArraySize(genres);

        for (int i = 0; i < m->genre_count; i++) {
            strncpy(m->genres[i], cJSON_GetArrayItem(genres, i)->valuestring, MAX_GENRE_LEN);
        }

        count++;
    }

    cJSON_Delete(json);
    return count;
}

int save_movies(struct movie movies[], int count) {
    /* 
    * Salva um array de filmes no arquivo JSON.
    * Retorna 1 em caso de sucesso e 0 em caso de erro.
    */
    cJSON *json = cJSON_CreateArray();

    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", movies[i].id);
        cJSON_AddStringToObject(item, "title", movies[i].title);
        cJSON_AddStringToObject(item, "director", movies[i].director);
        cJSON_AddNumberToObject(item, "year", movies[i].year);

        cJSON *genres = cJSON_CreateArray();
        for (int j = 0; j < movies[i].genre_count; j++) {
            cJSON_AddItemToArray(genres, cJSON_CreateString(movies[i].genres[j]));
        }
        cJSON_AddItemToObject(item, "genres", genres);

        cJSON_AddItemToArray(json, item);
    }

    char *output = cJSON_Print(json);
    FILE *file = fopen(JSON_FILE, "w");

    if (!file) {
        perror("Banco de Dados JSON está faltando.\n");
        return 0;
    }

    fputs(output, file);
    fclose(file);

    cJSON_free(output);
    cJSON_Delete(json);
    return 1;
}

int add_movie(struct movie *m) {
    /* 
    * Adiciona um novo filme ao banco de dados.
    * Gera um novo ID, salva no JSON e retorna o ID ou -1 em caso de erro.
    */
    struct movie movies[MAX_MOVIES];
    int count = load_movies(movies, MAX_MOVIES);
    int new_id = generate_new_id(movies, count);

    m->id = new_id;
    movies[count] = *m;
    count++;

    // Simula operação demorada para testar concorrência entre clientes
    //sleep(30);  -> APENAS PARA TESTE DE CONCORRENCIA

    if (save_movies(movies, count)) {
        return new_id;
    }
    return -1;
}

int remove_movie(int id) {
    /* 
    * Remove um filme do banco de dados com base no ID fornecido.
    * Retorna 1 em caso de sucesso e 0 se o filme não for encontrado.
    */
    struct movie movies[MAX_MOVIES];
    int count = load_movies(movies, MAX_MOVIES);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (movies[i].id == id) {
            for (int j = i; j < count - 1; j++) {
                movies[j] = movies[j + 1];
            }
            count--;
            found = 1;
            break;
        }
    }

    if (found) {
        return save_movies(movies, count);
    }

    return 0; 
}


int add_genre(int id, const char *genre) {
    /* 
    * Adiciona um gênero a um filme, se ele ainda não tiver esse gênero 
    * e não tiver atingido o limite de 3.
    * Retorna 1 se adicionado, -1 se já existe, -2 se excedeu o limite e 0 se não encontrou o filme.
    */
    struct movie movies[MAX_MOVIES];
    int count = load_movies(movies, MAX_MOVIES);

    for (int i = 0; i < count; i++) {
        if (movies[i].id == id) {
            for (int j = 0; j < movies[i].genre_count; j++) {
                if (strcasecmp(movies[i].genres[j], genre) == 0) {
                    return -1; 
                }
            }
            if (movies[i].genre_count >= MAX_GENRES) {
                return -2; 
            }

            strncpy(movies[i].genres[movies[i].genre_count], genre, MAX_GENRE_LEN);
            movies[i].genre_count++;

            return save_movies(movies, count);
        }
    }

    return 0; 
}


int get_movie_by_id(int id, struct movie *out) {
    /* 
    * Busca um filme por ID e armazena o resultado em 'out'.
    * Retorna 1 se encontrado, 0 caso contrário.
    */
    struct movie movies[MAX_MOVIES];
    int count = load_movies(movies, MAX_MOVIES);

    for (int i = 0; i < count; i++) {
        if (movies[i].id == id) {
            *out = movies[i];
            return 1;
        }
    }
    return 0;
}

int list_movies(struct movie out[], int max) {
    /* 
    * Lista todos os filmes existentes carregando diretamente do JSON.
    * Retorna a quantidade de filmes encontrados.
    */
    return load_movies(out, max);
}

int list_movies_by_genre(const char *genre, struct movie out[], int max) {
    /* 
    * Lista os filmes que possuem um determinado gênero.
    * Retorna a quantidade de filmes encontrados com esse gênero.
    */
    struct movie all[MAX_MOVIES];
    int total = load_movies(all, MAX_MOVIES);
    int count = 0;

    for (int i = 0; i < total && count < max; i++) {
        for (int j = 0; j < all[i].genre_count; j++) {
            if (strcasecmp(all[i].genres[j], genre) == 0) {
                out[count++] = all[i];
                break;
            }
        }
    }

    return count;
}
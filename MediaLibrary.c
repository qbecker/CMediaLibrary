

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "jsmn.h"




//A structure to hold movies
typedef struct _this_Movie{
    char title[500];
    char rated[500];
    char released[500];
    char runtime[500]; 
    char plot[500];
    char filename[500];
    char genre[500];
    char actors[500];
}Movie;




void listAll();
Movie* get(char title[]);
bool removeMovie(char title[]);
bool add(Movie* movie);
char* readFile(char* filename);
jsmntok_t *json_tokenise(char *js);
bool json_token_streq(char *js, jsmntok_t *t, char *s);
char* concat(const char *s1, const char *s2, const char *s3);
Movie* parseObject(jsmntok_t token, char* JSON_STRING);
bool parseJson(char* filename);
char* parseMessage(char* message);
char* getTitles();
char* surroundInQuote(char* item);

int libraryCount = 0;

Movie* DB = NULL; //supposedly this will be a dynamic array?



bool add(Movie* movie){
    bool ret = true;
    
    if(DB == NULL){
        DB = malloc(sizeof( Movie));
        if(DB == NULL){
            printf("Somethings off...");
            ret = false;
            return ret;
        }
    }else{
        Movie *newPointer = get(movie->title);
        if(newPointer->title != NULL){
            removeMovie(movie->title);
        }
        newPointer = realloc(DB, (libraryCount + 1) * sizeof(Movie));
        //reassign the pointer to the database to the new newly sized database
        DB = newPointer;
    }
    
    //incriment the library
    libraryCount = libraryCount + 1;
    
    //now add the data to the library
    strcpy(DB[libraryCount -1].title, movie->title);
    strcpy(DB[libraryCount -1].rated , movie->rated);
    strcpy(DB[libraryCount -1].released , movie->released);
    strcpy(DB[libraryCount -1].runtime, movie->runtime);
    strcpy(DB[libraryCount -1].plot , movie->plot);
    strcpy(DB[libraryCount -1].filename , movie->filename);
    strcpy(DB[libraryCount -1].genre , movie->genre);
    strcpy(DB[libraryCount -1].actors , movie->actors);
    
    return ret;
}


bool removeMovie(char title[]){
    bool ret = false;
    int count = 0;
    int v = 0;
    int insertCount = 0;
    Movie* newPointer = NULL;
    newPointer = malloc((libraryCount + 1) * sizeof(Movie));
    while(count < libraryCount){
        v = strcmp(DB[count].title,title);
        if(v != 0){
            strcpy(newPointer[insertCount].title, DB[count].title);
            strcpy(newPointer[insertCount].rated , DB[count].rated);
            strcpy(newPointer[insertCount].released , DB[count].released);
            strcpy(newPointer[insertCount].runtime, DB[count].runtime);
            strcpy(newPointer[insertCount].plot , DB[count].plot);
            strcpy(newPointer[insertCount].filename , DB[count].filename);
            strcpy(newPointer[insertCount].genre , DB[count].genre);
            strcpy(newPointer[insertCount].actors, DB[count].actors);
            insertCount++;
        }else{
            ret = true;
        }
        count = count + 1;
    }
    if(ret == true){
        free(DB);
        DB = newPointer;
        libraryCount = libraryCount -1;
    }else{
        free(newPointer);   
    }
    return ret;
}



char* readFile(char* filename){
    FILE* f = NULL;
    char* buff = NULL;
    int c;
    int counter = 0;
    char* newPointer = NULL;
    buff = malloc(sizeof(char));
    if(buff != NULL){
        f = fopen(filename, "r");
        if(f != NULL){
            while((c = getc(f)) != EOF ){
                buff = realloc(buff, (counter + 1 * sizeof(char)));
                if(buff == NULL){
                    break;
                }
                buff[counter] = (char) c;
                counter++;
            }
        }
    }
    return buff;
}



Movie* get(char title[]){

    int count = 0;
    Movie *retVal = NULL;
    for(count = 0; count < libraryCount; count++){
        int v;
        v = strcmp(DB[count].title, title);
        if(v == 0){
            retVal = malloc(sizeof( Movie));
            retVal = &DB[count];
        }
    }
    return retVal;
}



void listAll(){
    int a = 0;
    while(a < libraryCount){
        printf("Title: %s\n", DB[a].title);
        printf("rated: %s\n", DB[a].rated);
        printf("released: %s\n", DB[a].released);
        printf("Runtime %s\n", DB[a].runtime);
        printf("Plot: %s\n", DB[a].plot);
        printf("filename: %s\n", DB[a].filename);
        printf("genre: %s\n", DB[a].genre);
        printf("Actor(s): %s\n", DB[a].actors);
        printf("\n\n");
        a++;
    }
}




bool json_token_streq(char *js, jsmntok_t *t, char *s)
{
    return (strncmp(js + t->start, s, t->end - t->start) == 0
            && strlen(s) == (size_t) (t->end - t->start));
}



static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == (tok->end - tok->start) &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
		
	}
	return -1;
}





Movie* parseObject(jsmntok_t token, char* JSON_STRING){
    
    char* substring = malloc(sizeof(char) * (token.end - token.start));
    substring = strndup(JSON_STRING + token.start, token.end - token.start);
    jsmn_parser myparser;
    jsmn_init(&myparser);
    int r=0;
    r = jsmn_parse(&myparser, substring, strlen(substring),NULL, 256);
    if(r<1){
        return NULL;
    }
    
    jsmntok_t tokens[r];
    jsmn_init(&myparser);
    jsmn_parse(&myparser, substring, strlen(substring),tokens, r);
    
    int i = 0;
    Movie* tempMovie;
    tempMovie = malloc(sizeof(Movie));
    tempMovie->actors[0] = '\0';
    tempMovie->genre[0] = '\0';

    
    
    for(i = 0; i < r; i++){
        if (jsoneq(substring, &tokens[i], "Released") == 0) {
			/* We may use strndup() to fetch string value */
			char* released = malloc(sizeof(char) * (tokens[i+1].end - tokens[i+1].start));
			released = strndup(substring + tokens[i+1].start, tokens[i+1].end-tokens[i+1].start);
			strcpy(tempMovie->released, released);
			free(released);
			
			i++;
		} else if (jsoneq(substring, &tokens[i], "Rated") == 0) {
			/* We may additionally check if the value is either "true" or "false" */
			//printf("- Rated: %.*s\n", tokens[i+1].end-tokens[i+1].start, substring + tokens[i+1].start);
			char* rated = malloc(sizeof(char) * (tokens[i+1].end-tokens[i+1].start));
			rated = strndup(substring + tokens[i+1].start, tokens[i+1].end-tokens[i+1].start);
			strcpy(tempMovie->rated, rated);
			free(rated);
			i++;
		}
		else if (jsoneq(substring, &tokens[i], "Runtime") == 0) {
			/* We may additionally check if the value is either "true" or "false" */
			//printf("- Runtime: %.*s\n", tokens[i+1].end-tokens[i+1].start, substring + tokens[i+1].start);
			char* runtime = malloc(sizeof(char) * (tokens[i+1].end-tokens[i+1].start));
			runtime = strndup(substring + tokens[i+1].start, tokens[i+1].end-tokens[i+1].start);
			strcpy(tempMovie->runtime, runtime);
			free(runtime);
			i++;
		}
		else if (jsoneq(substring, &tokens[i], "Filename") == 0) {
			/* We may additionally check if the value is either "true" or "false" */
		//	printf("- Filename: %.*s\n", tokens[i+1].end-tokens[i+1].start, substring + tokens[i+1].start);
			char* filename = malloc(sizeof(char) * (tokens[i+1].end-tokens[i+1].start));
			filename = strndup(substring + tokens[i+1].start, tokens[i+1].end-tokens[i+1].start);
			strcpy(tempMovie->filename, filename);
			free(filename);
			i++;
		}
		else if (jsoneq(substring, &tokens[i], "Title") == 0) {
			/* We may additionally check if the value is either "true" or "false" */
			//printf("- Title: %.*s\n", tokens[i+1].end-tokens[i+1].start, substring + tokens[i+1].start);
		    //printf("%s", substring + tokens[i+1].start);
			char* title = malloc(sizeof(char) * (tokens[i+1].end-tokens[i+1].start));
			title = strndup(substring + tokens[i+1].start, tokens[i+1].end-tokens[i+1].start);
			strcpy(tempMovie->title, title);
			free(title);
			i++;
		}
		else if (jsoneq(substring, &tokens[i], "Plot") == 0) {
			/* We may want to do strtol() here to get numeric value */
		//	printf("- Plot: %.*s\n", tokens[i+1].end-tokens[i+1].start, substring + tokens[i+1].start);
			char* plot = malloc(sizeof(char) * (tokens[i+1].end-tokens[i+1].start));
			plot = strndup(substring + tokens[i+1].start, tokens[i+1].end-tokens[i+1].start);
			strcpy(tempMovie->plot, plot);
			free(plot);
			i++;
		}else if (jsoneq(substring, &tokens[i], "Actors") == 0) {
			int j;
			//printf("- Actor(s):\n");
			if (tokens[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			
			for (j = 0; j < tokens[i+1].size; j++) {
				jsmntok_t *g = &tokens[i+j+2];
				//printf("  * %.*s\n", g->end - g->start, substring + g->start);
				char* temp = malloc(sizeof(char) *(g->end - g->start));
				//temp is the string value of the actors from the json object
				temp = strndup(substring + g->start, g->end - g->start);
				char *actors = tempMovie->actors;
				temp = concat(actors, temp, NULL);
				strcpy(tempMovie->actors, temp);
				free(temp);
			}
			i += tokens[i+1].size+1;
		}
		else if (jsoneq(substring, &tokens[i], "Genre") == 0) {
			int j;
			if (tokens[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < tokens[i+1].size; j++) {
				jsmntok_t *g = &tokens[i+j+2];
				//printf("  * %.*s\n", g->end - g->start, substring + g->start);
				char* temp = malloc(sizeof(char) *(g->end - g->start));
				//temp is the string value of the genre from the json object
				temp = strndup(substring + g->start, g->end - g->start);
				char *genre = tempMovie->genre;
				temp = concat(genre, temp, NULL);
				strcpy(tempMovie->genre, temp);
				free(temp);
			}
			i += tokens[i+1].size+1;
		}
		
    }
    free(substring);
    return tempMovie;
}



char* parseMessage(char* message){
    char* ret;
    char* method = malloc(sizeof(char));
    char* params = malloc(sizeof(char));
    int r;
    jsmn_parser parser;
    jsmn_init(&parser);
    r = jsmn_parse(&parser, message, strlen(message), NULL, 0);
    if(r>0){
        jsmn_init(&parser);
        jsmntok_t tokens[r];
        jsmn_parse(&parser, message, strlen(message), tokens, r);
        int k;
        for(k = 0; k< r; k++){
            if (jsoneq(message, &tokens[k], "method") == 0) {
			/* We may use strndup() to fetch string value */
			method = malloc(sizeof(char) * (tokens[k+1].end - tokens[k+1].start));
			method = strndup(message + tokens[k+1].start, tokens[k+1].end-tokens[k+1].start);
			
			
			k++;
		    }else if (jsoneq(message, &tokens[k], "params") == 0) {
			int j;
			if (tokens[k+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < tokens[k+1].size; j++) {
				jsmntok_t *g = &tokens[k+j+2];
			     params = malloc(sizeof(char) *(g->end - g->start));
				params = strndup(message + g->start, g->end - g->start);
				
			}
			k += tokens[k+1].size+1;
		}
		
		
        }
    }
    if(strcmp(method, "getTitles") == 0){
	   
	    ret = getTitles();
	}
    free(params);
    free(method);
   // printf("\n%s\n",ret);
    return ret;
}

char* concat(const char *s1, const char *s2, const char *s3){
    char* result = NULL;
    if(s3 != NULL){
        result = malloc(strlen(s1)+strlen(s2)+strlen(s3)+1);
        if(result != NULL){
            strcpy(result, s1);
            strcat(result, s3);
            strcat(result, s2);
        }
    }else{
        result = malloc(strlen(s1)+strlen(s2)+1);
        if(result != NULL){
            strcpy(result, s1);
            strcat(result, s2);
        }
    }
    return result;
}


char* getTitles(){
    char* ret = " ";
    if(libraryCount > 0){
        int a = 0;
        while(a < libraryCount){
            printf("\n%s\n", DB[a].title);
            if(a == 0){
                ret = concat(ret, DB[a].title, NULL);
            }else{
                ret = concat(ret, DB[a].title, ",");
            }
            a++;
        } 
    }else{
        ret = "None";
    }
    return ret;
}


bool parseJson(char* filename){
    bool ret = false;
    char *JSON_STRING;
    int r;
    int i;
    
    JSON_STRING = readFile("movies.json");
    jsmn_parser parser;
    jsmn_init(&parser);
    r = jsmn_parse(&parser, JSON_STRING, strlen(JSON_STRING), NULL ,0);
    if(r>0){
        jsmn_init(&parser);
        jsmntok_t tokens[r];
        jsmn_parse(&parser, JSON_STRING, strlen(JSON_STRING), tokens, r);
        int k;
        //k=1 because we skip the first object...because the whole thing is an object
        for(k = 1; k < r; k++){
            if(tokens[k].type == 1){
               Movie* movieToAdd = parseObject(tokens[k], JSON_STRING);
               if(movieToAdd != NULL){
                   add(movieToAdd);
               }
               free(movieToAdd);
            }
        }
    }
    free(JSON_STRING);
    if(libraryCount > 0){
        ret = true;
    }
    return ret;
}

char* surroundInQuote(char* item){
    char*ret;
    printf("\nin surround\n");
    if(item != NULL){
        printf("\n its not null\n");
        printf("%s", item);
        ret = concat("\"",item, NULL);
        ret = concat(ret, "\"", NULL);
    }
    
    return ret;
}

char* packageMessage(char* message){
    char* ret;
    char* firstHalf = "{\"result\": [";
    char* end = "]}";
    ret = concat(firstHalf, message, NULL);
    ret = concat(ret, end, NULL);
    return ret;
}

void* readRequest(void* param){
    int sock = *((int *) param);
    
        int n;
        char* message = "";
        bool flag = true;
        while(flag){
            char buffer[256]= {0};
            n = recv(sock, buffer, sizeof(buffer), 0);
            if(n >= (int)sizeof(buffer)){
                message = concat(message, buffer, NULL);
            }else{
                message = concat(message, buffer, NULL);
                flag = false;
            }
        }
        
        char* parsed = parseMessage(message);
        parsed = packageMessage(parsed);
        printf("%s", parsed);
        send(sock, parsed, strlen(parsed),0);
        free(message);
        free(parsed);
}

int main(){
    int sockfd, newsockfd, portno, clilen, pid;
     struct sockaddr_in serv_addr, cli_addr;

     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi("8080");
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     printf("initalizing socket....");
     bool notConnected = true;
     while(notConnected){
         if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
            notConnected = true;
         
        }else{
            notConnected = false;
             printf("socket initialized");
        }
     }
     
              
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
    
    char *filename = "movies.json";
     bool didParse = parseJson(filename);
     if(!didParse){
         printf("Error parsing json, the library is empty");
         exit(0);
     }
    listAll();
    
    while(true){
        printf("\n Awaiting connections...\n");
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0){
           exit(0);
        }else{
            pthread_t myThread;
            pthread_create(&myThread, NULL, readRequest, (void*)&newsockfd);
            pthread_join(myThread, NULL);
        }
    }
}

















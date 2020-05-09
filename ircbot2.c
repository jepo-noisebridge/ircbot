#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

/* A simple IRC bot. Connects to an IRC network, joins a channel then sits
 * idle, responding to the server's PING messges and printing everything the
 * server sends it. */

/*So far I've added the ability to identify on the freenode irc. I registered and verified the nick jepbot through the email address jeremy@clued-in.com. */


static char *server  = "chat.freenode.net";
static char *channel = "#jepbot";
static char nick[32];
static int  isRegistered = 0;
static int  isIdentified = 0;
time_t rawtime;
struct tm * timeinfo;
static char *finaltimeinfo;
static char *spacer = " ";
static char *rplcdspacer = "\t";
struct tm * localtimeinfo;
char *str_replace(char *orig, char *rep, char *with);
char *streamdmssg = "";
char *stream_hostname = "";
//char mssg[300];
//char message_buffer[300];
char *username_begin;
char *username_end;
char *last_colon_mssg;
int mssg_length = 0;
int username_length = 0;
int username_length2 = 0;
char *username = "";
char *message = "";

struct {
char mssg[300];
char message_buffer[300];

} ircsrvrply;

static void onConnect(dyad_Event *e) {
  /* Generate a random nick name */
  sprintf(nick, "jepbot", (int)(dyad_getTime()) % 0xFFFF);
  /* Introduce ourselves to the server */
  dyad_writef(e->stream, "NICK %s\r\n", nick);
  dyad_writef(e->stream, "USER %s %s bla :%s\r\n", nick, nick, nick);
}

static void onError(dyad_Event *e) {
  printf("error: %s\n", e->msg);
}

static void onLine(dyad_Event *e) {

  printf("%s\n", e->data);

  //assign stream content to character array
  streamdmssg = e->data;

  username_end = strstr(streamdmssg, "!");
  username_begin = strstr(streamdmssg, ":");

  last_colon_mssg = strrchr(streamdmssg, ':'); 

  //overwrite previous string data in array
  memcpy(ircsrvrply.mssg, " ",300);
  memcpy(ircsrvrply.message_buffer, " ",300);


  mssg_length = strcspn(streamdmssg,"\0");
  username_length = strcspn(streamdmssg,"!");
  username_length2 = strlen(streamdmssg);


  // printf("The response is this many bytes long: %d\n", mssg_length);

  if(username_length != username_length2){
  memcpy(ircsrvrply.mssg, username_begin+1, username_length-1);
  memcpy(ircsrvrply.message_buffer, last_colon_mssg+1, mssg_length-1); 
  username =ircsrvrply.mssg;
  message = ircsrvrply.message_buffer;
    printf("\n%s\n\n", message);
  };


 //this will compare the entire stream with the message in quotes 
 //commenting out because is obselete. Stream is now parsed so that jepbot can respond to multiple users and commands
 /*
  if(!memcmp(e->data,":jepo!~jepo@unaffiliated/jepo PRIVMSG jepbot :Hello", 51)){
      printf("This is a test %s\n", e->data);
	  dyad_writef(e->stream, "PRIVMSG #jepbot Hello\t%s,\tI\tam\tjepbot!\r\n", username);
 	 
  }
*/
 
  //This will compare the message from the parsed stream with the char array "Howdy!" 
// char *howdy = "Howdy!";
  if(!memcmp(message, "Howdy!", 6)){
      printf("%s\n", e->data);
	  dyad_writef(e->stream, "PRIVMSG #jepbot Howdy\t%s,\tI\tam\tjepbot!\r\n", username);
  }

//This will compare the message from the parsed stream with the char array "Hello" 
// char *hello = "Hello";
  if(!memcmp(message, "Hello", 5)){
      printf("%s\n", e->data);
	  dyad_writef(e->stream, "PRIVMSG #jepbot Hello\t%s,\tI\tam\tjepbot!\r\n", username);
  }

//This will compare the message from the parsed stream with the char array "Hi" 
// char *hi = "Hi";
  if(!memcmp(message, "Hi", 2)){
      printf("%s\n", e->data);
	  dyad_writef(e->stream, "PRIVMSG #jepbot Hi\t%s,\tI\tam\tjepbot!\r\n", username);
  }






  if(!memcmp(e->data,":jepo!~jepo@unaffiliated/jepo PRIVMSG jepbot :hello", 51)){
      printf("%s\n", e->data);

	  dyad_writef(e->stream, "PRIVMSG #jepbot hello,\tI\tam\tjepbot!\r\n");
  }


  /* Handle PING */
  if (!memcmp(e->data, "PING", 4)) {
    dyad_writef(e->stream, "PONG%s\r\n", e->data + 4);
    printf("%s\n", e->data);
    time(&rawtime);
    localtimeinfo = localtime(&rawtime);
    finaltimeinfo = asctime(localtimeinfo);
    finaltimeinfo = str_replace(finaltimeinfo, spacer, rplcdspacer);
  
   //testing the effect of memset on finaltimeinfo. result is that this will replace the time field with asterisks 
   // memset(finaltimeinfo, '*', 24);

    dyad_writef(e->stream, "PRIVMSG #jepbot I\twas\tpinged\tat\t%s\r\n", finaltimeinfo);
    printf("%s\n", e->data);

  }

  /* Handle RPL_WELCOME */
  if (!isRegistered && strstr(e->data, "001")) {
    printf("%s\n", e->data);
	  /* Join channel */
    dyad_writef(e->stream, "JOIN %s\r\n", channel);
    isRegistered = 1;
  }
  if (isRegistered && strstr(e->data, "001")) {
    printf("%s\n", e->data);
	  /* Identify - note that the password for the registered Nick needs to be in here I have removed jepbots password*/
    dyad_writef(e->stream, "PRIVMSG NICKSERV :IDENTIFY 'Put Password Here' \n");
    isIdentified = 1;
  }

  if (!memcmp(e->data, "VERSION", 4)) {
    dyad_writef(e->stream, "NOTICE %s\r\n", e->data + 4);
  }

}

//All that sweet code that you found at stack overflow
// You must free the result if result is non-NULL.
char * str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}


int main(void) {
  dyad_Stream *s;
  char  *button = "N";

 printf("Welcome to jepbot2.0\nPress Y to run jepBot and Q to quit\n");
 int entered = scanf("%hhi",button[0]);
 printf("\n%s\n",button);
// if(!memcmp(button,'Y',1)){

  dyad_init();
  
  s = dyad_newStream();
  dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
  dyad_addListener(s, DYAD_EVENT_ERROR,   onError,   NULL);
  dyad_addListener(s, DYAD_EVENT_LINE,    onLine,    NULL);
  
  dyad_connect(s, server, 6667);

  while (dyad_getStreamCount() > 0) {
    dyad_update();

  }

 //}
 //if(button=='Q')
  dyad_shutdown();
  return 0;
}

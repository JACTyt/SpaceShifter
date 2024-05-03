#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <ctype.h>
     
struct gamedifficulty{
    int enemys;
    int enemy_steps;
    int stoppers;
    int coins;
};
//GAME PREFERENCE METHODS
int space_shifter(int difficulty);
void generate_field(struct gamedifficulty *gd);
void generate_znak(int color,int size, char znak);
int generate_number(int min, int max);
void set_game_difficulty(struct gamedifficulty *gd,int coins, int enemys, int enemy_steps,int stoppers);
void change_difficulty(int current_difficulty,struct gamedifficulty *gd, char current_difficulty_name[]);
void loadHUD(int level, char command[5],int lasers, int lifes, int coins);

//USER MOVE METHODS
void get_user_move(int[2],char command[5],int *lasers, int *coins, int *killedEnemys, int *lifes, bool *playerTurn, int *shots);
int move_x(const int,int[2]);
int move_y(const int,int[2]);
void bullet(const int position[], int side, char znak,int *coins, int *killedEnemys, int *lifes);
void laser(const int position[], int side, char znak, int *coins, int *killedEnemys, int *lifes);
//FOR TIME DELAY
int nanosleep(const struct timespec *req, struct timespec *rem);

void give_bonus(const char symbol, int *coins, int *killedEnemys, int *lifes);
bool game_is_over(int current_difficulty, int coins, int killedEnemys, int lifes, int shots);
void send_enemy(const int position[], int side, int steps, char znak,int *lifes);
void move_enemys(struct gamedifficulty *gd, int *lifes);
bool level_is_over(int current_difficulty, int coins, int killedEnemys, int lifes, int shots);
void loadMessageMenu(char center_message[],char lower_message[], int border_color, int text_color);
void loadBorderBox(int border_color,int text_color);
void initialise_color_pairs();
bool find_symbol(char symbol);
int calculate_score(int current_difficulty, int coins, int killedEnemys, int lifes);


int main(int argc, char *argv[]){
    int current_difficulty = 1;
    if(argc==2){
    	if(atoi(argv[1]) <=2 && atoi(argv[1]) >=0)
        	current_difficulty = atoi(argv[1]);
        else
            current_difficulty = 1;
    }
    space_shifter(current_difficulty);
    return 0;
}


int space_shifter(int difficulty){
    srand(time(NULL));
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
    start_color();
    initialise_color_pairs();
    delwin(stdscr);
    LINES = 11;
    COLS = 44;
    stdscr = newwin(LINES,COLS,0,0);

    struct gamedifficulty gd={2,2,2}; 
    int current_difficulty = difficulty;
    char current_difficulty_name[7];
    int lifes = 3; //Point this
    int coins = 0; //Point this
    int killedEnemys = 0; //Point this
    int level = 1; //Point this
    int lasers = 0; //Point this
    int shots = 0; //Point this
    char command[6]= "EMPTY\0";
    bool playerTurn = false; //Point this
    change_difficulty(current_difficulty,&gd,current_difficulty_name); // sOMETHING WITH THIS METHOD IN THIS LINE DONT WORKS 
    while (!game_is_over(current_difficulty, coins, killedEnemys, lifes, shots))
    {
        char string[40];
        if(level==1){
            sprintf(string,"Press any button to start game"); // 30
            char string2[40];
            sprintf(string2,"Game difficulty: %s",current_difficulty_name);
            loadMessageMenu(string,string2,3,1);
        }
        else{
            sprintf(string,"Press any button to start new level"); // 35
            loadMessageMenu(string,"",3,1);
        }
        clear();
        generate_field(&gd);
        int playerPosition[2]={5,3}; // x , y
        while (!level_is_over(current_difficulty, coins, killedEnemys, lifes, shots))
        {
            // HUD LOAD
            attron(COLOR_PAIR(3));
            mvprintw(playerPosition[1],playerPosition[0],">");
            loadHUD(level, command, lasers, lifes, coins);
            refresh();
            // Player Move
            playerTurn = true;
            get_user_move(playerPosition,command,&lasers, &coins, &killedEnemys, &lifes,&playerTurn,&shots);
            // Enemy Move
            if(!playerTurn){
                move_enemys(&gd,&lifes);
                refresh();
            }
            loadHUD(level, command, lasers, lifes, coins);
        }
        level++;
        lasers++;
        set_game_difficulty(&gd,gd.coins+level%3,gd.enemys+1+level%2,gd.enemy_steps,gd.stoppers-gd.stoppers/8);
    }
    endwin();
    return EXIT_SUCCESS;
}

void initialise_color_pairs(){
    //            Text      Background
    short backgroundColor = COLOR_BLACK;
	init_pair(1, COLOR_RED, backgroundColor);  // GAME BACKGROUND, MENU
	init_pair(2, COLOR_YELLOW,COLOR_RED); // HUD LINE
    init_pair(3,COLOR_GREEN,backgroundColor); // Player
    init_pair(4,COLOR_YELLOW, backgroundColor); // Lives Text
    init_pair(5, COLOR_GREEN,COLOR_GREEN); // -live(In game) line
    init_pair(6,COLOR_MAGENTA,backgroundColor); 
}

void change_difficulty(int difficulty,struct gamedifficulty *gd, char current_difficulty_name[]){
    switch (difficulty)
    {
        //EASY
    case 0:
        set_game_difficulty(&*gd,2,1,3,25);
        break;
        //NORMAL
    case 1:
        set_game_difficulty(&*gd,2,2,4,20);
        break;
        //HARD
    case 2:
        set_game_difficulty(&*gd,1,3,6,15);
        break;
    default:
        set_game_difficulty(&*gd,2,2,4,20);
        break;
    }
    
    switch (difficulty)
    {
        case 0:
            sprintf(current_difficulty_name,"EASY");
            break;
        case 1:
            sprintf(current_difficulty_name,"NORMAL");
            break;
        case 2:
            sprintf(current_difficulty_name,"HARD");
            break;
        default:
            sprintf(current_difficulty_name,"NORMAL");
            break;
    }
    
}

void set_game_difficulty(struct gamedifficulty *gd,int coins,int enemys,int enemy_steps,int stoppers){
    gd->coins = coins;
    gd->enemys = enemys;
    gd->stoppers = stoppers;
    gd->enemy_steps = enemy_steps;
}

void bullet(const int position[], int side, char znak,int *coins, int *killedEnemys, int *lifes){
    struct timespec ts = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 0.01 * 1000000000L  // nr of nanosecs
    };
    
    if(position[0] < 0 || position[0] > COLS-1 ){
        return;
    }
    int localposition[2] ={position[0],position[1] } ;
    mvprintw(position[1],position[0], " ");
    localposition[0] += side;
    attron(COLOR_PAIR(1));
    //mvprintw(localposition[1],localposition[0],"_");
    char symbol = mvinch(localposition[1],localposition[0]);
    if(symbol != ' '){
        mvprintw(localposition[1],localposition[0]," ");
        give_bonus(symbol,&*coins, &*killedEnemys, &*lifes);
        return;
    }
    mvprintw(localposition[1],localposition[0],"%c",znak);
    refresh();
    nanosleep(&ts,NULL);   
    bullet(localposition,side,znak,&*coins,&*killedEnemys,&*lifes);
    
}
void give_bonus(const char symbol, int *coins, int *killedEnemys, int *lifes){
    switch (symbol)
    {
    case 'C':
        *coins = *coins+1;
        break;
    case '<':
        *killedEnemys=*killedEnemys+1;
        break;
    case '+':
        //printf("Using Med Aid");
        *lifes=*lifes+1;
        break;
    case 'B':
        break;
    default:
        break;
    }
}

void laser(const int position[], int side, char znak, int *coins, int *killedEnemys, int *lifes){
    if(position[0] < 0 || position[0] > COLS-2){
        return;
    }
    int localposition[2] ={position[0],position[1] } ;
    mvprintw(position[1],position[0], " ");
    localposition[0] += side;
    attron(COLOR_PAIR(1));
    char symbol = mvinch(localposition[1],localposition[0]);
    if(symbol != ' '){
        mvprintw(localposition[1],localposition[0]," ");
        give_bonus(symbol,&*coins,&*killedEnemys,&*lifes);
    }
    mvprintw(localposition[1],localposition[0],"%c",znak);
    refresh();
    struct timespec ts = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 0.01 * 1000000000L  // nr of nanosecs
    };
    nanosleep(&ts,NULL);   
    laser(localposition,side,znak,&*coins,&*killedEnemys,&*lifes);
}

void move_enemys(struct gamedifficulty *gd, int *lifes){
    for (int i = 0; i < COLS; i++)
    {
        for (int j = 0; j < LINES; j++)
        {
            char got_symbol = mvinch(j,i);
            int coords[] = {i,j};
            if(got_symbol == '<')   
                send_enemy(coords,-1,gd->enemy_steps,'<', &*lifes);
        }
    }
}

void send_enemy(const int position[], int side, int steps,char znak,int *lifes){ //***
    int localposition[2] ={position[0],position[1] } ;
    mvprintw(position[1],position[0], " ");
    //printf("ENEME = %i", steps);
    localposition[0] += side;
    attron(COLOR_PAIR(1));
    if((steps==0)&&(localposition[0]==5)){
        localposition[0] +=side;
    }
    mvprintw(localposition[1],localposition[0],"%c",znak);
    refresh();
    if(localposition[0] <= 1){
        *lifes= *lifes-1;
        return;
    }
    if(steps<=0){
        return;
    }
    struct timespec ts = {
        .tv_sec = 0,                    // nr of secs
        .tv_nsec = 0.01 * 1000000000L  // nr of nanosecs
    };
    nanosleep(&ts,NULL);   
    send_enemy(localposition,side,--steps,znak,&*lifes);
}

void get_user_move(int playerPosition[2],char command[5],int *lasers, int *coins, int *killedEnemys, int *lifes, bool *playerTurn, int *shots){
    int input = getchar();
    //mvprintw(0,0,"%c",command[0]);
    //char c = input; 
    for (int i =  0 ; i < strlen(command) ; i++)
    {
        command[i] = command[i+1];
    }
    command[strlen(command)] = toupper(input);

    // COMMANDS
    char *is_exit = strstr(command, "EXIT");
    if(is_exit != NULL){
        endwin();
        exit(EXIT_SUCCESS);
    }
    char *is_laser = strstr(command, "LAZER");
    if(is_laser != NULL && lasers>0){
        *lasers= *lasers -1;
        laser(playerPosition, 1, '>',&*coins,&*killedEnemys,&*lifes);        
    }
    char *is_author = strstr(command, "DEV");
    if(is_author != NULL){
        attron(COLOR_PAIR(4));
        mvprintw(LINES-3,3,"Made by Dmytro Maistruk");
        struct timespec ts = {
            .tv_sec = 0,                    // nr of secs
            .tv_nsec = 0.01 * 1000000000L  // nr of nanosecs
        };
        nanosleep(&ts,NULL);
        attroff(COLOR_PAIR(4));
        refresh();        
    }
    switch (input)
    {
        case 'w' : case 'W': case KEY_UP:
            move_y(-1,playerPosition);
            break;
        case 's': case 'S': case KEY_DOWN:
            move_y(1,playerPosition);
            break;
        case ' ':
            //mvprintw(LINES/2,COLS/2 ,"BULLET");
            *playerTurn = false;
            bullet(playerPosition, 1, '>', &*coins, &*killedEnemys,&*lifes);
            *shots = *shots + 1;
            break;
        default:
            break;
    }
}

void generate_field(struct gamedifficulty *gd){
    generate_znak(6,gd->stoppers,'#');
    generate_znak(4,gd->coins,'C');
    generate_znak(1,gd->enemys,'<');
    //BONUSES    
    //Aid kit
    int a = generate_number(0,1);
    generate_znak(3,a,'+');
    //Bomb
    /*
    int b = generate_number(0,level-1);
    generate_znak(3,b,'B');
    */
}
void generate_znak(int color,int size, char znak){
    int minCols = 12;
    attron(COLOR_PAIR(color));
    for (int i = 0; i < size; i++)
    {
        char symbol;
        int x,y;
        do
        {
        x = generate_number(minCols,COLS-1);   
        y = generate_number(1,LINES-1-5);
        symbol = mvinch(y,x);
        } while (symbol!=' ');
        mvprintw(y,x,"%c",znak);
    }    
}

void loadHUD(int level, char command[5],int lasers, int lifes, int coins){
    loadBorderBox(3,3);
    int Line = LINES - 5;
    attron(COLOR_PAIR(2));
    /*
    for (int i = 0; i < COLS; i++)
    {
        mvprintw(Line,i," ");
    }
    */
    char line = '-';
    for (int i = 1; i < COLS-1; i++)
    {
        attron(COLOR_PAIR(3));
        mvprintw(Line,i,"%c",line);
    }
    
    // HUD
    Line = LINES-3; 
    attron(COLOR_PAIR(4));
    char string[64];
    sprintf(string,"/Health: %i /Coins: %i",lifes,coins);
    mvprintw(Line-1,3,"%s",string);
    //mvprintw(Line+1,COLS/2-strlen(string)/2,"%s",string);
    sprintf(string,"/Lasers: %i /Level: %i /COMMAND: %s",lasers,level,command);
    mvprintw(Line+1,3,"%s",string);
    //mvprintw(Line+1,COLS/2-strlen(string)/2,"%s",string);
    
    for (int i = 1; i < 2; i++)
    {
        for (int j = 1; j < LINES-5; j++)
        {
            attron(COLOR_PAIR(5));
            mvprintw(j,i," ");
            attroff(COLOR_PAIR(5));
        }
        
    }
    
    
}

bool find_symbol(char symbol){
    for (int i = 0; i < COLS; i++)
    {
        for (int j = 0; j < LINES; j++)
        {
            char got_symbol = mvinch(j,i);
            if(got_symbol == symbol)   
                return true;
        }
    }
    return false;
}

void loadMessageMenu(char center_message[],char lower_message[], int border_color, int text_color){
    clear();
    loadBorderBox(border_color,border_color);
    attron(COLOR_PAIR(text_color));
    mvprintw(LINES/2, COLS/2 - strlen(center_message)/2,"%s",center_message);
    mvprintw((LINES/2)+2, COLS/2 - strlen(lower_message)/2,"%s",lower_message);
    refresh();
    getchar();
    clear();
}
void loadBorderBox(int border_color,int text_color){
    attron(COLOR_PAIR(border_color));
    box(stdscr,0,0);
    attron(COLOR_PAIR(text_color));
    mvprintw(0,2,"Space Shifter");
}

int move_x(int side, int position[2]){
    mvprintw(position[1],position[0]," ");
    if(side > 0){
        if(position[0] < COLS-1)
            position[0]+= side;
    }else if ( side < 0)
    {
        if(position[0] > 0)
            position[0]+= side;
    }
    return 0;
}

int move_y(const int side, int position[2]){
    mvprintw(position[1],position[0]," ");
    position[1] +=side;
    if(position[1]>LINES-1-5){
        position[1] = 1;
    }
    else if (position[1]< 1)
    {
        position[1] = LINES-1-5;
    }
    return 0;
}


bool game_is_over(int current_difficulty, int coins, int killedEnemys, int lifes, int shots){
    if(coins>=10){
        //Win
        char string[34];  
        char string2[128];
        sprintf(string,"You Won! Your score: %i",calculate_score(current_difficulty, coins, killedEnemys, lifes));
        sprintf(string2,"Coins: %i Killed enemys: %i Shots: %i",coins,killedEnemys,shots);
        loadMessageMenu(string,string2,3,4);
        endwin();
        exit(EXIT_SUCCESS);
        return 1;
    }
    return 0;
}
int calculate_score(int current_difficulty, int coins, int killedEnemys, int lifes){
    // Enemy - 10 points, Health - 50 points, Coins - 100 points
    //EASY - *1, NORMAL - *2, HARD - *3
    return ((killedEnemys*20 + lifes*50 + coins*100)*(current_difficulty+1));
}
bool level_is_over(int current_difficulty, int coins, int killedEnemys, int lifes, int shots){
    
    if(lifes<=0){
        char string[34];  
        char string2[128];
        sprintf(string,"You Lost! Your score: %i",calculate_score(current_difficulty, coins, killedEnemys, lifes));
        sprintf(string2,"Coins: %i Killed enemys: %i Shots: %i",coins,killedEnemys,shots);
        loadMessageMenu(string,string2,1,1);
        endwin();
        exit(EXIT_SUCCESS);
        return 1;
    }
    if(!find_symbol('<')){
        return 1;
    }
    return 0;
}

int generate_number(int min, int max){
    //srand(time(NULL));
    int number = min + rand() % (max - min + 1);
    return number;
}

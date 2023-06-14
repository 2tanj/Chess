#include "stdc++.h"
#include "GL/glut.h"
#include "FMOD/fmod.hpp"

#include "Chess/Game.h"
#include "Model.h"
#include "Sprite.h"

enum GameState {
    SPLASH,
    MENU,
    CONTROLS,
    IN_PROGRESS,
    MODE_SELECTION,
    PAUSED,
    END // Currently unimplemented
};

// GLOBAL VARIABLES

// Window size and position
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_POS_X 50
#define WINDOW_POS_Y 50

// Length of splashscreen
#define SPLASH_THRESHOLD 3500

/**
    Variables for look at function
    eye     - position of eye
    center  - position of the center of view
    up      - up vertex
*/
GLfloat eyeX = 5.0,    eyeY = 0.0,    eyeZ = -5.0,
        centerX = 0.0, centerY = 0.0, centerZ = 0.0,
        upX = 0.0,     upY = 0.0,     upZ = -1.0;


// Variables for perspective function
GLfloat     fovy = 50.0, zNear = 0.1, zFar = 20.0;


// Variables for light
GLfloat position[] =          { 0.0f, 0.0f, 100.0f, 0.0f };
GLfloat diffusion[] =         { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat normal_board[] =      { 0.0f, 0.0f, 1.0f };
GLfloat normal_valid_move[] = { 0.0f, 0.0f, -1.0f };
GLfloat mat_diffusion[] =     { 0.8, 0.8, 0.8, 1.0 };
GLfloat mat_specular[] =      { 0.1, 0.1, 0.1, 1.0 };
float   ang = 0;

// Variables for managing view
GLfloat screen_ratio, zoomOut = 2;


// Model Loading
Model Pawn   ("Models/Pawn.obj");
Model Rook   ("Models/Rook.obj");
Model Knight ("Models/Knight.obj");
Model Bishop ("Models/Bishop.obj");
Model King   ("Models/King.obj");
Model Queen  ("Models/Queen.obj");

// Variables for sprites
Sprite* splashScreen;
Sprite* menuScreen;
Sprite* controlsScreen;
Sprite* backgroundScreen;
Sprite* pausedScreen;
Sprite* modeScreen;
Sprite* arrowIcon;

// Variables for sounds
FMOD::System* audioMgr;
FMOD::Sound* sfxClick;
FMOD::Sound* sfxTicking;
FMOD::Sound* sfxOver;
FMOD::Channel* backgroundChannel;
FMOD::Channel* sfxChannel;
bool isOverPlayed = false, isTickingPlayed = false; // used for preventing constant playing of sounds


// Array used for storing all the positions of the main menu arrow used for navigating
Sprite::Point arrowPositions[3] = { {445, 416}, {445, 353}, {445, 290} };
uint16_t arrowIndex = 2;

// Pre start
bool pressed = false;

// Game loading
Game* chess;
void newGame();


// Real-time variables
GameState gameState;
bool inGame = false, verify = false;
int  selectedRow = 1, selectedCol = 1;
int  moveToRow = 1, moveToCol = 1;
bool selected = false;
bool board_rotating = true;
int  rotation = 0;
bool check = false, checkMate = false, timeOut = false;
bool closeGame = false;
bool needPromote = false;

//                                                  default time set to 5 mins           ---     unimplemented    ---
float elapsedTime = 0, startTime = 0, goalTime = 0, selectedTime = 300000, showTime = 0, whiteTime = 0, blackTime = 0;
float pausedTime = 0, unPausedTime = 0;
bool unlimitedGame = false;


// Chess board vertices
GLfloat    chessBoard[12][3] = {{-4.0, -4.0, 0.5},
                                {-4.0,  4.0, 0.5},
                                { 4.0,  4.0, 0.5},
                                { 4.0, -4.0, 0.5},

                                {-4.5, -4.5, 0.5},
                                {-4.5,  4.5, 0.5},
                                { 4.5,  4.5, 0.5},
                                { 4.5, -4.5, 0.5},

                                {-5.0, -5.0, 0.0},
                                {-5.0,  5.0, 0.0},
                                { 5.0,  5.0, 0.0},
                                { 5.0, -5.0, 0.0}};


void showWord(int x, int y, std::string word)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WINDOW_WIDTH / 2, WINDOW_WIDTH / 2, -WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 2, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    int l, i;

    l = word.length(); // see how many characters are in text string.
    glRasterPos2i(x, y); // location to start printing text
    glColor3f(0, 0, 0);
    for (i = 0; i < l; i++) // loop until i is greater then l
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, word[i]); // Print a character on the screen
    }
}

/**
     Returns full time from seconds, from 126 to 02:06

BUG: doesnt work for times over 10 minutes, it will write "012:15"
     redundant to implement that here
*/
std::string printTime(int time) 
{
    if (time < 60)
        return std::to_string(time) + 's';

    int minutes = time / 60;
    int seconds = time % 60;

    // returns the time formated properly, if seconds < 10 we add a 0 before the seconds to avoid ->(02:5)
    return seconds < 10 ? 
        '0' + std::to_string(minutes) + ':' + '0' + std::to_string(seconds) :
        '0' + std::to_string(minutes) + ':' + std::to_string(seconds);
}

bool initFmod()
{
    FMOD_RESULT result;
    result = FMOD::System_Create(&audioMgr);
    if (result != FMOD_OK)
    {
        return false;
    }
    result = audioMgr->init(50, FMOD_INIT_NORMAL, NULL);
    if (result != FMOD_OK)
    {
        return false;
    }
    return true;
}

const bool loadAudio()
{
    FMOD_RESULT result;

    result = audioMgr->createSound("Sounds/clockTicking.wav", FMOD_LOOP_NORMAL, 0, &sfxTicking);

    result = audioMgr->createSound("Sounds/gameOver.wav", FMOD_DEFAULT, 0, &sfxOver);
    result = audioMgr->createSound("Sounds/wood.wav", FMOD_DEFAULT, 0, &sfxClick);

    return true;
}

void initSprites() {
    splashScreen = new Sprite(1);
    splashScreen->SetFrameSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    splashScreen->SetNumberOfFrames(1);
    splashScreen->AddTexture("Sprites/splash.png", false);
    splashScreen->IsActive(true);
    splashScreen->IsVisible(true);
    splashScreen->SetPosition(0.0f, 0.0f);

    menuScreen = new Sprite(1);
    menuScreen->SetFrameSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    menuScreen->SetNumberOfFrames(1);
    menuScreen->AddTexture("Sprites/mainmenu.png", false);
    menuScreen->IsActive(true);
    menuScreen->IsVisible(true);
    menuScreen->SetPosition(0.0f, 0.0f);

    controlsScreen = new Sprite(1);
    controlsScreen->SetFrameSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    controlsScreen->SetNumberOfFrames(1);
    controlsScreen->AddTexture("Sprites/controls.png", false);
    controlsScreen->IsActive(true);
    controlsScreen->IsVisible(true);
    
    backgroundScreen = new Sprite(1);
    backgroundScreen->SetFrameSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    backgroundScreen->SetNumberOfFrames(1);
    backgroundScreen->AddTexture("Sprites/background.png", false);
    backgroundScreen->IsActive(true);
    backgroundScreen->IsVisible(true);
    
    pausedScreen = new Sprite(1);
    pausedScreen->SetFrameSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    pausedScreen->SetNumberOfFrames(1);
    pausedScreen->AddTexture("Sprites/paused.png", false);
    pausedScreen->IsActive(true);
    pausedScreen->IsVisible(true);
    
    modeScreen = new Sprite(1);
    modeScreen->SetFrameSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    modeScreen->SetNumberOfFrames(1);
    modeScreen->AddTexture("Sprites/mode.png", false);
    modeScreen->IsActive(true);
    modeScreen->IsVisible(true);
    
    arrowIcon = new Sprite(1);
    arrowIcon->SetFrameSize(100, 50);
    arrowIcon->SetNumberOfFrames(1);
    arrowIcon->AddTexture("Sprites/arrowChess.png", false);
    arrowIcon->IsActive(true);
    arrowIcon->IsVisible(true);
    arrowIcon->UseTransparency(true);
    arrowIcon->SetPosition(WINDOW_WIDTH / 2 + 45, 290);
}

/**
    Drawing the "cursor" when a piece is picked
*/
void drawMoveToSquare()
{
    float r = 1.0 * (moveToRow - 5), c = 1.0 * (moveToCol - 5);
    if (selected)
    {
        glPushMatrix();
            glColor3f(0.5f, 1.0f, 0.0f);
            glTranslatef(r, c, 0.502f);
            glScalef(0.98f, 0.98f, 1.0f);
            glBegin(GL_TRIANGLES);
                glNormal3fv(normal_valid_move);
                glVertex3f(0.0f, 0.0f, 0.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);
                glVertex3f(0.0f, 1.0f, 0.0f);
                glVertex3f(0.0f, 0.0f, 0.0f);
                glVertex3f(1.0f, 1.0f, 0.0f);
                glVertex3f(1.0f, 0.0f, 0.0f);
            glEnd();
        glPopMatrix();
    }
    glColor3f(0, 0, 0);
}

/**
    Drawing a chess board using points from array "chessBoard"
*/
void drawChessBoard()
{
    glPushMatrix();
        /**Drawing bottom of the chess board*/
        glNormal3fv(normal_valid_move);
        glBegin(GL_QUADS);
            glColor3f(1.0, 0.0, 0.0);
            for (int i = 8; i < 12; i++) glVertex3fv(chessBoard[i]);
        glEnd();
        /**Drawing top of the chess board*/
        glBegin(GL_QUADS);
            glColor3f(0.55, 0.24, 0.09);
            glColor3f(0.803, 0.522, 0.247);
            glVertex3fv(chessBoard[0]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[4]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[5]);
            glColor3f(0.803, 0.522, 0.247);
            glVertex3fv(chessBoard[1]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.803, 0.522, 0.247);
            glVertex3fv(chessBoard[1]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[5]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[6]);
            glColor3f(0.803, 0.522, 0.247);
            glVertex3fv(chessBoard[2]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.803, 0.522, 0.247);
            glVertex3fv(chessBoard[2]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[6]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[7]);
            glColor3f(0.803, 0.522, 0.247);
            glVertex3fv(chessBoard[3]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.803, 0.522, 0.247);
            glVertex3fv(chessBoard[3]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[7]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[4]);
            glColor3f(0.803, 0.522, 0.247);
            glVertex3fv(chessBoard[0]);
        glEnd();
        /**Drawing side of the chess board*/
        glBegin(GL_QUADS);
            glColor3f(1.0, 0.95, 0.9);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[4]);
            glColor3f(1.000, 1.000, 1.000);
            glVertex3fv(chessBoard[8]);
            glColor3f(1.000, 1.000, 1.000);
            glVertex3fv(chessBoard[9]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[5]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[5]);
            glColor3f(1.000, 1.000, 1.000);
            glVertex3fv(chessBoard[9]);
            glColor3f(1.000, 1.000, 1.000);
            glVertex3fv(chessBoard[10]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[6]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[6]);
            glColor3f(1.000, 1.000, 1.000);
            glVertex3fv(chessBoard[10]);
            glColor3f(1.000, 1.000, 1.000);
            glVertex3fv(chessBoard[11]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[7]);
        glEnd();
        glBegin(GL_QUADS);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[7]);
            glColor3f(1.000, 1.000, 1.000);
            glVertex3fv(chessBoard[11]);
            glColor3f(1.000, 1.000, 1.000);
            glVertex3fv(chessBoard[8]);
            glColor3f(0.545, 0.271, 0.075);
            glVertex3fv(chessBoard[4]);
        glEnd();
    glPopMatrix();
    glColor3f(0, 0, 0);
}

void drawBoardSquares()
{
    float r, c;
    for (int row = 1; row <= 8; row++)
    {
        for (int col = 1; col <= 8; col++)
        {
            r = 1.0 * (row - 5);
            c = 1.0 * (col - 5);
            if (row == selectedRow && col == selectedCol)
            {
                if (selected) glColor3f(0.33f, 0.420f, 0.184f);
                else if (chess->isSquareOccupied(selectedRow, selectedCol))
                    if (chess->getPieceColor(selectedRow, selectedCol) == chess->getTurnColor())
                        glColor3f(0.0f, 0.5f, 0.0f);
                    else glColor3f(1.0f, 0.0f, 0.0f);
                else glColor3f(0.3f, 0.7f, 0.5f);
            }
            else if ((row + col) & 1) glColor3f(1.0, 1.0, 1.0);
            else glColor3f(0.0, 0.0, 0.0);
            glPushMatrix();
                glTranslatef(r, c, 0.5f);
                glBegin(GL_TRIANGLES);
                    glNormal3fv(normal_board);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(1.0f, 1.0f, 0.0f);
                    glVertex3f(0.0f, 1.0f, 0.0f);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(1.0f, 1.0f, 0.0f);
                    glVertex3f(1.0f, 0.0f, 0.0f);
                glEnd();
            glPopMatrix();
        }
    }
    glColor3f(0, 0, 0);
}

/**
    Draw Valid Moves of Selected Piece
*/
void drawValidMoves()
{
    if (selected)
    {
        std::vector<Move> validMoves = chess->getValidMoves(selectedRow, selectedCol);
        int vec_size = validMoves.size(), row, col;
        for (int id = 0; id < vec_size; id++)
        {
            row = validMoves[id].getDestinationPosition().first;
            col = validMoves[id].getDestinationPosition().second;
            switch (validMoves[id].getType())
            {
            case MoveType::NORMAL:
                glColor3f(0.8f, 1.0f, 0.6f);
                break;
            case MoveType::CAPTURE:
                glColor3f(1.0f, 0.0f, 0.0f);
                break;
            case MoveType::EN_PASSANT:
                glColor3f(0.8f, 1.0f, 0.6f);
                break;
            case MoveType::CASTLING:
                glColor3f(0.196f, 0.804f, 0.196f);
                break;
            }
            float r = 1.0 * (row - 5), c = 1.0 * (col - 5);
            glPushMatrix();
                glTranslatef(r, c, 0.501);
                glScalef(0.99f, 0.99f, 1.0f);
                glBegin(GL_TRIANGLES);
                    glNormal3fv(normal_valid_move);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(1.0f, 1.0f, 0.0f);
                    glVertex3f(0.0f, 1.0f, 0.0f);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(1.0f, 1.0f, 0.0f);
                    glVertex3f(1.0f, 0.0f, 0.0f);
                glEnd();
            glPopMatrix();
        }
    }
    glColor3f(0, 0, 0);
}

/**
    Drawing Chess Pieces in Board
*/
void drawChessPieces()
{
    float z;
    for (int row = 1; row <= 8; row++)
    {
        for (int col = 1; col <= 8; col++)
        {
            if (chess->isSquareOccupied(row, col))
            {
                glPushMatrix();
                if (selected && row == selectedRow && col == selectedCol) z = 1.0;
                else z = 0.6;
                glTranslatef((row - 5) * 1.0f + 0.5f, (col - 5) * 1.0f + 0.5f, z);
                glScalef(.18f, .18f, .18f);
                switch (chess->getPieceColor(row, col))
                {
                case PieceColor::WHITE:
                    glRotatef(90, 0.0f, 0.0f, 1.0f);
                    glColor3f(0.9f, 0.9f, 0.9f);
                    break;
                case PieceColor::BLACK:
                    glRotatef(-90, 0.0f, 0.0f, 1.0f);
                    glColor3f(0.1f, 0.1f, 0.1f);
                    break;
                }
                switch (chess->getPiece(row, col)->getType())
                {
                    case PieceType::PAWN: Pawn.Draw(); break;
                    case PieceType::ROOK: Rook.Draw(); break;
                    case PieceType::KNIGHT: Knight.Draw(); break;
                    case PieceType::BISHOP: Bishop.Draw(); break;
                    case PieceType::QUEEN: Queen.Draw(); break;
                    case PieceType::KING: King.Draw(); break;
                }
                glPopMatrix();
            }
        }
    }
    glColor3f(0, 0, 0);
}

void key_W_pressed(PieceColor color)
{
    switch (color)
    {
    case PieceColor::WHITE:
        if (!selected && selectedRow < 8) selectedRow++;
        if (selected && moveToRow < 8) moveToRow++;
        break;
    case PieceColor::BLACK:
        if (!selected && selectedRow > 1) selectedRow--;
        if (selected && moveToRow > 1) moveToRow--;
        break;
    }
}

void key_D_pressed(PieceColor color)
{
    switch (color)
    {
    case PieceColor::WHITE:
        if (!selected && selectedCol < 8) selectedCol++;
        if (selected && moveToCol < 8) moveToCol++;
        break;
    case PieceColor::BLACK:
        if (!selected && selectedCol > 1) selectedCol--;
        if (selected && moveToCol > 1) moveToCol--;
        break;
    }
}

void key_S_pressed(PieceColor color)
{
    switch (color)
    {
    case PieceColor::WHITE:
        if (!selected && selectedRow > 1) selectedRow--;
        if (selected && moveToRow > 1) moveToRow--;
        break;
    case PieceColor::BLACK:
        if (!selected && selectedRow < 8) selectedRow++;
        if (selected && moveToRow < 8) moveToRow++;
        break;
    }
}

void key_A_pressed(PieceColor color)
{
    switch (color)
    {
    case PieceColor::WHITE:
        if (!selected && selectedCol > 1) selectedCol--;
        if (selected && moveToCol > 1) moveToCol--;
        break;
    case PieceColor::BLACK:
        if (!selected && selectedCol < 8) selectedCol++;
        if (selected && moveToCol < 8) moveToCol++;
        break;
    }
}

void updateTurn(PieceColor color)
{
    switch (color)
    {
    case PieceColor::WHITE:
        selectedRow = 1;
        selectedCol = 8;
        break;
    case PieceColor::BLACK:
        selectedRow = 8;
        selectedCol = 1;
        break;
    }
}

void doRotationBoard(PieceColor color)
{
    switch (color)
    {
    case PieceColor::WHITE:
        if (rotation < 180) rotation += 4;
        else board_rotating = false;
        break;
    case PieceColor::BLACK:
        if (rotation < 360) rotation += 4;
        else
        {
            rotation = 0;
            board_rotating = false;
        }
        break;
    }
}

void endOfTurn()
{
    selected = false;
    needPromote = false;
    check = false;
    chess->nextTurn();
    if (chess->inCheckMateState())
    {
        checkMate = true;
    }
    else if (chess->inCheckState())
    {
        check = true;
    }
    board_rotating = true;
    updateTurn(chess->getTurnColor());
}

void Enable2D()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
}

void Disable2D()
{
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void Render2D() 
{
    Enable2D();

    switch (gameState)
    {
    case SPLASH:
        if (elapsedTime >= SPLASH_THRESHOLD)
            gameState = MENU;

        splashScreen->Render();
        break;

    case MENU:
        menuScreen->Render();

        arrowIcon->SetPosition(arrowPositions[arrowIndex]);
        arrowIcon->Render();
        break;

    case CONTROLS:
        controlsScreen->Render();
        break;

    case PAUSED:
        isTickingPlayed = false;
        backgroundChannel->setPaused(true);

        pausedScreen->Render();

        arrowIcon->SetPosition(arrowPositions[arrowIndex]);
        arrowIcon->Render();
        break;
    
    case MODE_SELECTION:
        modeScreen->Render();

        arrowIcon->SetPosition(arrowPositions[arrowIndex]);
        arrowIcon->Render();
        break;

    case IN_PROGRESS:
        // starting the ticking sound once, when the game starts
        if (!isTickingPlayed) {
            audioMgr->playSound(sfxTicking, 0, false, &backgroundChannel);
            backgroundChannel->setPaused(false);
            backgroundChannel->setLoopCount(-1);
            backgroundChannel->setVolume(1);
            
            isTickingPlayed = true;
        }
        // stoping the sound when the game ends
        if (timeOut || checkMate && isTickingPlayed) {
            backgroundChannel->setPaused(true);
            isTickingPlayed = false;
        }

        backgroundScreen->Render();
        break;
    }
    Disable2D();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    elapsedTime = glutGet(GLUT_ELAPSED_TIME);

    Render2D();

    if (gameState == GameState::IN_PROGRESS) 
    {
        /**
            Changing view perspective
        */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fovy, screen_ratio, zNear, zoomOut * zFar);
        /**
            Drawing model mode
        */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(zoomOut * eyeX, zoomOut * eyeY, zoomOut * eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

        /**
            Draw code here
        */
        if (board_rotating) doRotationBoard(chess->getTurnColor());
        GLfloat ambient_model[] = { 0.5, 0.5, 0.5, 1.0 };

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_model);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_diffusion);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffusion);

        glScalef(1.0f, 1.0f, -1.0f);

        glLightfv(GL_LIGHT0, GL_POSITION, position);

        glRotatef(rotation, 0, 0, 1);

        drawChessBoard();
        drawBoardSquares();
        drawChessPieces();
        drawMoveToSquare();
        drawValidMoves();

        // Time calculation and printing
        if (!unlimitedGame) {
            showTime = (goalTime - (elapsedTime - startTime));

            if (showTime / 1000 <= 0)
                timeOut = true;
            if(!checkMate && !timeOut)
                showWord(-27, WINDOW_HEIGHT / 2 - 70, printTime(showTime / 1000));
        }
        if (!checkMate && !timeOut) {
            std::string s = chess->getTurnColor() == PieceColor::BLACK ? "BLACK's MOVE" : "WHITE's MOVE";
            showWord(-92, WINDOW_HEIGHT / 2 - 50, s);
        }

        // Text printing when needed
        if (needPromote)
        {
            showWord(-200, WINDOW_HEIGHT / 2 - 24, "Promote to: (Q) Queen | (R) Rook | (B) Bishop | (K) Knight");
        }
        else if (verify) showWord(-200, WINDOW_HEIGHT / 2 - 24, "Are you sure to retry? Yes (O)  or  No (X)");
        else
        {
            if (check)
            {
                std::string s = chess->getTurnColor() == PieceColor::BLACK ? "BLACK PIECE" : "WHITE PIECE";
                showWord(-150, WINDOW_HEIGHT / 2 - 24, s + " CHECKED!");
            }
            if (checkMate)
            {
                if (!isOverPlayed) {
                    audioMgr->playSound(sfxOver, 0, false, &sfxChannel);
                    sfxChannel->setVolume(3);
                    isOverPlayed = true;
                }

                std::string s = chess->getTurnColor() == PieceColor::BLACK ? "WHITE PLAYER" : "BLACK PLAYER";
                showWord(-100, WINDOW_HEIGHT / 2 - 24, "CHECK MATE!");
                showWord(-140, WINDOW_HEIGHT / 2 - 50, s + " WIN!");
                showWord(-150, -WINDOW_HEIGHT / 2 + 50, "Do you want to play again?");
                showWord(-120, -WINDOW_HEIGHT / 2 + 25, "Yes (O)  or  No (X)");
            }
            if (timeOut) 
            {
                if (!isOverPlayed) {
                    audioMgr->playSound(sfxOver, 0, false, &sfxChannel);
                    sfxChannel->setVolume(3);
                    isOverPlayed = true;
                }
                
                std::string s = chess->getTurnColor() == PieceColor::BLACK ? "WHITE PLAYER" : "BLACK PLAYER";
                showWord(-80, WINDOW_HEIGHT / 2 - 24, "TIMEOUT!");
                showWord(-120, WINDOW_HEIGHT / 2 - 50, s + " WIN!");
                showWord(-150, -WINDOW_HEIGHT / 2 + 50, "Do you want to play again?");
                showWord(-120, -WINDOW_HEIGHT / 2 + 25, "Yes (O)  or  No (X)");
            }
        }
    }
    if (closeGame) exit(0)/*glutDestroyWindow(glutGetWindow())*/;

    // TODO: timer za svaki tim, ikonice playmode

    // izmeni render funkciju za sprajtove i tamo manuelno menjaj boju figurica
    // nova funkcija za render2d gde samo iscrtavam ikonice koje su dodate u vector(poseban za oba tima)
    // ova funkcija se poziva na kraju ove kako bi se iscrtala preko svega
    // definisi gornji levi i desni cosak i onda za iscrtavanje svake dodaj startingSize + iconGap

    //Render2D();


    glutSwapBuffers();

    glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    screen_ratio = (GLdouble)width / (GLdouble)height;
}

void specialFunc(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        if (gameState == GameState::MENU || gameState == GameState::PAUSED || gameState == GameState::MODE_SELECTION) {
            if (arrowIndex == 2) arrowIndex = 0;
            else arrowIndex++;
            return;
        }
        zoomOut -= 0.1;
        break;
    case GLUT_KEY_DOWN:
        if (gameState == GameState::MENU || gameState == GameState::PAUSED || gameState == GameState::MODE_SELECTION) {
            if (arrowIndex == 0) arrowIndex = 2;
            else arrowIndex--;
            return;
        }
        zoomOut += 0.1;
        break;
    case GLUT_KEY_LEFT:
        rotation -= 5;
        //ang += 5;
        break;
    case GLUT_KEY_RIGHT:
        rotation += 5;
        //ang -= 5;
        break;
    default: break;
    }

}

#define INGAME gameState == GameState::IN_PROGRESS

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (gameState == GameState::PAUSED) return;
        if (!needPromote && !checkMate && !timeOut && !verify && INGAME && !board_rotating) key_W_pressed(chess->getTurnColor());
        break;
    case 'a':
    case 'A':
        if (gameState == GameState::PAUSED) return;
        if (!needPromote && !checkMate && !timeOut && !verify&& !timeOut  && INGAME && !board_rotating) key_A_pressed(chess->getTurnColor());
        break;
    case 's':
    case 'S':
        if (gameState == GameState::PAUSED) return;
        if (!needPromote && !checkMate && !timeOut && !verify && INGAME && !board_rotating) key_S_pressed(chess->getTurnColor());
        break;
    case 'd':
    case 'D':
        if (gameState == GameState::PAUSED) return;
        if (!needPromote && !checkMate && !timeOut && !verify && INGAME && !board_rotating) key_D_pressed(chess->getTurnColor());
        break;
    case ' ':
        if (gameState == GameState::MENU) {
            audioMgr->playSound(sfxClick, 0, false, &sfxChannel);
            sfxChannel->setVolume(3);

            switch (arrowIndex) {
            case 0:
                exit(0);
                break;
            case 1:
                gameState = GameState::CONTROLS;
                break;
            case 2:
                gameState = GameState::MODE_SELECTION;
                break;
            }
        }
        else if (gameState == GameState::PAUSED) {
            audioMgr->playSound(sfxClick, 0, false, &sfxChannel);
            sfxChannel->setVolume(3);

            switch (arrowIndex) {
            case 0:
                exit(0);
                break;
            case 1:
                arrowIndex = 2;
                gameState = GameState::MENU;
                break;
            case 2:
                unPausedTime = glutGet(GLUT_ELAPSED_TIME);
                goalTime += (unPausedTime - pausedTime) / 2;
                startTime += (unPausedTime - pausedTime) / 2;
                gameState = GameState::IN_PROGRESS;
                break;
            }
        }
        else if (gameState == GameState::MODE_SELECTION) {
            audioMgr->playSound(sfxClick, 0, false, &sfxChannel);
            sfxChannel->setVolume(3);

            switch (arrowIndex) {
            case 0:
                unlimitedGame = true;
                break;
            case 1:
                //selectedTime = 1000; for testing(timer = 1sec)
                selectedTime = 90000;
                unlimitedGame = false;
                break;
            case 2:
                selectedTime = 300000;
                unlimitedGame = false;
                break;
            }
            newGame();
        }
        else if (gameState == GameState::CONTROLS) { 
            audioMgr->playSound(sfxClick, 0, false, &sfxChannel); 
            sfxChannel->setVolume(3);

            gameState = GameState::MENU; 
        }

        if (!needPromote && !checkMate && !timeOut && !verify && INGAME && !board_rotating)
        {
            if (selected)
            {
                audioMgr->playSound(sfxClick, 0, false, &sfxChannel);
                sfxChannel->setVolume(3);

                if (chess->move(selectedRow, selectedCol, moveToRow, moveToCol))
                {
                    Piece* movedPiece = chess->getPiece(moveToRow, moveToCol);
                    if (movedPiece->getType() == PieceType::PAWN &&
                        ((movedPiece->getColor() == PieceColor::BLACK && moveToRow == chess->getBoard()->MIN_ROW_INDEX)
                            || moveToRow == chess->getBoard()->MAX_ROW_INDEX))
                    {
                        needPromote = true;
                    }
                    if (needPromote) break;
                    endOfTurn();
                }
                selected = false;
            }
            else if (chess->isSquareOccupied(selectedRow, selectedCol) && chess->getPieceColor(selectedRow, selectedCol) == chess->getTurnColor())
            {
                selected = !selected;
                if (selected)
                {
                    moveToRow = selectedRow;
                    moveToCol = selectedCol;
                }
            }
        }
        break;
    case 'n':
    case 'N':
        if (gameState == GameState::PAUSED) return;
        if (gameState != GameState::IN_PROGRESS) newGame();
        else verify = true;
        break;
    case 'o': case 'O':
        if (checkMate || timeOut || verify) { delete chess; newGame(); verify = false; }
        break;
    case 'x': case 'X':
        if (checkMate || timeOut) { closeGame = true; delete chess; }
        if (verify) { verify = false; }
        break;
    case 'q': case 'Q':
        if (needPromote)
        {
            chess->promote(moveToRow, moveToCol, PieceType::QUEEN);
            endOfTurn();
            break;
        }
        else break;
    case 'r': case 'R':
        if (needPromote)
        {
            chess->promote(moveToRow, moveToCol, PieceType::ROOK);
            endOfTurn();
            break;
        }
        else break;
    case 'b': case 'B':
        if (needPromote)
        {
            chess->promote(moveToRow, moveToCol, PieceType::BISHOP);
            endOfTurn();
            break;
        }
        else break;
    case 'k': case 'K':
        if (needPromote)
        {
            chess->promote(moveToRow, moveToCol, PieceType::KNIGHT);
            endOfTurn();
            break;
        }
        else break;
    case 'p': case 'P':
        if (gameState == GameState::PAUSED) {
            // Compensating for time lost during pause
            unPausedTime = glutGet(GLUT_ELAPSED_TIME);
            goalTime += (unPausedTime - pausedTime) / 2;
            startTime += (unPausedTime - pausedTime) / 2;
            gameState = GameState::IN_PROGRESS;
            return;
        }

        // Restricting pauses outside of in_progress mode
        if (gameState != GameState::IN_PROGRESS)
            return;
        
        pausedTime = glutGet(GLUT_ELAPSED_TIME);
        gameState = GameState::PAUSED;
        break;

    default: break;
    }
}

void init()
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SMOOTH);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_COLOR_MATERIAL);

    initFmod();
    loadAudio();

    initSprites();
    gameState = GameState::SPLASH;

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void newGame()
{
    chess = new Game();
    selectedRow = 1; selectedCol = 1;
    moveToRow = 1; moveToCol = 1;
    selected = false;
    board_rotating = true;
    rotation = 0;
    inGame = true;
    gameState = GameState::IN_PROGRESS;
    check = false;
    checkMate = false;
    timeOut = false;
    isOverPlayed = false;
    isTickingPlayed = false;
    goalTime = selectedTime;
    startTime = glutGet(GLUT_ELAPSED_TIME);
    updateTurn(chess->getTurnColor());
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(WINDOW_POS_X, WINDOW_POS_Y);
    glutCreateWindow("CS233 - 4550 - Sah");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialFunc);
    glutMainLoop();
    return 0;
}

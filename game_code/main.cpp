#include <windows.h>
#include <stdint.h>
#include <windowsx.h>
#include <math.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <algorithm>

#define internal static 
#define local_persist static 
#define global_variable static
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool OnPlay = true;

global_variable bool Running;
global_variable bool NewGame = false;

// Basic colors 
global_variable uint32 Black = 0x00000000;
global_variable uint32 White = 0xffffffff;
global_variable uint32 Grey = 0x00b4b4b4;

// Displayed Bitmap declaration
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable float BitmapXPosition = 0;  
global_variable int BitmapWidth = 1000;
global_variable int BitmapHeight = 800;
global_variable int BytesPerPixel = 4;

// Timers needed for some animations
global_variable float LevelTimer = 402;
global_variable float FinishLevelTimer = 0;
global_variable float ShrinkingTimeCounter = 0;
global_variable double ShroomJumpTimeCounter = 0;
global_variable double LoopTime = 0;

// Sprites
global_variable char MarioLevelFileName[] = "..\\levels\\FirstLevel.txt";
global_variable char MarioSpritesFileName[] = "../sprites/mariobros.png";
global_variable uint32 *MarioSpritesImage = NULL;
global_variable char StaticObjectsFileName[] = "..\\sprites\\tileset.png";
global_variable uint32* StaticObjectsImage = NULL;
global_variable char EnemiesFileName[] = "..\\sprites\\enemies.png";
global_variable uint32* EnemiesImage = NULL;
global_variable char BackgroundsFileName[] = "..\\sprites\\background_images.png";
global_variable uint32* BackgroundsImage = NULL;
global_variable char SaveButtonFileName[] = "..\\sprites\\SaveButton.png";
global_variable uint32* SaveButtonImage = NULL;
global_variable char WelcomePageFileName[] = "..\\sprites\\mario_menu.png";
global_variable uint32* WelcomePage = NULL;
global_variable char NumLettersFileName[] = "..\\sprites\\num_letters.png";
global_variable uint32* NumLettersImage = NULL;

// Sprites size (width, height)
global_variable int MarioSpritesWidth;
global_variable int MarioSpritesHeight;
global_variable int BackgroundsImageWidth;
global_variable int BackgroundsImageHeight;
global_variable int StaticObjectsImageWidth;
global_variable int StaticObjectsImageHeight;
global_variable int EnemiesImageWidth;
global_variable int EnemiesImageHeight;
global_variable int SaveButtonImageWidth;
global_variable int SaveButtonImageHeight;
global_variable int WelcomePageWidth;
global_variable int WelcomePageHeight;
global_variable int NumLettersWidth;
global_variable int NumLettersHeight;

// number of all the possible tiles  in the game
global_variable const int MarioImagesNbr = 18;
global_variable const int StaticObjectsImgsNbr = 23;
global_variable int const EnemiesImgsNbr = 6;
global_variable int const BackgroundImgsNbr = 45;

// base resolution caracteristics
global_variable int MarioOriginalWidth = 16;
global_variable int MarioOriginalHeight= 16;

global_variable float MarioMaxWalkingVelocity = float(MarioOriginalWidth) / 0.11;

global_variable int StaticObjectWidth = 16;
global_variable int StaticObjectHeight = 16;

global_variable int const OriginalBitmapWidth = 480;
global_variable int const OriginalBitmapHeight = 240;
global_variable uint32 OriginalBitmap[OriginalBitmapWidth* OriginalBitmapHeight];

// music setting booleans 
global_variable bool SetDeathVel = false;
global_variable bool DeathMusic = false;
global_variable bool BackgroundMusic = false;
global_variable bool FinishLevelMusicStop = false;

// Other global variables 
global_variable int MarioScore = 0;
global_variable int CoinCounter = 0;
global_variable int FlagX = -1;
global_variable float FlagY = -1;
global_variable float Gravity = 300;

LARGE_INTEGER PerformanceFrequencyL;

enum {
    MOVING_BIG,
    SLIDING_BIG = MOVING_BIG + 3,
    JUMPING_BIG,
    SQUATTING_BIG,
    STANDING_BIG,
    HANGING_BIG,
    MOVING,
    SLIDING = MOVING + 3,
    JUMPING,
    SQUATTING,
    STANDING,
    HANGING,
    DYING,
    SHROOM,
};

enum
{
    BRICK,
    BRICK2,
    BRICK3,
    QM,
    VTUBETL,
    VTUBETR,
    VTUBEBL,
    VTUBEBR,
    HTUBETL,
    HTUBETR,
    HTUBEBL,
    HTUBEBR,
    QM2,
    QM3,
    QM4,
    COIN1,
    COIN2,
    COIN3,
    COIN4,
    POLEUP,
    POLEDOWN,
    FLAG,
};

enum
{
    WORM,
    TURTLE = 3
};

struct KeyboardState {
    bool LeftArrowDown = false;
    bool RightArrowDown = false;
    bool UpArrowDown = false;
    bool DownArrowDown = false;
    bool EnterDown = false;
    void init() {
        LeftArrowDown = false;
        RightArrowDown = false;
        UpArrowDown = false;
        DownArrowDown = false;
        EnterDown = false;
    }
};
global_variable KeyboardState MyKeyboard;

struct MouseState {
    bool LeftButtonDown = false;
    bool RightButtonDown = false;
    bool LeftButtonHeld = false;
    bool RightButtonHeld = false;
    void operator=(MouseState A)
    {
        LeftButtonDown = A.LeftButtonDown;
        RightButtonDown = A.RightButtonDown;
    }
};
global_variable MouseState MyMouseState;

struct RealPoint {
    float X = -1;
    float Y = -1;
    RealPoint operator+(RealPoint A)
    { 
        RealPoint Result;
        Result.X = A.X + X;
        Result.Y = A.Y + Y;
        return Result;
    }
    RealPoint operator-(RealPoint A)
    {
        RealPoint Result;
        Result.X = X - A.X;
        Result.Y = Y - A.Y;
        return Result;
    }
    RealPoint operator*(RealPoint A)
    {
        RealPoint Result;
        Result.X = X * A.X;
        Result.Y = Y * A.Y;
        return Result;
    }
    void operator=(RealPoint A)
    {
        X = A.X;
        Y = A.Y;
    }
};

struct RectangleSize {
    int Width;
    int Height;
};

struct Point {
    int X = -1;
    int Y = -1;
    void operator=(Point A) {
        X = A.X;
        Y = A.Y;
    }
    bool operator==(Point A) {
        return (A.X == X && A.Y == Y);
    }
};

// static objects (especially Bricks here), can be unbroken, breaking, broke. can collide with other elements 
struct StaticObject {
    char Type;
    Point TopLeft;
    bool HasMushroom = false;
    bool AnimateMushroom = false;
    bool ShouldBreak = false;
    bool IsAlive = true;
};
global_variable int StaticObjectsNbr;
// array of all static objects
global_variable StaticObject* Brick = NULL;

// background objects, just images, no collision 
struct BgndObject {
    Point TopLeft;
    int Identifier;
};
global_variable int BgndNbr;
// array of all background objects
global_variable BgndObject* Bgnd = NULL;

// the moving displayed score, can be more than one
struct ScoreStruct
{
    RealPoint InitialPos;
    RealPoint ScorePos;
    int Value;
    char Length;
};
global_variable std::vector<ScoreStruct> Scores;

Point MouseCoords = { 0,0 };
POINT MouseRealCoords;

// Info to store when displaying multiple moving coins
struct CoinBrickMvtStruct {
    Point InitialPos;
    float Vel;
    RealPoint LastPos;
    RealPoint RealPos;
    bool StartedLaunching = false;
    bool HeightCheckOn = false;
    float ImgIndex = 0;
};

// Info needed to display multiple Breaking bricks 
struct BrickBreakingInfo{
    RealPoint PartsPos[4] = { 0 };
    RealPoint PartsVel[4] = { 0 };
    bool startedBreaking = false;
};

global_variable std::vector<CoinBrickMvtStruct> CoinsToLaunchInfo;
global_variable std::vector<CoinBrickMvtStruct> BricksToShakeInfo;
global_variable std::vector<int> ShakyBricksIndices;
global_variable std::vector<BrickBreakingInfo> BricksToBreakInfo;
global_variable std::vector<int> BrokeBricksIndices;


// Coordinates of tiles in the sprites, all the tiles are represented as such
struct ExtractAndDisplayInfo {
    Point ColumnAndRaw = { 0,0 };
    RectangleSize OriginalSize = { 0,0 };
    void operator=(ExtractAndDisplayInfo A)
    {
        ColumnAndRaw.X = A.ColumnAndRaw.X;
        ColumnAndRaw.Y = A.ColumnAndRaw.Y;

        OriginalSize.Height = A.OriginalSize.Height;
        OriginalSize.Width = A.OriginalSize.Width;
    }
};

global_variable ExtractAndDisplayInfo MarioImagesInfo[MarioImagesNbr];
global_variable ExtractAndDisplayInfo StaticObjectsInfo[StaticObjectsImgsNbr];
global_variable ExtractAndDisplayInfo EnemiesImagesInfo[EnemiesImgsNbr];

struct Enemy {
    RealPoint CurrentPosition;
    RealPoint LastPosition;
    RealPoint Velocity = { -float(StaticObjectWidth) / float(0.5), 0 };

    float DyingTimeCounter = 0;
    
    bool DyingVelSet = false;
    bool YPosChanged = false;
    int DyingImIdx = 2;
    int ImgToShowIdx = 0;
    bool IsAlive = true;
    bool KilledByTurtle = false;
    int StampedOn = 0;
    bool IsLimitedDown = true;
    int Type = 0;
    bool StartMoving = false;
};
global_variable int EnemiesNbr;
global_variable Enemy* Enemies = NULL;

// main caracter data structure
struct Caracter {
    int ImageToShowIndx = 0;
    RealPoint CurrentPosition;
    RealPoint LastPosition;
    RealPoint Velocity;
    RealPoint Acceleration;
        
    bool ShouldWalk = false;
    bool WalkingRight= true;
    bool StartedWalking = true;
    float InitialWalkVelocity = 0;
    float WalkingAccDuration = 2;

    bool ShouldSlide = false;
    bool ShouldJump = false;
    float InitialJumpPosY = 0;
    float JumpVelocity ;
    float JumpHeight ;
    float JumpDuration ;
    
    bool IsLimitedDown = true;
    
    bool IsBig = false;
    bool IsSquatting = false;
    bool IsDying = false;
    bool FinishedLevel = false;
    bool IsShrinking = false;
    
    bool AnimateGrowing = false;
    bool AnimateShrinking = false;
};

global_variable Caracter Mario;

// Info needed to animate shroom 
struct ShroomStruct
{
    RealPoint InitialPos;
    RealPoint Position ;
    RealPoint LastPosition;
    RealPoint Vel;
    bool Rising = true;
    bool StartedMoving = false;
};

global_variable std::vector<ShroomStruct> ShroomsInfo;

// Reads a .bmp file and extracts the image data and stores it in Image.
internal void
GetBMPPixelArray(uint32** Image, char* FileName, int *Width = NULL, int *Height = NULL, int *FileBytesPerPixel = NULL)
{
    HANDLE FileHnd = CreateFileA(FileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    uint8 DataBuffer[4] = {};
    LPVOID VPDataBuffer = (void*)DataBuffer;
    DWORD NbBytesToRead = 4;
    DWORD BytesRead = 0;
    DWORD FilePointer = 0;

    uint32 BitmapDataStartingPointer;
    FilePointer = SetFilePointer(FileHnd, 10, NULL, 0);
    ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
    BitmapDataStartingPointer = (DataBuffer[3] << 24 | DataBuffer[2] << 16 | DataBuffer[1] << 8 | DataBuffer[0]);

    uint32 ImageWidth;
    FilePointer = SetFilePointer(FileHnd, 18, NULL, 0);
    ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
    ImageWidth = (DataBuffer[3] << 24 | DataBuffer[2] << 16 | DataBuffer[1] << 8 | DataBuffer[0]);
    if (Width)
        *Width = ImageWidth;

    uint32 ImageHeight;
    FilePointer = SetFilePointer(FileHnd, 22, NULL, 0);
    ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL); 
    ImageHeight = (DataBuffer[3] << 24 | DataBuffer[2] << 16 | DataBuffer[1] << 8 | DataBuffer[0]);
    if (Height)
        *Height = ImageHeight;

    FilePointer = SetFilePointer(FileHnd, 28, NULL, 0);
    ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
    if (FileBytesPerPixel) {
        *FileBytesPerPixel = (DataBuffer[1] << 8 | DataBuffer[0]);
        *FileBytesPerPixel /= 8;
    }
    int NumPixels = ImageWidth * ImageHeight;
    
    uint32* MyImage = new uint32[NumPixels];

    if (FileBytesPerPixel)
        NbBytesToRead = *FileBytesPerPixel;
    else
        NbBytesToRead = 3;

    FilePointer = SetFilePointer(FileHnd, BitmapDataStartingPointer, NULL, 0);
    for(int Y = ImageHeight - 1 ; Y >= 0; Y--)
        for (int X = 0; X < ImageWidth; X++) {
            ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
            MyImage[Y*ImageWidth + X] = (DataBuffer[2] << 16 | DataBuffer[1] << 8 | DataBuffer[0]);
        }
    *Image = MyImage;
    CloseHandle(FileHnd);
}

// Function to insert color or a total image and stretch it if necessary into a zone on destination, it checks transparency when source is a png
// this one is used to insert background colors and full images and stretch them 
internal void
InsertStretchImageToZone(uint32* SrcBitmap, int SrcWidth, int SrcHeight, uint32* DestBitmap, int DestWidth, int DestHeight, 
                         bool FromPng = false, Point ZoneTopLeft = { 0, 0 }, Point ZoneRightBottom = { -1, -1 }) {
    
    uint32 TransparencyMask = 0xff000000;
    uint32 Pixel;
    
    // Destination zone 
    if (ZoneRightBottom.X == -1)
        ZoneRightBottom = { DestWidth, DestHeight };
    int ZoneWidth = ZoneRightBottom.X - ZoneTopLeft.X;
    int ZoneHeight = ZoneRightBottom.Y - ZoneTopLeft.Y;
    
    for (int Y = ZoneTopLeft.Y; Y < ZoneRightBottom.Y; Y++)
    {
        for (int X = ZoneTopLeft.X; X < ZoneRightBottom.X; X++)
        {
            int Ysrc = (float(SrcHeight - 1) / (ZoneHeight - 1)) * (Y - ZoneTopLeft.Y);
            int Xsrc = (float(SrcWidth - 1) / (ZoneWidth - 1)) * (X - ZoneTopLeft.X);
            Pixel = SrcBitmap[Ysrc * SrcWidth + Xsrc];
            if(FromPng)
            {
                if (Pixel & TransparencyMask)
                {
                    uint32 Red = Pixel & 0x000000ff;
                    uint32 Green = Pixel & 0x0000ff00;
                    uint32 Blue = Pixel & 0x00ff0000;
                    Pixel = (Red << 16) | Green | (Blue >> 16);
                    DestBitmap[Y * DestWidth + X] = Pixel;

                }
            }
            else
                DestBitmap[Y * DestWidth + X] = Pixel;            
        }
    }
}

internal bool 
IsInDisplayRange(Point ImgPos, int ImgWidth, int ImgHeight, int DisplayXPos, int DisplayWidth, int DisplayHeight) {
    if (ImgPos.X + ImgWidth - int(DisplayXPos) > 0 && ImgPos.X < int(DisplayXPos) + DisplayWidth && ImgPos.Y + ImgHeight > 0 && ImgPos.Y < DisplayHeight)
        return true;
    return false;
}


internal void
LoadMarioImages() {
    int n;
    MarioSpritesImage = (uint32*)stbi_load(MarioSpritesFileName, &MarioSpritesWidth, &MarioSpritesHeight, &n, 0);
    StaticObjectsImage = (uint32*)stbi_load(StaticObjectsFileName, &StaticObjectsImageWidth, &StaticObjectsImageHeight, &n, 0);
    EnemiesImage = (uint32*)stbi_load(EnemiesFileName, &EnemiesImageWidth, &EnemiesImageHeight, &n, 0);
    BackgroundsImage = (uint32*)stbi_load(BackgroundsFileName, &BackgroundsImageWidth, &BackgroundsImageHeight, &n, 0);
    WelcomePage = (uint32*)stbi_load(WelcomePageFileName, &WelcomePageWidth, &WelcomePageHeight, &n, 0);
    NumLettersImage = (uint32*)stbi_load(NumLettersFileName, &NumLettersWidth, &NumLettersHeight, &n, 0);
}

// Inserts a zone(topleft + width + height) from sprites to the display bitmap(borders probs), no stretching, does horizontal and vertical inverse, 
// IsStaticObject in order to adjust extraction coords, caracter sprites and static objects sprites are separated ad organised differently
internal void 
InsertImage(uint32* OriginalSprite, int SpriteWidth, ExtractAndDisplayInfo ImgInfo, Point ImgPos, bool HorInverse = false, bool IsStaticObject = false, bool VerInverse = false)
{
    int ImgWidth = ImgInfo.OriginalSize.Width;
    int ImgHeight = ImgInfo.OriginalSize.Height;
    if (IsInDisplayRange(ImgPos, ImgWidth, ImgHeight, BitmapXPosition, OriginalBitmapWidth, OriginalBitmapHeight))
    {
        Point SrcImgPos;
        if(!IsStaticObject)
            SrcImgPos = { ImgInfo.ColumnAndRaw.X * MarioOriginalWidth, ImgInfo.ColumnAndRaw.Y * 2* MarioOriginalHeight };
        else
            SrcImgPos = { ImgInfo.ColumnAndRaw.X * StaticObjectWidth, ImgInfo.ColumnAndRaw.Y * StaticObjectHeight };

        for (int Y = ImgPos.Y; Y < ImgPos.Y + ImgHeight; Y++)
        {
            if (Y < 0)
                Y = 0;
            if (Y == OriginalBitmapHeight)
                break;
            for (int X = ImgPos.X - int(BitmapXPosition); X < ImgPos.X + ImgWidth - int(BitmapXPosition); X++)
            {
                if (X == OriginalBitmapWidth)
                    break;
                if (X < 0)
                    X = 0;

                int Ysrc = Y - ImgPos.Y;
                int Xsrc = X - (ImgPos.X - int(BitmapXPosition));
                uint32 TransparencyMask = 0xff000000;
                uint32 Pixel;
                if (!VerInverse)
                    if (!HorInverse)
                        Pixel = OriginalSprite[(Ysrc + SrcImgPos.Y) * SpriteWidth + Xsrc + SrcImgPos.X];
                    else
                        Pixel = OriginalSprite[(Ysrc + SrcImgPos.Y) * SpriteWidth + (SrcImgPos.X + ImgWidth - Xsrc - 1)];
                else
                    Pixel = OriginalSprite[(SrcImgPos.Y + ImgHeight - 1 - Ysrc) * SpriteWidth + Xsrc + SrcImgPos.X];
                if (Pixel & TransparencyMask)
                {
                    uint32 Red = Pixel & 0x000000ff;
                    uint32 Green = Pixel & 0x0000ff00;
                    uint32 Blue = Pixel & 0x00ff0000;
                    OriginalBitmap[Y * OriginalBitmapWidth + X] = (Red << 16) | Green | (Blue >> 16);
                }
            }
        }
    }
}

internal void
InsertBgndObjects() {
    for (int i = 0; i < BgndNbr; i++)
    {
        int OriginalYIndex = int(float(Bgnd[i].Identifier * 16) / BackgroundsImageWidth);
        int OriginalXIndex = ((Bgnd[i].Identifier * 16) % BackgroundsImageWidth) / 16;
        InsertImage(BackgroundsImage, BackgroundsImageWidth, { {OriginalXIndex, OriginalYIndex}, {StaticObjectWidth, StaticObjectHeight} }, Bgnd[i].TopLeft, false, true);
    }

}

internal void
InsertString(char* MyString, Point StringPos) {

    int i = 0;
    while (MyString[i] != '\0')
    {
        int LetterPosInImage = MyString[i] - 'a';
        InsertImage(NumLettersImage, NumLettersWidth, { {LetterPosInImage, 1},  {16,16} }, { StringPos.X + 16 * i, StringPos.Y }, false, true);
        i++;
    }

}

internal void
InsertScoreAndTime() {

    // SCORE
    Point TopLeft = { 2 * StaticObjectWidth , 3 * StaticObjectWidth / 2 };
    for (int i = 0; i < 6; i++)
    {
        int Digit = int(MarioScore / pow(10, i)) - int(MarioScore / pow(10, i + 1)) * 10;
        Point DigitPos = { TopLeft.X + (5 - i) * 16 + int(BitmapXPosition), TopLeft.Y };
        InsertImage(NumLettersImage, NumLettersWidth, { {Digit, 0}, {16, 16} }, DigitPos, false, true);
        
    }
    char MyString[] = "mario";
    Point StringPos = { TopLeft.X + int(BitmapXPosition), TopLeft.Y - 16 };
    InsertString(MyString, StringPos);
    
    // TIME
    char MyString1[] = "time";
    Point TimePos = { OriginalBitmapWidth - 6 * 16 + int(BitmapXPosition), TopLeft.Y - 16 };
    InsertString(MyString1, TimePos);
    for (int i = 0; i < 3; i++)
    {
        int Digit = int(LevelTimer / pow(10, i)) - int(LevelTimer / pow(10, i + 1)) * 10;
        Point DigitPos = { TimePos.X + (2 - i) * 16 ,  TimePos.Y + 16 };
        InsertImage(NumLettersImage, NumLettersWidth, { {Digit, 0}, {16, 16} }, DigitPos, false, true);
    }

    //Coin Count 
    InsertImage(StaticObjectsImage, StaticObjectsImageWidth, StaticObjectsInfo[COIN3], { TopLeft.X + 12 * 16 + int(BitmapXPosition), TopLeft.Y }, false, true);
    InsertImage(NumLettersImage, NumLettersWidth, { {10, 2}, {10, 10} }, { TopLeft.X + 12 * 16 + int(BitmapXPosition) + 16, TopLeft.Y +3 }, false, true);

    for (int i = 0; i < 2; i++)
    {
        int Digit = int(CoinCounter / pow(10, i)) - int(CoinCounter/ pow(10, i + 1)) * 10;
        Point DigitPos = { TopLeft.X + 14 * 16 + (1 - i) * 16 + int(BitmapXPosition),  TopLeft.Y };
        InsertImage(NumLettersImage, NumLettersWidth, { {Digit, 0}, {16, 16} }, DigitPos, false, true);
    }
}

internal void 
InsertBricks()
{
    for (int i = 0; i < StaticObjectsNbr; i++)
    {
        if(Brick[i].IsAlive)
        {
            if (Brick[i].Type == QM || Brick[i].Type == QM2 || Brick[i].Type == QM3)
            {
                local_persist float ImgIndex = 0;
                float AnimationDuration = 3;
                ImgIndex += LoopTime * 5 / AnimationDuration;
                if (ImgIndex >= 5)
                    ImgIndex = 0;

                switch (int(ImgIndex))
                {
                case 0: case 1:
                {
                    Brick[i].Type = QM;
                }break;
                case 2: case 4:
                {
                    Brick[i].Type = QM2;
                }break;
                case 3:
                {
                    Brick[i].Type = QM3;
                }break;
                default:
                    break;
                }
            }
            InsertImage(StaticObjectsImage, StaticObjectsImageWidth, StaticObjectsInfo[Brick[i].Type], Brick[i].TopLeft, false, true);
        }

    }
}

internal void
InsertEnemies()
{
    for (int i = 0; i < EnemiesNbr; i++)
    {
        Point EnemyPos = { int(Enemies[i].CurrentPosition.X), int(Enemies[i].CurrentPosition.Y) };
        if (Enemies[i].IsAlive)
            InsertImage(EnemiesImage, EnemiesImageWidth, EnemiesImagesInfo[Enemies[i].ImgToShowIdx], EnemyPos, (Enemies[i].Velocity.X > 0), false, Enemies[i].KilledByTurtle);

    }
}

internal void 
TreatEnemiesMvtAndColl()
{
    int MarioHeight = MarioImagesInfo[Mario.ImageToShowIndx].OriginalSize.Height;
    int MarioWidth = MarioImagesInfo[Mario.ImageToShowIndx].OriginalSize.Width;
    int EnemyWidth = 16;
    int BrickHeight = 16;
    int BrickWidth = 16;

    for(int i =0; i < EnemiesNbr; i++)
    {
        if(Enemies[i].IsAlive)
        {
            int EnemyHeight = EnemiesImagesInfo[Enemies[i].ImgToShowIdx].OriginalSize.Height;
            if (!Enemies[i].StartMoving && Enemies[i].CurrentPosition.X - Mario.CurrentPosition.X < OriginalBitmapWidth)
                Enemies[i].StartMoving = true;

            if (Enemies[i].StartMoving && ((Enemies[i].Type == TURTLE && (Enemies[i].StampedOn == 0 || Enemies[i].StampedOn == 2)) ||
                (Enemies[i].Type == WORM && (Enemies[i].StampedOn == 0 || Enemies[i].KilledByTurtle))))
            {
                Enemies[i].LastPosition.X = Enemies[i].CurrentPosition.X;
                Enemies[i].CurrentPosition.X = LoopTime * Enemies[i].Velocity.X + Enemies[i].LastPosition.X;
                Enemies[i].Velocity.Y = Gravity * LoopTime + Enemies[i].Velocity.Y;
                Enemies[i].LastPosition.Y = Enemies[i].CurrentPosition.Y;
                Enemies[i].CurrentPosition.Y = LoopTime * Enemies[i].Velocity.Y + Enemies[i].LastPosition.Y;

                if (Enemies[i].IsLimitedDown && !Enemies[i].KilledByTurtle)
                {
                    local_persist float ImgIndex = 0;
                    float AnimationDuration = 5;
                    ImgIndex += LoopTime * 2 / AnimationDuration;
                    if (ImgIndex >= 2)
                        ImgIndex = 0;
                    Enemies[i].ImgToShowIdx = Enemies[i].Type + ImgIndex;
                }

            }

            if (!Enemies[i].KilledByTurtle)
            {
                //Collision with static objects 
                for (int j = 0; j < StaticObjectsNbr; j++)
                {
                    if(Brick[j].IsAlive)
                    {
                        if (Enemies[i].CurrentPosition.X + EnemyWidth > Brick[j].TopLeft.X && Enemies[i].CurrentPosition.X < Brick[j].TopLeft.X + BrickWidth)
                        {
                            if (Enemies[i].CurrentPosition.Y < Brick[j].TopLeft.Y + BrickHeight && Enemies[i].LastPosition.Y >= Brick[j].TopLeft.Y + BrickHeight)
                            {
                                Enemies[i].CurrentPosition.Y = Brick[j].TopLeft.Y + BrickHeight;
                                Enemies[i].Velocity.Y = 0;
                            }
                            if (Enemies[i].CurrentPosition.Y + EnemyHeight > Brick[j].TopLeft.Y && Enemies[i].LastPosition.Y + EnemyHeight <= Brick[j].TopLeft.Y)
                            {
                                Enemies[i].CurrentPosition.Y = Brick[j].TopLeft.Y - EnemyHeight;
                                Enemies[i].Velocity.Y = 0;
                            }
                        }
                        if (Enemies[i].CurrentPosition.Y + EnemyHeight > Brick[j].TopLeft.Y && Enemies[i].CurrentPosition.Y < Brick[j].TopLeft.Y + BrickHeight)
                        {
                            if (Enemies[i].CurrentPosition.X + EnemyWidth > Brick[j].TopLeft.X && Enemies[i].LastPosition.X + EnemyWidth <= Brick[j].TopLeft.X)
                            {
                                Enemies[i].CurrentPosition.X = Brick[j].TopLeft.X - EnemyWidth;
                                Enemies[i].Velocity.X = -Enemies[i].Velocity.X;
                            }
                            if (Enemies[i].CurrentPosition.X < Brick[j].TopLeft.X + BrickWidth && Enemies[i].LastPosition.X >= Brick[j].TopLeft.X + BrickWidth)
                            {
                                Enemies[i].CurrentPosition.X = Brick[j].TopLeft.X + BrickWidth;
                                Enemies[i].Velocity.X = -Enemies[i].Velocity.X;
                            }
                        }
                    }
                }

                if (Enemies[i].Type == WORM)
                {
                    if (Enemies[i].StampedOn == 1)
                    {
                        if (!Enemies[i].YPosChanged)
                        {
                            Enemies[i].CurrentPosition.Y += EnemyHeight - EnemiesImagesInfo[Enemies[i].DyingImIdx].OriginalSize.Height;
                            Enemies[i].YPosChanged = true;
                        }
                        Enemies[i].ImgToShowIdx = Enemies[i].DyingImIdx;
                        if (Enemies[i].DyingTimeCounter > 0.7)
                        {
                            Enemies[i].IsAlive = false;
                        }
                    }

                    if (Enemies[i].StampedOn == 0)
                    {
                        //Collision With other enemies 
                        for (int j = 0; j < EnemiesNbr; j++)
                        {
                            if(j!=i)
                            if (Enemies[j].IsAlive && !Enemies[j].KilledByTurtle && !(Enemies[j].Type == WORM && Enemies[j].StampedOn == 1))
                            {
                                int SecEnemyHeight = EnemiesImagesInfo[Enemies[j].ImgToShowIdx].OriginalSize.Height;
                                if (Enemies[i].CurrentPosition.Y + EnemyHeight > Enemies[j].CurrentPosition.Y && Enemies[i].CurrentPosition.Y < Enemies[j].CurrentPosition.Y + SecEnemyHeight)
                                    if (Enemies[i].CurrentPosition.X + EnemyWidth > Enemies[j].CurrentPosition.X + 2 && Enemies[i].CurrentPosition.X < Enemies[j].CurrentPosition.X + EnemyWidth - 2)
                                    {
                                        if (Enemies[i].Velocity.X > 0)
                                            Enemies[i].CurrentPosition.X = Enemies[j].CurrentPosition.X - EnemyWidth + 2;
                                        else
                                            Enemies[i].CurrentPosition.X = Enemies[j].CurrentPosition.X + EnemyWidth - 2;
                                        Enemies[i].Velocity.X = -Enemies[i].Velocity.X;
                                    }
                                
                            }
                        }
                    }
                }

                if (Enemies[i].Type == TURTLE)
                {
                    if (Enemies[i].StampedOn != 0)
                    {
                        // dead enemies image are smaller than alive enemies, so Y pos adjustement is necessary.
                        Enemies[i].ImgToShowIdx = Enemies[i].DyingImIdx;
                        if (!Enemies[i].YPosChanged)
                        {
                            Enemies[i].CurrentPosition.Y = Enemies[i].CurrentPosition.Y + EnemiesImagesInfo[Enemies[i].Type].OriginalSize.Height - EnemiesImagesInfo[Enemies[i].DyingImIdx].OriginalSize.Height;
                            Enemies[i].YPosChanged = true;
                        }
                    }
                    //Collision With other enemies 
                    for (int j = 0; j < EnemiesNbr; j++)
                    {
                        if (j == i || !Enemies[j].IsAlive || Enemies[j].KilledByTurtle)
                            continue;

                        int SecEnemyHeight = EnemiesImagesInfo[Enemies[j].ImgToShowIdx].OriginalSize.Height;
                        if (Enemies[i].CurrentPosition.Y + EnemyHeight > Enemies[j].CurrentPosition.Y && Enemies[i].CurrentPosition.Y < Enemies[j].CurrentPosition.Y + SecEnemyHeight)
                            // next condition treats when i penetrates j, maybe make a symetric penetration 
                            if (Enemies[i].CurrentPosition.X + EnemyWidth > Enemies[j].CurrentPosition.X + 2 && Enemies[i].CurrentPosition.X < Enemies[j].CurrentPosition.X + EnemyWidth - 2)
                            {
                                if (Enemies[i].StampedOn == 2 && (Enemies[j].Type != TURTLE || Enemies[j].StampedOn == 0))
                                {
                                    Enemies[j].KilledByTurtle = true;
                                    ScoreStruct MyScore;
                                    MyScore.InitialPos = Enemies[j].CurrentPosition;
                                    MyScore.ScorePos = MyScore.InitialPos;
                                    MyScore.Value = 100;
                                    MyScore.Length = 3;
                                    Scores.push_back(MyScore);
                                    PlaySound(TEXT("../sounds/smb_kick.wav"), NULL, SND_FILENAME | SND_ASYNC);
                                    continue;
                                }
                                if (Enemies[j].StampedOn == 2 && Enemies[i].StampedOn == 0)
                                {
                                    Enemies[i].KilledByTurtle = true;
                                    ScoreStruct MyScore;
                                    MyScore.InitialPos = Enemies[i].CurrentPosition;
                                    MyScore.ScorePos = MyScore.InitialPos;
                                    MyScore.Value = 100;
                                    MyScore.Length = 3;
                                    Scores.push_back(MyScore);
                                    PlaySound(TEXT("../sounds/smb_kick.wav"), NULL, SND_FILENAME | SND_ASYNC);
                                    continue;
                                }
                                
                                if (Enemies[i].Velocity.X > 0)
                                    Enemies[i].CurrentPosition.X = Enemies[j].CurrentPosition.X - EnemyWidth + 2;
                                else
                                    Enemies[i].CurrentPosition.X = Enemies[j].CurrentPosition.X + EnemyWidth - 2;
                                Enemies[i].Velocity.X = -Enemies[i].Velocity.X;
                            }
                    }
                }
            }
        }
        if (Enemies[i].KilledByTurtle)
        {
            if (Enemies[i].Type == TURTLE)
                Enemies[i].ImgToShowIdx = Enemies[i].DyingImIdx;
            if (!Enemies[i].DyingVelSet)
            {
                Enemies[i].DyingVelSet = true;
                Enemies[i].Velocity.Y = -120;
            }
            Enemies[i].Velocity.X = 0;
        }
    }

}

internal void
TreatCollisions()
{
    int MarioHeight = MarioImagesInfo[Mario.ImageToShowIndx].OriginalSize.Height;
    int MarioWidth = MarioImagesInfo[Mario.ImageToShowIndx].OriginalSize.Width;
    int BrickHeight = 16;
    int BrickWidth = 16;

    // collision with bitmap left border
    if (Mario.CurrentPosition.X < BitmapXPosition)
    {
        Mario.CurrentPosition.X = BitmapXPosition;
        Mario.Velocity.X = 0;
    }
    // Collision with static obbjects
    for (int i = 0; i < StaticObjectsNbr; i++)
    {
        if(Brick[i].IsAlive && Brick[i].Type != POLEUP && Brick[i].Type != POLEDOWN)
        {
            if (Mario.CurrentPosition.X + MarioWidth - 2 > Brick[i].TopLeft.X && Mario.CurrentPosition.X < Brick[i].TopLeft.X + BrickWidth - 2)
            {
                if (Mario.CurrentPosition.Y + MarioHeight > Brick[i].TopLeft.Y && Mario.LastPosition.Y + MarioHeight <= Brick[i].TopLeft.Y)
                {
                    Mario.CurrentPosition.Y = Brick[i].TopLeft.Y - MarioHeight;
                    Mario.IsLimitedDown = true;
                    Mario.Velocity.Y = 0;
                }
            }

            if (Mario.CurrentPosition.Y + MarioHeight > Brick[i].TopLeft.Y && Mario.CurrentPosition.Y < Brick[i].TopLeft.Y + BrickHeight)
            {
                if (Mario.CurrentPosition.X + MarioWidth - 2 > Brick[i].TopLeft.X && Mario.LastPosition.X + MarioWidth - 2 <= Brick[i].TopLeft.X)
                {
                    Mario.CurrentPosition.X = Brick[i].TopLeft.X - MarioWidth + 2;
                    Mario.Velocity.X = 0;
                    Mario.StartedWalking = true;
                }
                if (Mario.CurrentPosition.X < Brick[i].TopLeft.X + BrickWidth - 2 && Mario.LastPosition.X >= Brick[i].TopLeft.X + BrickWidth - 2)
                {
                    Mario.CurrentPosition.X = Brick[i].TopLeft.X + BrickWidth - 2;
                    Mario.Velocity.X = 0;
                    Mario.StartedWalking = true;
                }
            }

            if (Mario.CurrentPosition.X + MarioWidth - 2  > Brick[i].TopLeft.X && Mario.CurrentPosition.X < Brick[i].TopLeft.X + BrickWidth - 2)
            {
                float BrickLastYPos = Brick[i].TopLeft.Y;
                float BrickCurrYPos = Brick[i].TopLeft.Y;
                std::vector<int>::iterator it = std::find(ShakyBricksIndices.begin(), ShakyBricksIndices.end(), i);
                if (it != ShakyBricksIndices.end())
                {
                    BrickLastYPos = BricksToShakeInfo[it - ShakyBricksIndices.begin()].LastPos.Y;
                    BrickCurrYPos = BricksToShakeInfo[it - ShakyBricksIndices.begin()].RealPos.Y;

                }
                if (Mario.CurrentPosition.Y <= BrickCurrYPos + BrickHeight && Mario.LastPosition.Y >= BrickLastYPos + BrickHeight)
                {
                    Mario.CurrentPosition.Y = BrickCurrYPos + BrickHeight;
                    Mario.Velocity.Y = 0;
                    if (Brick[i].HasMushroom)
                    {
                        Brick[i].HasMushroom = false;
                        PlaySound(TEXT("../sounds/smb_powerup_appears.wav"), NULL, SND_FILENAME | SND_ASYNC);

                        ShroomStruct ShroomInfo;
                        ShroomInfo.InitialPos = { float(Brick[i].TopLeft.X), float(Brick[i].TopLeft.Y) };
                        ShroomInfo.Position = ShroomInfo.InitialPos;
                        ShroomInfo.LastPosition = ShroomInfo.Position;
                        ShroomInfo.StartedMoving = true;
                        ShroomsInfo.push_back(ShroomInfo);
                        if (Brick[i].Type == QM || Brick[i].Type == QM2 || Brick[i].Type == QM3)
                            Brick[i].Type = QM4;
                    }
                    else if (Mario.CurrentPosition.X + 5 * MarioWidth / 6 > Brick[i].TopLeft.X && Mario.CurrentPosition.X + MarioWidth / 6 < Brick[i].TopLeft.X + BrickWidth)
                    {
                        if (Brick[i].Type == QM || Brick[i].Type == QM2 || Brick[i].Type == QM3)
                        {
                            Brick[i].Type = QM4;

                            CoinBrickMvtStruct NewCoin;
                            NewCoin.Vel = -180;
                            NewCoin.RealPos = { float(Brick[i].TopLeft.X), float(Brick[i].TopLeft.Y) };
                            NewCoin.InitialPos = Brick[i].TopLeft;
                            CoinsToLaunchInfo.push_back(NewCoin);
                         
                            CoinBrickMvtStruct ShakeInfo;
                            ShakeInfo.Vel = -70;
                            ShakeInfo.RealPos = { float(Brick[i].TopLeft.X), float(Brick[i].TopLeft.Y) };
                            ShakeInfo.InitialPos = Brick[i].TopLeft;
                            BricksToShakeInfo.push_back(ShakeInfo);
                            ShakyBricksIndices.push_back(i);

                            MarioScore+= 200;
                            CoinCounter += 1;
                            PlaySound(TEXT("../sounds/smb_coin.wav"), NULL, SND_FILENAME | SND_ASYNC);

                        }
                        if (Brick[i].Type == BRICK)
                        {
                            if (Mario.IsBig)
                            {
                                Brick[i].IsAlive = false;
                                BrickBreakingInfo Info;
                                Info.PartsPos[0] = { float(Brick[i].TopLeft.X), float(Brick[i].TopLeft.Y) };
                                Info.PartsPos[1] = { float(Brick[i].TopLeft.X + StaticObjectWidth / 2), float(Brick[i].TopLeft.Y) };
                                Info.PartsPos[2] = { float(Brick[i].TopLeft.X), float(Brick[i].TopLeft.Y + StaticObjectWidth / 2) };
                                Info.PartsPos[3] = { float(Brick[i].TopLeft.X + StaticObjectWidth / 2), float(Brick[i].TopLeft.Y + StaticObjectWidth / 2) };

                                Info.PartsVel[0] = { -20, -150 };
                                Info.PartsVel[1] = { 20, -150 };
                                Info.PartsVel[2] = { -20, -120 };
                                Info.PartsVel[3] = { 20, -120 };
                                BrokeBricksIndices.push_back(i);
                                BricksToBreakInfo.push_back(Info);

                                mciSendString("close breakblock", NULL, 0, NULL);
                                mciSendString("open \"../sounds/smb_breakblock.wav\" type waveaudio alias breakblock", NULL, 0, NULL);
                                mciSendString("play breakblock", NULL, 0, NULL);
                            }
                            else
                            {
                                std::vector<int>::iterator it = std::find(ShakyBricksIndices.begin(), ShakyBricksIndices.end(), i);
                                if (it != ShakyBricksIndices.end())
                                {
                                    int elet_index = it - ShakyBricksIndices.begin();
                                    BricksToShakeInfo[elet_index].Vel = -70;
                                    BricksToShakeInfo[elet_index].RealPos = { float(Brick[i].TopLeft.X), float(Brick[i].TopLeft.Y) };
                                }
                                else
                                {
                                    CoinBrickMvtStruct ShakeInfo;
                                    ShakeInfo.Vel = -70;
                                    ShakeInfo.RealPos = { float(Brick[i].TopLeft.X), float(Brick[i].TopLeft.Y) };
                                    ShakeInfo.InitialPos = Brick[i].TopLeft;
                                    BricksToShakeInfo.push_back(ShakeInfo);
                                    ShakyBricksIndices.push_back(i);
                                }
                                PlaySound(TEXT("../sounds/smb_bump.wav"), NULL, SND_FILENAME | SND_ASYNC);
                            }
                        }

                    }
                }
            }
        }
        if ((Brick[i].Type == POLEUP || Brick[i].Type == POLEDOWN) && !Mario.FinishedLevel)
        {
            if (Mario.CurrentPosition.Y + MarioHeight > Brick[i].TopLeft.Y && Mario.CurrentPosition.Y < Brick[i].TopLeft.Y + BrickHeight)
            {
                if (Mario.CurrentPosition.X + MarioWidth  > Brick[i].TopLeft.X + StaticObjectWidth / 2 + 2  && Mario.LastPosition.X + MarioWidth <= Brick[i].TopLeft.X + StaticObjectWidth / 2 + 2)
                {
                    Mario.FinishedLevel = true;
                    Mario.CurrentPosition.X = Brick[i].TopLeft.X + StaticObjectWidth / 2 - MarioWidth + 2;
                }
            }
        }
    }

    //Collision with enemies
    if(!Mario.IsShrinking)
    {
        for (int i = 0; i < EnemiesNbr; i++)
        {
            int EnemyWidth = 16;
            int EnemyHeight = EnemiesImagesInfo[Enemies[i].ImgToShowIdx].OriginalSize.Height;
            if (Enemies[i].IsAlive && !Enemies[i].KilledByTurtle && Enemies[i].StartMoving)
            {
                if (Mario.CurrentPosition.X + MarioWidth > Enemies[i].CurrentPosition.X && Mario.CurrentPosition.X < Enemies[i].CurrentPosition.X + EnemyWidth)
                {
                    if (Mario.CurrentPosition.Y + MarioHeight > Enemies[i].CurrentPosition.Y && Mario.LastPosition.Y + MarioHeight < Enemies[i].CurrentPosition.Y)
                    {
                        Mario.CurrentPosition.Y = Enemies[i].CurrentPosition.Y - MarioHeight;
                        MarioScore += 100;
                        if (Enemies[i].Type == TURTLE)
                        {
                            // Turtle.StampedOn == 0 : When turtle is out of shell (collision with enemies is normal)
                            // Turtle.StampedOn == 1 : When turtle is in shell but No mvt (collision with enemies is normal but with mario it starts moving)
                            // Turtle.StampedOn == 2 : When turtle is in shell but moving (collision kills enemies and mario except when he stamps on it)
                            Mario.Velocity.Y = -100;
                            if (Enemies[i].StampedOn == 1)
                            {
                                int TurtleVelPositive = (Mario.CurrentPosition.X + MarioWidth / 2 < Enemies[i].CurrentPosition.X + EnemyWidth / 2);
                                int VelSign = 1 * TurtleVelPositive + (-1) * !TurtleVelPositive;
                                Enemies[i].Velocity.X = VelSign * 100;
                                Enemies[i].StampedOn = 2;
                            }
                            else
                            {
                                if (Enemies[i].StampedOn == 0)
                                    PlaySound(TEXT("../sounds/smb_stomp.wav"), NULL, SND_FILENAME | SND_ASYNC);
                                Enemies[i].Velocity.X = 0;
                                Enemies[i].StampedOn = 1;
                                ScoreStruct MyScore;
                                MyScore.InitialPos = Enemies[i].CurrentPosition;
                                MyScore.ScorePos = MyScore.InitialPos;
                                MyScore.Value = 100;
                                MyScore.Length = 3;
                                Scores.push_back(MyScore);
                            }
                        }
                        if (Enemies[i].Type == WORM && Enemies[i].StampedOn == 0)
                        {
                            Mario.Velocity.Y = -100;
                            Enemies[i].StampedOn = 1;
                            Enemies[i].Velocity.X = 0;
                            Enemies[i].DyingTimeCounter = 0;

                            ScoreStruct MyScore;
                            MyScore.InitialPos = Enemies[i].CurrentPosition;
                            MyScore.ScorePos = MyScore.InitialPos;
                            MyScore.Value = 100;
                            MyScore.Length = 3;
                            Scores.push_back(MyScore);
                            PlaySound(TEXT("../sounds/smb_stomp.wav"), NULL, SND_FILENAME | SND_ASYNC);
                        }
                    }

                    if (Mario.CurrentPosition.Y < Enemies[i].CurrentPosition.Y + EnemyHeight && Mario.LastPosition.Y >= Enemies[i].CurrentPosition.Y + EnemyHeight)
                    {
                        if (Mario.IsBig)
                        {
                            Mario.IsShrinking = true;
                            Mario.AnimateShrinking = true;
                            PlaySound(TEXT("../sounds/smb_pipe.wav"), NULL, SND_FILENAME | SND_ASYNC);

                        }
                        else
                            Mario.IsDying = true;
                    }
                }
                if (Mario.CurrentPosition.Y + MarioHeight > Enemies[i].CurrentPosition.Y + 5 && Mario.CurrentPosition.Y < Enemies[i].CurrentPosition.Y + EnemyHeight - 5)
                {
                    if (Mario.CurrentPosition.X + MarioWidth > Enemies[i].CurrentPosition.X + 5 && Mario.CurrentPosition.X <= Enemies[i].CurrentPosition.X + EnemyWidth - 5)
                    {
                        if ((Enemies[i].Type != TURTLE || Enemies[i].StampedOn == 0 || Enemies[i].StampedOn == 2) && !(Enemies[i].Type == WORM && Enemies[i].StampedOn == 1))
                        {
                            if (Mario.IsBig)
                            {
                                Mario.IsShrinking = true;
                                Mario.AnimateShrinking = true;
                                PlaySound(TEXT("../sounds/smb_pipe.wav"), NULL, SND_FILENAME | SND_ASYNC);
                            }
                            else
                                Mario.IsDying = true;

                        }
                        if (Enemies[i].Type == TURTLE && Enemies[i].StampedOn == 1)
                        {
                            Mario.CurrentPosition.X = Mario.LastPosition.X;
                            int TurtleVelPositive = (Mario.CurrentPosition.X + MarioWidth / 2 < Enemies[i].CurrentPosition.X + EnemyWidth / 2);
                            int VelSign = 1 * TurtleVelPositive + (-1) * !TurtleVelPositive;
                            Enemies[i].Velocity.X = VelSign * 100;
                            Enemies[i].StampedOn = 2;
                        }

                    }
                }
            }
        }
    }
}

internal void
TreatCoinLaunching()
{   
    for(int i = 0; i < CoinsToLaunchInfo.size(); i++)
    {
        CoinsToLaunchInfo[i].Vel += Gravity * LoopTime;
        CoinsToLaunchInfo[i].RealPos.Y += CoinsToLaunchInfo[i].Vel * LoopTime;

        char ImToShow = 0;
        float AnimationDuration = 0.3;
        CoinsToLaunchInfo[i].ImgIndex += LoopTime * 4 / AnimationDuration;
        if (CoinsToLaunchInfo[i].ImgIndex >= 4)
            CoinsToLaunchInfo[i].ImgIndex = 0;

        switch (int(CoinsToLaunchInfo[i].ImgIndex))
        {
        case 0:
        {
            ImToShow = COIN1;
        }break;
        case 1:
        {
            ImToShow = COIN2;

        }break;
        case 2:
        {
            ImToShow = COIN3;

        }break;

        case 3:
        {
            ImToShow = COIN4;
        }break;
        default:
            break;
        }
        InsertImage(StaticObjectsImage, StaticObjectsImageWidth, StaticObjectsInfo[ImToShow], { int(CoinsToLaunchInfo[i].RealPos.X), int(CoinsToLaunchInfo[i].RealPos.Y) }, false, true);
        if (int(CoinsToLaunchInfo[i].RealPos.Y) + StaticObjectHeight < CoinsToLaunchInfo[i].InitialPos.Y - StaticObjectHeight)
            CoinsToLaunchInfo[i].HeightCheckOn = true;

        if (int(CoinsToLaunchInfo[i].RealPos.Y) + StaticObjectHeight > CoinsToLaunchInfo[i].InitialPos.Y - StaticObjectHeight && CoinsToLaunchInfo[i].HeightCheckOn)
        {
            ScoreStruct MyScore;
            MyScore.ScorePos.Y = CoinsToLaunchInfo[i].InitialPos.Y - 2 * StaticObjectHeight;
            MyScore.ScorePos.X = CoinsToLaunchInfo[i].InitialPos.X;
            MyScore.InitialPos = MyScore.ScorePos;
            MyScore.Length = 3;
            MyScore.Value = 200;
            Scores.push_back(MyScore);

            CoinsToLaunchInfo.erase(CoinsToLaunchInfo.begin() + i);
        }
    }
}

internal void
TreatShakyBreakyBricks() {
    for(int k =0 ; k < ShakyBricksIndices.size(); k++)
    {
        int i = ShakyBricksIndices[k];
        for (int j = 0; j < EnemiesNbr; j++)
        {
            if (Enemies[j].CurrentPosition.X < Brick[i].TopLeft.X + StaticObjectWidth && Enemies[j].CurrentPosition.X + StaticObjectWidth > Brick[i].TopLeft.X)
                if (int(Enemies[j].CurrentPosition.Y) + EnemiesImagesInfo[Enemies[j].ImgToShowIdx].OriginalSize.Height == Brick[i].TopLeft.Y)
                {
                    PlaySound(TEXT("../sounds/smb_kick.wav"), NULL, SND_FILENAME | SND_ASYNC);
                    Enemies[j].KilledByTurtle = true;
                    ScoreStruct MyScore;
                    MyScore.InitialPos = Enemies[j].CurrentPosition;
                    MyScore.ScorePos = MyScore.InitialPos;
                    MyScore.Value = 100;
                    MyScore.Length = 3;
                    Scores.push_back(MyScore);
                }

        }
        BricksToShakeInfo[k].LastPos.Y = BricksToShakeInfo[k].RealPos.Y;
        BricksToShakeInfo[k].Vel += Gravity * LoopTime;
        BricksToShakeInfo[k].RealPos.Y += BricksToShakeInfo[k].Vel * LoopTime;
        Brick[i].TopLeft.Y = int(BricksToShakeInfo[k].RealPos.Y);
        if (Brick[i].TopLeft.Y >= BricksToShakeInfo[k].InitialPos.Y)
        {
            Brick[i].TopLeft.Y = BricksToShakeInfo[k].InitialPos.Y;
            BricksToShakeInfo.erase(BricksToShakeInfo.begin() + k);
            ShakyBricksIndices.erase(ShakyBricksIndices.begin() + k);
        }
        float LastYPos = BricksToShakeInfo[k].LastPos.Y;

        for(int l = 0; l < ShroomsInfo.size(); l++)
        {
            if (ShroomsInfo[l].Position.X + 16 > Brick[i].TopLeft.X  && ShroomsInfo[l].Position.X <= Brick[i].TopLeft.X + 16)
            {
                if (BricksToShakeInfo[k].RealPos.Y < ShroomsInfo[l].Position.Y + 16 && LastYPos >= ShroomsInfo[l].Position.Y + 16)
                {
                    ShroomsInfo[l].Position.Y = Brick[i].TopLeft.Y - 16;
                    if (ShroomsInfo[l].Position.X < Brick[i].TopLeft.X + 8 && ShroomsInfo[l].Vel.X > 0)
                        ShroomsInfo[l].Vel.X = -ShroomsInfo[l].Vel.X;
                    if (ShroomsInfo[l].Position.X > Brick[i].TopLeft.X + 8 && ShroomsInfo[l].Vel.X < 0)
                        ShroomsInfo[l].Vel.X = -ShroomsInfo[l].Vel.X;
                }
            }
        }
    }

    for(int k = 0; k < BricksToBreakInfo.size(); k++)
    {
        int i = BrokeBricksIndices[k];
        
        for (int j = 0; j < EnemiesNbr; j++)
        {
            if (Enemies[j].CurrentPosition.X < Brick[i].TopLeft.X + StaticObjectWidth && Enemies[j].CurrentPosition.X + StaticObjectWidth > Brick[i].TopLeft.X)
                if (int(Enemies[j].CurrentPosition.Y) + StaticObjectWidth == Brick[i].TopLeft.Y)
                {
                    PlaySound(TEXT("../sounds/smb_kick.wav"), NULL, SND_FILENAME | SND_ASYNC);
                    Enemies[j].KilledByTurtle = true;
                    ScoreStruct MyScore;
                    MyScore.InitialPos = Enemies[j].CurrentPosition;
                    MyScore.ScorePos = MyScore.InitialPos;
                    MyScore.Value = 100;
                    MyScore.Length = 3;
                    Scores.push_back(MyScore);
                }
        }
        
        BricksToBreakInfo[k].PartsVel[0].Y += Gravity * float(LoopTime);
        BricksToBreakInfo[k].PartsVel[1].Y += Gravity * float(LoopTime);
        BricksToBreakInfo[k].PartsVel[2].Y += Gravity * float(LoopTime);
        BricksToBreakInfo[k].PartsVel[3].Y += Gravity * float(LoopTime);

        RealPoint LoopTimePoint = { float(LoopTime), float(LoopTime) };
        BricksToBreakInfo[k].PartsPos[0] = BricksToBreakInfo[k].PartsPos[0] + BricksToBreakInfo[k].PartsVel[0] * LoopTimePoint;
        BricksToBreakInfo[k].PartsPos[1] = BricksToBreakInfo[k].PartsPos[1] + BricksToBreakInfo[k].PartsVel[1] * LoopTimePoint;
        BricksToBreakInfo[k].PartsPos[2] = BricksToBreakInfo[k].PartsPos[2] + BricksToBreakInfo[k].PartsVel[2] * LoopTimePoint;
        BricksToBreakInfo[k].PartsPos[3] = BricksToBreakInfo[k].PartsPos[3] + BricksToBreakInfo[k].PartsVel[3] * LoopTimePoint;
        if (BricksToBreakInfo[k].PartsPos[0].Y > OriginalBitmapHeight)
        {
            BricksToBreakInfo.erase(BricksToBreakInfo.begin() + k);
            BrokeBricksIndices.erase(BrokeBricksIndices.begin() + k);
        }
    }

}

internal void
ShowScores()
{
    float ScoreVel = -25;
    for (auto i = 0; i < Scores.size(); i++)
    {
        Scores.at(i).ScorePos.Y += ScoreVel * LoopTime;
        for (int j = 0; j < Scores.at(i).Length ; j++)
        {
            int Digit = int(Scores.at(i).Value / pow(10, j)) - int(Scores.at(i).Value / pow(10, j + 1)) * 10;
            Point DigitPos = { int(Scores.at(i).ScorePos.X) + (Scores.at(i).Length - j - 1) * 10, int(Scores.at(i).ScorePos.Y) };
            InsertImage(NumLettersImage, NumLettersWidth, { {Digit, 2}, {9, 9} }, DigitPos, false, true);
        }
        if (Scores.at(i).ScorePos.X < BitmapXPosition + StaticObjectWidth)
            Scores.at(i).ScorePos.X = BitmapXPosition + StaticObjectWidth;
        if (Scores.at(i).InitialPos.Y - Scores.at(i).ScorePos.Y > 32)
            Scores.erase(Scores.begin() + i);
    }

}
internal void
InsertBreakingBricks()
{
    for(int j = 0; j < BricksToBreakInfo.size(); j++)
        for (int i = 0; i < 4; i++)
            InsertImage(StaticObjectsImage, StaticObjectsImageWidth, { StaticObjectsInfo[BRICK].ColumnAndRaw, {StaticObjectWidth / 2, StaticObjectWidth / 2} }, { int(BricksToBreakInfo[j].PartsPos[i].X), int(BricksToBreakInfo[j].PartsPos[i].Y) }, false, true);
        

}

internal void 
UpdateMarioXPos() {
    if (Mario.IsLimitedDown && Mario.ShouldWalk && !Mario.ShouldSlide)
    {
        local_persist float ImgIndex = 0;
        float AnimationDuration = -0.2 * Mario.Velocity.X/MarioMaxWalkingVelocity + 0.4;
        ImgIndex += LoopTime * 3 / AnimationDuration;
        if (ImgIndex >= 3)
            ImgIndex = 0;
        
        Mario.ImageToShowIndx = Mario.IsBig ? MOVING_BIG + ImgIndex : MOVING + ImgIndex;
    }
    Mario.Velocity.X = LoopTime * Mario.Acceleration.X + Mario.Velocity.X;
    if (Mario.WalkingRight&& Mario.Velocity.X > MarioMaxWalkingVelocity)
        Mario.Velocity.X = MarioMaxWalkingVelocity;
    if (!Mario.WalkingRight&& Mario.Velocity.X < -MarioMaxWalkingVelocity)
        Mario.Velocity.X = -MarioMaxWalkingVelocity;
    Mario.LastPosition.X    = Mario.CurrentPosition.X;
    Mario.CurrentPosition.X = LoopTime * Mario.Velocity.X  + Mario.LastPosition.X;
}

internal void 
UpdateMarioYPos(float JumpHeight = Mario.JumpHeight)
{
    Mario.Velocity.Y        = LoopTime*Gravity + Mario.Velocity.Y;
    Mario.LastPosition.Y    = Mario.CurrentPosition.Y;
    Mario.CurrentPosition.Y = LoopTime * Mario.Velocity.Y + Mario.LastPosition.Y;
    if(!Mario.IsLimitedDown && !Mario.IsDying)
        Mario.ImageToShowIndx = Mario.IsBig ? JUMPING_BIG : JUMPING;
}

internal void
AnimateShrooms() {
    int ShroomHeight = 16;
    int ShroomWidth = 16;

    for(int i = 0; i < ShroomsInfo.size(); i++)
    {
        if (ShroomsInfo[i].Rising)
        {
            ShroomsInfo[i].Vel.Y = -20;
            ShroomsInfo[i].LastPosition = ShroomsInfo[i].Position;
            ShroomsInfo[i].Position.Y = LoopTime * ShroomsInfo[i].Vel.Y + ShroomsInfo[i].LastPosition.Y;
            if (int(ShroomsInfo[i].Position.Y) + ShroomHeight == ShroomsInfo[i].InitialPos.Y)
            {
                ShroomsInfo[i].Rising = false;
                ShroomsInfo[i].Position.Y = ShroomsInfo[i].InitialPos.Y - ShroomHeight;
                ShroomsInfo[i].LastPosition = ShroomsInfo[i].Position;
                ShroomsInfo[i].Vel.Y = 0;
                ShroomsInfo[i].Vel.X = 45;
            }
        }
        else
        {
            int MarioHeight = MarioImagesInfo[Mario.ImageToShowIndx].OriginalSize.Height;
            int MarioWidth = MarioImagesInfo[Mario.ImageToShowIndx].OriginalSize.Width;

            ShroomsInfo[i].LastPosition = ShroomsInfo[i].Position;
            ShroomsInfo[i].Position.X = LoopTime * ShroomsInfo[i].Vel.X + ShroomsInfo[i].LastPosition.X;
            ShroomsInfo[i].Vel.Y = LoopTime * Gravity + ShroomsInfo[i].Vel.Y;
            ShroomsInfo[i].Position.Y = LoopTime * ShroomsInfo[i].Vel.Y + ShroomsInfo[i].LastPosition.Y;

            for (int j = 0; j < StaticObjectsNbr; j++)
            {
                // check if shroom is in Brick's width range and colliding with it vertically, here i treat : up to bottom
                if (ShroomsInfo[i].Position.X < Brick[j].TopLeft.X + ShroomWidth && ShroomsInfo[i].Position.X + ShroomWidth > Brick[j].TopLeft.X)
                {
                    if (ShroomsInfo[i].Position.Y + ShroomHeight > Brick[j].TopLeft.Y && ShroomsInfo[i].LastPosition.Y + ShroomHeight <= Brick[j].TopLeft.Y)
                    {
                        ShroomsInfo[i].Position.Y = Brick[j].TopLeft.Y - ShroomHeight;
                        ShroomsInfo[i].Vel.Y = 0;
                    }
                }

                // check that shroom is in Brick's height range 
                if (ShroomsInfo[i].Position.Y < Brick[j].TopLeft.Y + StaticObjectHeight && ShroomsInfo[i].Position.Y + ShroomHeight > Brick[j].TopLeft.Y)
                {
                    // change shroom's velocity sign when collinding horizontally with the Brick
                    if (ShroomsInfo[i].Position.X + ShroomWidth > Brick[j].TopLeft.X && ShroomsInfo[i].LastPosition.X + ShroomWidth <= Brick[j].TopLeft.X)
                    {
                        ShroomsInfo[i].Position.X = Brick[j].TopLeft.X - ShroomWidth;
                        ShroomsInfo[i].Vel.X = -ShroomsInfo[i].Vel.X;
                    }
                    if (ShroomsInfo[i].Position.X < Brick[j].TopLeft.X + StaticObjectWidth && ShroomsInfo[i].LastPosition.X >= Brick[j].TopLeft.X + StaticObjectWidth)
                    {
                        ShroomsInfo[i].Position.X = Brick[j].TopLeft.X + StaticObjectWidth;
                        ShroomsInfo[i].Vel.X = -ShroomsInfo[i].Vel.X;
                    }
                }
            }

            // Check if mario and the Shroom collide
            if (Mario.CurrentPosition.Y < ShroomsInfo[i].Position.Y + ShroomHeight && Mario.CurrentPosition.Y + MarioHeight > ShroomsInfo[i].Position.Y)
            {
                if (Mario.CurrentPosition.X + MarioWidth > ShroomsInfo[i].Position.X && Mario.CurrentPosition.X < ShroomsInfo[i].Position.X + ShroomWidth)
                {
                    if (!Mario.IsBig)
                    {
                        ShroomsInfo.erase(ShroomsInfo.begin() + i);
                        Mario.AnimateGrowing = true;
                        MarioScore += 1000;
                        PlaySound(TEXT("../sounds/smb_powerup.wav"), NULL, SND_FILENAME | SND_ASYNC);
                    }
                }
            }
        }

        Point ShroomPos = { int(ShroomsInfo[i].Position.X), int(ShroomsInfo[i].Position.Y) };
        InsertImage(MarioSpritesImage, MarioSpritesWidth, MarioImagesInfo[SHROOM], ShroomPos);
    }
}

internal void
HandleMarioLevelFinish()
{
    local_persist bool FlagDown = false;
    local_persist float FlagVelY = 100;
    if (!FinishLevelMusicStop)
    {
        mciSendString("close music", NULL, 0, NULL);
        PlaySound(TEXT("../sounds/smb_flagpole.wav"), NULL, SND_FILENAME | SND_ASYNC);
        FinishLevelMusicStop = true;
    }
    if(!FlagDown)
    {
        Mario.Velocity.X = 0;
        Mario.Velocity.Y = 100;
        UpdateMarioYPos();

        FlagY = FlagVelY * LoopTime + FlagY;
        
        if (((FlagY + 16 > Mario.CurrentPosition.Y) && !Mario.IsBig) || FlagY > Mario.CurrentPosition.Y)
            FlagDown = true;

        Mario.ImageToShowIndx = Mario.IsBig ? HANGING_BIG : HANGING;

    }
    else {
        local_persist bool PosChange = false;
        local_persist float InitPosX;
        if (!PosChange)
        {
            InitPosX = Mario.CurrentPosition.X;
            PlaySound(TEXT("../sounds/smb_stage_clear.wav"), NULL, SND_FILENAME | SND_ASYNC);
            PosChange = true;
            Mario.CurrentPosition.X += 6;
            Mario.Acceleration.X = 2 * MarioMaxWalkingVelocity / Mario.WalkingAccDuration;
            Mario.ShouldWalk = true;
        }

        if (Mario.CurrentPosition.X - InitPosX < 5 * StaticObjectWidth)
        {
            FinishLevelTimer = 0;
            UpdateMarioXPos();
            UpdateMarioYPos();
        }
        else
        {
            Mario.ImageToShowIndx = Mario.IsBig ? STANDING_BIG : STANDING;
            if (FinishLevelTimer > 5)
            {
                NewGame = false;
                FlagDown = false;
                PosChange = false;
            }

        }
    }
}


internal void 
HandleMarioGrowing()
{
        float GrowingDuration = 1;
        static float GrowingImgIndex = 0;
        GrowingImgIndex += LoopTime * 8 / GrowingDuration;
        local_persist int SizeChange = 0;
        switch(int(GrowingImgIndex)){
        case 0: case 2: case 4: case 5: case 7:
        {
            if (!Mario.IsBig)
                SizeChange = 1;
            Mario.IsBig = true;

        }break;
        case 1: case 3: case 6:
        {
            if (Mario.IsBig)
                SizeChange = -1;
            Mario.IsBig = false;
        }break;
        default:
            break;
        }
        if (GrowingImgIndex > 7)
        {
            Mario.AnimateGrowing = false;
            Mario.IsBig = true;
            GrowingImgIndex = 0;
        }

        if (SizeChange == 1)
        {
            Mario.CurrentPosition.Y -= MarioImagesInfo[STANDING_BIG].OriginalSize.Height - MarioImagesInfo[STANDING].OriginalSize.Height;
            SizeChange = 0;
        }
        if (SizeChange == -1)
        {
            Mario.CurrentPosition.Y += MarioImagesInfo[STANDING_BIG].OriginalSize.Height - MarioImagesInfo[STANDING].OriginalSize.Height;
            SizeChange = 0;
        }
        Mario.ImageToShowIndx = Mario.IsBig ? STANDING_BIG : STANDING;
}

internal void
HandleMarioShrinking()
{
    float ShrinkingDuration = 1;
    static float ShrinkingImgIndex = 0;
    ShrinkingImgIndex += LoopTime * float(8) / ShrinkingDuration;
    local_persist int SizeChange = 0;

    switch (int(ShrinkingImgIndex)) {
    case 0: case 2: case 4: case 5: case 7:
    {
        if (Mario.IsBig)
            SizeChange = -1;
        Mario.IsBig = false;
    }break;
    case 1: case 3: case 6:
    {
        if (!Mario.IsBig)
            SizeChange = 1;
        Mario.IsBig = true;
    }break;
    default:
        break;
    }
    if (ShrinkingImgIndex >= 8)
    {
        Mario.AnimateShrinking = false;
        ShrinkingImgIndex = 0;  
    }
    if (SizeChange == 1)
    {
        Mario.CurrentPosition.Y -= MarioImagesInfo[STANDING_BIG].OriginalSize.Height - MarioImagesInfo[STANDING].OriginalSize.Height;
        SizeChange = 0;
    }
    if (SizeChange == -1)
    {
        Mario.CurrentPosition.Y += MarioImagesInfo[STANDING_BIG].OriginalSize.Height - MarioImagesInfo[STANDING].OriginalSize.Height;
        SizeChange = 0;
    }
}


internal void
InitialiseGamePrams()
{
    BitmapXPosition = 0;

    char MyObjType = 0;
    int  ObjectIdentifier = 0;
    int  XPos = 0;
    int  YPos = 0;
    char ShroomsNbr = 0;
    Point ShroomsPositions[100];

    LPVOID  MyObjTypePtr = (void*)(&MyObjType);
    LPVOID  ObjectIdentifierPtr = (void*)(&ObjectIdentifier);
    LPVOID  XPosPtr = (void*)(&XPos);
    LPVOID  YPosPtr = (void*)(&YPos);
    LPVOID ShroomsNbrPtr = (void*)&ShroomsNbr;
    
    LPVOID EnemiesNbroPtr = (void*)&EnemiesNbr;
    LPVOID StatObjNbroPtr = (void*)&StaticObjectsNbr;
    LPVOID BgndNbroPtr = (void*)&BgndNbr;

    HANDLE ReadFileHnd = CreateFileA(MarioLevelFileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (ReadFileHnd != INVALID_HANDLE_VALUE) {

        DWORD BytesRead = 1;
        int FileSizeInBytes = GetFileSize(ReadFileHnd, NULL);
        SetFilePointer(ReadFileHnd, FileSizeInBytes - 12, NULL, 0);
        ReadFile(ReadFileHnd,
            EnemiesNbroPtr,
            sizeof(EnemiesNbr),
            &BytesRead,
            NULL);
        ReadFile(ReadFileHnd,
            StatObjNbroPtr,
            sizeof(StaticObjectsNbr),
            &BytesRead,
            NULL);
        ReadFile(ReadFileHnd,
            BgndNbroPtr,
            sizeof(BgndNbr),
            &BytesRead,
            NULL);
        Enemies = new Enemy[EnemiesNbr];
        Brick = new StaticObject[StaticObjectsNbr];
        Bgnd = new BgndObject[BgndNbr];
        SetFilePointer(ReadFileHnd, 0, NULL, 0);

        ReadFile(ReadFileHnd,
            ShroomsNbrPtr,
            sizeof(ShroomsNbr),
            &BytesRead,
            NULL);
        for (int i = 0; i < ShroomsNbr; i++)
        {
            LPVOID ShroomsPosPtr = (void*)(ShroomsPositions + i);
            ReadFile(ReadFileHnd,
                ShroomsPosPtr,
                sizeof(Point),
                &BytesRead,
                NULL);
        }
        int i = 0;
        int BricksIndex = 0;
        int EnemiesIndex = 0;
        int BgndIndex = 0;

        while (i < EnemiesNbr + StaticObjectsNbr + BgndNbr + 1)
        {
            ReadFile(ReadFileHnd,
                MyObjTypePtr,
                sizeof(MyObjType),
                &BytesRead,
                NULL);

            ReadFile(ReadFileHnd,
                ObjectIdentifierPtr,
                sizeof(ObjectIdentifier),
                &BytesRead,
                NULL);

            ReadFile(ReadFileHnd,
                XPosPtr,
                sizeof(XPos),
                &BytesRead,
                NULL);

            ReadFile(ReadFileHnd,
                YPosPtr,
                sizeof(YPos),
                &BytesRead,
                NULL);

            // TILE Case : here i have just the brick
            if (MyObjType == 0) {
                Brick[BricksIndex].TopLeft = { XPos, YPos };
                Brick[BricksIndex].Type = ObjectIdentifier;
                for (int j = 0; j < ShroomsNbr; j++)
                    if (ShroomsPositions[j].X == XPos && ShroomsPositions[j].Y == YPos)
                        Brick[BricksIndex].HasMushroom = true;
                if (ObjectIdentifier == POLEUP)
                {
                    FlagX = XPos;
                    FlagY = YPos;
                }
                BricksIndex++;
            }

            // EnemyCase
            if (MyObjType == 1)
            {
                Enemies[EnemiesIndex].Type = ObjectIdentifier;
                Enemies[EnemiesIndex].CurrentPosition = { float(XPos), float(YPos) };
                Enemies[EnemiesIndex].LastPosition = Enemies[EnemiesIndex].CurrentPosition;
                Enemies[EnemiesIndex].ImgToShowIdx = Enemies[EnemiesIndex].Type;

                if (ObjectIdentifier == TURTLE)
                    Enemies[EnemiesIndex].DyingImIdx = 5;
                else
                    Enemies[EnemiesIndex].DyingImIdx = 2;
                EnemiesIndex++;

            }
            // MArio
            if (MyObjType == 2)
            {
                Mario.CurrentPosition = { float(XPos), float(YPos) };
            }
            // BACKGROUND
            if (MyObjType == 3) {
                Bgnd[BgndIndex].TopLeft = { XPos, YPos };
                Bgnd[BgndIndex].Identifier = ObjectIdentifier;
                BgndIndex++;
            }
            i++;
        }

    }
    CloseHandle(ReadFileHnd);

    Mario.LastPosition = Mario.CurrentPosition;

    Mario.JumpVelocity = 8 * MarioOriginalHeight;
    Mario.JumpHeight = 4 * MarioOriginalHeight;  
    Mario.JumpDuration = 2 * Mario.JumpHeight / Mario.JumpVelocity;

    Mario.Acceleration.X = 0;
    Mario.FinishedLevel = false;
    Mario.ShouldWalk = false;
    Mario.IsBig = false;

    Mario.Velocity.X = 0;
    Mario.Velocity.Y = 0;

    Mario.ImageToShowIndx = STANDING;
    SetDeathVel = false;
    DeathMusic = false;
    BackgroundMusic = false;
    FinishLevelMusicStop = false;
    CoinCounter = 0;
    MarioScore = 0;
    LevelTimer = 401;

    Running = true;
}

internal void
HandleMarioDying()
{
    if(!DeathMusic)
    {
        DeathMusic = true;
        mciSendString("close music", NULL, 0, NULL);
        PlaySound(TEXT("../sounds/smb_mariodie.wav"), NULL, SND_FILENAME | SND_ASYNC);
    }
   
    Mario.ImageToShowIndx = DYING;
    if (!SetDeathVel)
    {
        SetDeathVel = true;
        Mario.Velocity.Y = -100;
    }
    UpdateMarioYPos();
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT* ClientRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(DeviceContext,
        /*
        X, Y, Width, Height,
        X, Y, Width, Height,
        */
        0, 0, BitmapWidth, BitmapHeight,
        0, 0, WindowWidth, WindowHeight,
        BitmapMemory,
        &BitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);
}

internal void
Win32ResizeDIBSection(int Width, int Height)
{
    if(BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;

    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

LRESULT CALLBACK
Win32GameWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch (Message)
    {
        case WM_KEYDOWN:
        {
            if (WParam == VK_LEFT)
                MyKeyboard.LeftArrowDown = true;
            if (WParam == VK_RIGHT)
                MyKeyboard.RightArrowDown = true;
            if (WParam == VK_UP)
                MyKeyboard.UpArrowDown = true;
            if (WParam == VK_DOWN)
                MyKeyboard.DownArrowDown = true;
            if (WParam == VK_RETURN)
                MyKeyboard.EnterDown = true;
            return 0;

        }break;
        case WM_KEYUP:
        {
            if (WParam == VK_LEFT)
                MyKeyboard.LeftArrowDown = false;
            if (WParam == VK_RIGHT)
                MyKeyboard.RightArrowDown = false;
            if (WParam == VK_UP)
                MyKeyboard.UpArrowDown = false;
            if (WParam == VK_DOWN)
                MyKeyboard.DownArrowDown = false;
            if (WParam == VK_RETURN)
                MyKeyboard.EnterDown = false;
        }break;

        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE \n");

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);

        } break;

        case WM_CLOSE:
        {
            Running = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY:
        {
            Running = false;
        } break;

        case WM_PAINT:
        {
            OutputDebugStringA("WM_PAINT\n");

            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);

            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;
        case MM_MCINOTIFY:
        {
            if(!DeathMusic && !FinishLevelMusicStop)
                BackgroundMusic = false;
        }break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }    
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    LoadMarioImages();
    QueryPerformanceFrequency(&PerformanceFrequencyL);
    int64 PerformanceFrequency = PerformanceFrequencyL.QuadPart;
    
    // Create the window class gor the game
    WNDCLASS WindowClass = {};
    WindowClass.lpfnWndProc = Win32GameWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "MarioWindowClass";

    if(RegisterClassA(&WindowClass))
    {
        HWND GameWindow =
            CreateWindowExA(
                0,
                WindowClass.lpszClassName,
                "Mario",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                1500,
                800,
                0,
                0,
                Instance,
                0);

        if (GameWindow)
        {
            // Initialising MarioImagesInfo
            MarioImagesInfo[STANDING]   = { {6, 1}, {MarioOriginalWidth, MarioOriginalWidth} };
            MarioImagesInfo[MOVING]     = { {0, 1}, {MarioOriginalWidth, MarioOriginalWidth} };
            MarioImagesInfo[MOVING + 1] = { {1, 1}, {MarioOriginalWidth, MarioOriginalWidth} };
            MarioImagesInfo[MOVING + 2] = { {2, 1}, {MarioOriginalWidth, MarioOriginalWidth} };
            MarioImagesInfo[SLIDING]    = { {3, 1}, {MarioOriginalWidth, MarioOriginalWidth} };
            MarioImagesInfo[JUMPING]    = { {4, 1}, {MarioOriginalWidth, MarioOriginalWidth} };
            MarioImagesInfo[HANGING]    = { {8, 1}, {MarioOriginalWidth, MarioOriginalWidth} };

            MarioImagesInfo[STANDING_BIG]   = { {6, 0}, {MarioOriginalWidth, 2 * MarioOriginalWidth} };
            MarioImagesInfo[MOVING_BIG]     = { {0, 0}, {MarioOriginalWidth, 2 * MarioOriginalWidth} };
            MarioImagesInfo[MOVING_BIG + 1] = { {1, 0}, {MarioOriginalWidth, 2 * MarioOriginalWidth} };
            MarioImagesInfo[MOVING_BIG + 2] = { {2, 0}, {MarioOriginalWidth, 2 * MarioOriginalWidth} };
            MarioImagesInfo[SLIDING_BIG]    = { {3, 0}, {MarioOriginalWidth, 2 * MarioOriginalWidth} };
            MarioImagesInfo[JUMPING_BIG]    = { {4, 0}, {MarioOriginalWidth, 2 * MarioOriginalWidth} };
            MarioImagesInfo[HANGING_BIG]    = { {8, 0}, {MarioOriginalWidth, 2*MarioOriginalWidth} };

            MarioImagesInfo[SQUATTING_BIG] = { {5, 0}, {MarioOriginalWidth, 2 * MarioOriginalWidth} };
            MarioImagesInfo[DYING]         = { {5, 1}, {MarioOriginalWidth, MarioOriginalWidth} };
            MarioImagesInfo[SHROOM]        = { {9, 0}, {StaticObjectWidth, StaticObjectHeight} };

            //Initialising Enemies Images info
            EnemiesImagesInfo[WORM]     = { {0, 0}, {StaticObjectWidth, StaticObjectHeight} };
            EnemiesImagesInfo[WORM + 1] = { {1, 0}, {StaticObjectWidth, StaticObjectHeight} };
            EnemiesImagesInfo[WORM + 2] = { {2, 0}, {StaticObjectWidth, StaticObjectHeight / 2} };

            EnemiesImagesInfo[TURTLE]     = { {6, 0}, {StaticObjectWidth, 23} };
            EnemiesImagesInfo[TURTLE + 1] = { {7, 0}, {StaticObjectWidth, 23} };
            EnemiesImagesInfo[TURTLE + 2] = { {10, 0}, {StaticObjectWidth, 14} };

            // Initialising StaticObjectsInfo
            StaticObjectsInfo[BRICK]   = { {1, 0}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[BRICK2]  = { {0, 0}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[BRICK3]  = { {0, 1}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[QM]      = { {24, 0}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[VTUBETL] = { {0, 2}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[VTUBETR] = { {1, 2}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[VTUBEBL] = { {0, 3}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[VTUBEBR] = { {1, 3}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[HTUBETL] = { {2, 2}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[HTUBETR] = { {3, 2}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[HTUBEBL] = { {2, 3}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[HTUBEBR] = { {3, 3}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[QM2]     = { {25, 0}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[QM3]     = { {26, 0}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[QM4]     = { {27, 0}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[COIN1]   = { {28, 0}, {StaticObjectWidth, 14} };
            StaticObjectsInfo[COIN2]   = { {29, 0}, {StaticObjectWidth, 14} };
            StaticObjectsInfo[COIN3]   = { {30, 0}, {StaticObjectWidth, 14} };
            StaticObjectsInfo[COIN4]   = { {31, 0}, {StaticObjectWidth, 14} };
            StaticObjectsInfo[FLAG] = { {25, 2}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[POLEUP] = { {26, 2}, {StaticObjectWidth, StaticObjectHeight} };
            StaticObjectsInfo[POLEDOWN] = { {26, 3}, {StaticObjectWidth, StaticObjectHeight} };

            Running = true;
            while (Running)
            {

                LARGE_INTEGER BeginTime, EndTime;
                QueryPerformanceCounter(&BeginTime);
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {   
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                HDC DeviceContext = GetDC(GameWindow);
                RECT ClientRect;
                GetClientRect(GameWindow, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;

                if (!NewGame)
                {
                    // RenderWelcomePage 
                    BitmapXPosition = 0;
                    if (MyKeyboard.DownArrowDown && !MyKeyboard.UpArrowDown && OnPlay)
                    {
                        PlaySound(TEXT("../sounds/smb_menu.wav"), NULL, SND_FILENAME | SND_ASYNC);
                        OnPlay = false;
                    }
                    if (MyKeyboard.UpArrowDown && !MyKeyboard.DownArrowDown && !OnPlay)
                    {
                        PlaySound(TEXT("../sounds/smb_menu.wav"), NULL, SND_FILENAME | SND_ASYNC);
                        OnPlay = true;
                    }

                    Point ImgPos = { 0,0 };
                    if (OnPlay)
                        ImgPos = { 159, 155 };
                    else
                        ImgPos = { 159, 175 };

                    InsertStretchImageToZone(WelcomePage, WelcomePageWidth, WelcomePageHeight, OriginalBitmap, OriginalBitmapWidth, OriginalBitmapHeight, true);
                    InsertImage(MarioSpritesImage, MarioSpritesWidth, MarioImagesInfo[STANDING], ImgPos);

                    //CheckNewGame
                    if (OnPlay && MyKeyboard.EnterDown)
                    {
                        NewGame = true;
                        InitialiseGamePrams();
                    }
                    if (!OnPlay && MyKeyboard.EnterDown)
                        PostQuitMessage(0);
                }
                else
                {
                    if(!BackgroundMusic)
                    {
                        BackgroundMusic = true;
                        mciSendString("close music", NULL, 0, NULL);
                        mciSendString("open \"../sounds/mario_bckgnd.wav\" type waveaudio alias music", NULL, 0, NULL);
                        mciSendString("play music notify", NULL, 0, GameWindow);
                    }

                    // Insert Default Bckgnd color
                    uint8 Red = 146;
                    uint8 Green = 144;
                    uint8 Blue = 255;
                    uint32 ZoneColor = ((Red << 16) | (Green << 8) | Blue);
                    InsertStretchImageToZone(&ZoneColor, 1, 1, OriginalBitmap, OriginalBitmapWidth, OriginalBitmapHeight);
                    
                    InsertBgndObjects();
                    InsertScoreAndTime();
                    AnimateShrooms(); 
                    TreatCoinLaunching();
                    InsertBricks();
                    //Insert flag
                    if (FlagX != -1)
                        InsertImage(StaticObjectsImage, StaticObjectsImageWidth, StaticObjectsInfo[FLAG], { FlagX - 8, int(FlagY + 16) }, false, true);
                    
                    InsertEnemies();

                    if (Mario.AnimateShrinking)
                        HandleMarioShrinking();
                    Mario.ImageToShowIndx = Mario.IsBig ? STANDING_BIG : STANDING;

                    if (!Mario.AnimateGrowing && !Mario.IsDying && !Mario.FinishedLevel && !Mario.AnimateShrinking)
                    {
                        if (MyKeyboard.DownArrowDown && Mario.IsBig && Mario.IsLimitedDown && !Mario.AnimateShrinking)
                        {
                            Mario.IsSquatting = true;
                            Mario.ImageToShowIndx = SQUATTING_BIG;
                            Mario.Acceleration.X = 0;
                            Mario.Velocity.X = 0;
                        }

                        if (!MyKeyboard.DownArrowDown)
                        {
                            Mario.IsSquatting = false;
                        }

                        if (!Mario.IsSquatting)
                        {
                            if (MyKeyboard.RightArrowDown && !MyKeyboard.LeftArrowDown)
                            {
                                Mario.WalkingRight= true;
                                Mario.ShouldWalk = true;
                            }
                            if (MyKeyboard.LeftArrowDown && !MyKeyboard.RightArrowDown)
                            {
                                Mario.WalkingRight= false;
                                Mario.ShouldWalk = true;
                            }
                            int MarioWalkingDirSign = Mario.WalkingRight* 1 + !Mario.WalkingRight* (-1);
                            if (Mario.ShouldWalk && Mario.StartedWalking)
                            {
                                Mario.Acceleration.X = 1.5 * MarioWalkingDirSign * MarioMaxWalkingVelocity / Mario.WalkingAccDuration;
                                Mario.StartedWalking = false;
                            }

                            if (!MyKeyboard.LeftArrowDown && !MyKeyboard.RightArrowDown)
                            {
                                Mario.ShouldWalk = false;
                                Mario.StartedWalking = true;
                                if (Mario.Velocity.X != 0)
                                {
                                    if ((Mario.WalkingRight&& Mario.Velocity.X > 0) || (!Mario.WalkingRight&& Mario.Velocity.X < 0))
                                        Mario.Acceleration.X = -3 * MarioWalkingDirSign * MarioMaxWalkingVelocity / Mario.WalkingAccDuration;
                                    else
                                    {
                                        Mario.Acceleration.X = 0;
                                        Mario.Velocity.X = 0;
                                    }
                                }
                            }

                            local_persist bool DirChange = false;
                            if ((Mario.WalkingRight&& Mario.Velocity.X < 0) || (!Mario.WalkingRight&& Mario.Velocity.X > 0))
                            {
                                DirChange = true;
                                Mario.Acceleration.X = 4 * MarioWalkingDirSign * MarioMaxWalkingVelocity / Mario.WalkingAccDuration;
                                if ((Mario.Velocity.X < -MarioMaxWalkingVelocity / 2 || Mario.Velocity.X > MarioMaxWalkingVelocity / 2))
                                    Mario.ShouldSlide = true;
                            }

                            if (Mario.IsLimitedDown && Mario.ShouldSlide)
                            {
                                Mario.ImageToShowIndx = Mario.IsBig ? SLIDING_BIG : SLIDING;
                            }
                            if (((Mario.WalkingRight&& Mario.Velocity.X >= 0) || (!Mario.WalkingRight&& Mario.Velocity.X <= 0)) && DirChange)
                            {
                                DirChange = false;
                                Mario.StartedWalking = true;
                                Mario.ShouldSlide = false;
                            }

                            if (MyKeyboard.UpArrowDown && Mario.IsLimitedDown)
                            {
                                Mario.IsLimitedDown = false;
                                Mario.Velocity.Y = -200;
                                if(Mario.IsBig)
                                    PlaySound(TEXT("../sounds/smb_jump_super.wav"), NULL, SND_FILENAME | SND_ASYNC);
                                else
                                    PlaySound(TEXT("../sounds/smb_jump_small.wav"), NULL, SND_FILENAME | SND_ASYNC);

                            }
                            UpdateMarioYPos();
                            UpdateMarioXPos();
                        }

                        // Change BitmapPos 
                        if (Mario.CurrentPosition.X - BitmapXPosition > float(OriginalBitmapWidth) / 2 && !Mario.FinishedLevel)
                        {
                            BitmapXPosition = Mario.CurrentPosition.X - float(OriginalBitmapWidth) / 2;
                        }
                        TreatCollisions();
                        //When Mario is Falling 
                        if (Mario.Velocity.Y > 0)
                            Mario.IsLimitedDown = false;
                    }

                    if (Mario.AnimateGrowing)
                        HandleMarioGrowing();

                    TreatEnemiesMvtAndColl();
                    TreatShakyBreakyBricks(); 
                    // Handle death by falling of the edge of level
                    if (Mario.CurrentPosition.Y + MarioImagesInfo[Mario.ImageToShowIndx].OriginalSize.Height > OriginalBitmapHeight && !Mario.IsDying)
                        Mario.IsDying = true;
                    if (Mario.CurrentPosition.Y - 50 * MarioImagesInfo[Mario.ImageToShowIndx].OriginalSize.Height > OriginalBitmapHeight && Mario.IsDying)
                    {
                        NewGame = false;
                        Mario.IsDying = false;
                        delete[] Enemies;
                        delete[] Brick;
                        delete[] Bgnd;
                        InitialiseGamePrams();
                    }
                    if (Mario.IsDying)
                        HandleMarioDying();
                    // Level finish handling
                    if (Mario.FinishedLevel)
                    {
                        HandleMarioLevelFinish();
                        TreatCollisions();

                    }

                    Point MarioIntPos = { int(Mario.CurrentPosition.X), int(Mario.CurrentPosition.Y) };

                    if (Mario.IsShrinking)
                    {
                        local_persist int counter = 0;
                        char msgbuf[200];
                        sprintf(msgbuf, "counter = %d, FluffTime = %f, LOOPTIME = %f \n", counter, ShrinkingTimeCounter, LoopTime);
                        counter++;
                        OutputDebugStringA(msgbuf);

                        bool InsertFlag = false;
                        if (int((ShrinkingTimeCounter - int(ShrinkingTimeCounter))*10) % 2 == 0)
                            InsertFlag = true;

                        if (InsertFlag)
                            InsertImage(MarioSpritesImage, MarioSpritesWidth, MarioImagesInfo[Mario.ImageToShowIndx], MarioIntPos, !Mario.WalkingRight);
                    }
                    else
                        InsertImage(MarioSpritesImage, MarioSpritesWidth, MarioImagesInfo[Mario.ImageToShowIndx], MarioIntPos, !Mario.WalkingRight);

                    InsertBreakingBricks();
                    ShowScores();
                }

                InsertStretchImageToZone(OriginalBitmap, OriginalBitmapWidth, OriginalBitmapHeight, (uint32*)BitmapMemory, BitmapWidth, BitmapHeight);

                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(GameWindow, DeviceContext);
                QueryPerformanceCounter(&EndTime);
                LoopTime = double(EndTime.QuadPart - BeginTime.QuadPart) / double(PerformanceFrequency);
                ShroomJumpTimeCounter += LoopTime;
                if(!Mario.FinishedLevel && !Mario.IsDying)
                    LevelTimer -= LoopTime;
                FinishLevelTimer += LoopTime;
                for (int i = 0; i < EnemiesNbr; i++)
                    if (Enemies[i].Type == WORM && Enemies[i].StampedOn)
                        Enemies[i].DyingTimeCounter += LoopTime;
                if (Mario.IsShrinking)
                    ShrinkingTimeCounter += LoopTime;
                if (ShrinkingTimeCounter > 5)
                {
                    Mario.IsShrinking = false;
                    ShrinkingTimeCounter = 0;
                }
            }
        }
        else
        {
        }
    }
    else
        {
        }
    
    return(0);
}


//
//DWORD errorCode = GetLastError();
//char errorMessage[256];
//sprintf(errorMessage, "Error code: %lu", errorCode);
//OutputDebugStringA(errorMessage);

                    //local_persist int counter = 0;
                    //char msgbuf[200];
                    //sprintf(msgbuf, "counter = %d, MouseCoords.X = %d, MouseCoords.Y = %d \n", counter, MouseCoords.X, MouseCoords.Y);
                    //counter++;
                    //OutputDebugStringA(msgbuf);
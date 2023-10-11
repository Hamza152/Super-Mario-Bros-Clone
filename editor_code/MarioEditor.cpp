#include <windows.h>
#include <stdint.h>
#include <windowsx.h>
#include <math.h>
#include <cstdio>
#include <iostream>
#include <fstream>

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

global_variable bool EditorRunning;
global_variable bool MarioInsertedInEditor = false;

// Displayed Bitmap declaration
global_variable BITMAPINFO EditorBitmapInfo;
global_variable float MainGridXPosition = 0;
global_variable bool EditorSaveButtonClicked = false;
global_variable int EditorEnemiesNbr = 0;
global_variable int EditorStaticObjectsNbr = 0;
global_variable int EditorBgndNbr = 0;

global_variable void* EditorBitmapMemory;

global_variable int EditorBitmapWidth = 2000;
global_variable int EditorBitmapHeight = 1080;
global_variable int BytesPerPixel = 4;

global_variable double LoopTime = 0;

global_variable uint32 Black = 0x00000000;
global_variable uint32 White = 0xffffffff;
global_variable uint32 Grey = 0x00b4b4b4;
global_variable uint8 Red = 99;
global_variable uint8 Green = 173;
global_variable uint8 Blue = 255;
global_variable uint32 ZoneColor = ((Red << 16) | (Green << 8) | Blue);

global_variable char MarioLevelFileName[] = "..\\levels\\MyLevel.txt";
global_variable char MarioSpritesFileName[] = "../sprites/mariobros.png";
global_variable uint32* MarioSpritesImage = NULL;
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

global_variable uint32* AllTheSprites[4];

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

global_variable int AllTheSpritesWidth[4];

global_variable int MarioOriginalWidth = 16;
global_variable int MarioOriginalHeight = 16;

global_variable int StaticObjectWidth = 16;
global_variable int StaticObjectHeight = 16;


global_variable int const SpritesWidth = 128;
global_variable int const DividingLineWidth = 2;

global_variable int const EditorOriginalBitmapWidth = 640 + SpritesWidth + DividingLineWidth;
global_variable int const EditorOriginalBitmapHeight = 480;
global_variable uint32 EditorOriginalBitmap[EditorOriginalBitmapWidth * EditorOriginalBitmapHeight];

global_variable float StretchWidthRatio = EditorBitmapWidth / EditorOriginalBitmapWidth;
global_variable float StretchHeightRatio = EditorBitmapHeight / EditorOriginalBitmapHeight;

global_variable bool SuppressionMode = false;

global_variable char ShroomsNbr = 0;

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
    SHROOM
};
enum {
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
    TILESNBR = FLAG + 3
};
enum{
    WORM = 0,
    TURTLE = 3
};

// for the sprites
enum {
    TILE,
    ENEMY,
    MARIO,
    BACKGROUND
};

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

Point MouseCoords = { 0,0 };
POINT MouseRealCoords;

Point SpriteSelectedItem;
Point GridSelectedItem = { -1, -1 };
Point mapping[TILESNBR];
Point ShroomsPositions[10];


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

global_variable const int MarioImagesNbr = 18;
global_variable const int StaticObjectsImgsNbr = 23;
global_variable int const EnemiesImgsNbr = 6;
global_variable int const BackgroundImgsNbr = 45;

global_variable ExtractAndDisplayInfo* AllImagesInfo[4];


struct SpriteEletInfo {
    bool IsEmpty = true; // to decide whether to treat it or not
    bool IsHighlighted = false;
    bool IsClickedOn = false;
    int Type; // background, tile, enemy, to know the source image where to get it 
    int Identifier; // position in source image
};
const int SpritesBlocksNbrV = EditorOriginalBitmapHeight / 16 - 2;
const int SpritesBlocksNbrH = SpritesWidth / 16;
SpriteEletInfo SpritesGrid[SpritesBlocksNbrH][SpritesBlocksNbrV];

// Putting Images into the main grid
struct MainGridElet {
    bool IsEmpty = true; // if true we put the default background color 
    bool IsPaired = false; // To check if it is shared with below square/ below square is always the one containing the obj.
    bool IsClickedOn = false;
    Point SpriteElet = { 0, 0 };
};
MainGridElet MainGridInfo[600][30];

internal void 
InsertHorizontalLine(uint32 Color, int Line, Point Limits, int Thickness)
{
    for (int i = Line; i < Line + Thickness; i++)
        for (int j = Limits.X; j < Limits.Y; j++)
            EditorOriginalBitmap[i * EditorOriginalBitmapWidth + j] = Color;
}

internal void
InsertVerticalLine(uint32 Color, int Column, Point Limits, int Thickness)
{
    for (int j = Limits.X; j <= Limits.Y; j++)
        for (int i = Column; i < Column + Thickness; i++)
            EditorOriginalBitmap[j * EditorOriginalBitmapWidth + i] = Color;
}


// Function to insert color or a total image into a zone on destination and it checks transparency when source is a png
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
            if (FromPng)
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
LoadEditorImages() {
    int n;
    AllTheSprites[MARIO] =    (uint32*)stbi_load(MarioSpritesFileName, &MarioSpritesWidth, &MarioSpritesHeight, &n, 0);
    AllTheSprites[TILE]  =   (uint32*)stbi_load(StaticObjectsFileName, &StaticObjectsImageWidth, &StaticObjectsImageHeight, &n, 0);
    AllTheSprites[ENEMY] =         (uint32*)stbi_load(EnemiesFileName, &EnemiesImageWidth, &EnemiesImageHeight, &n, 0);
    SaveButtonImage =      (uint32*)stbi_load(SaveButtonFileName, &SaveButtonImageWidth, &SaveButtonImageHeight, &n, 0);
    AllTheSprites[BACKGROUND] =     (uint32*)stbi_load(BackgroundsFileName, &BackgroundsImageWidth, &BackgroundsImageHeight, &n, 0);
    
}

internal void
EditorInsertImage(uint32* OriginalSprite, int SpriteWidth, ExtractAndDisplayInfo ImgInfo, Point ImgPos, bool IsStaticObject = false, bool Highlight = false)
{
    Point SrcImgPos;
    if (!IsStaticObject)
        SrcImgPos = { ImgInfo.ColumnAndRaw.X * MarioOriginalWidth, ImgInfo.ColumnAndRaw.Y * 2 * MarioOriginalHeight };
    else
        SrcImgPos = { ImgInfo.ColumnAndRaw.X * StaticObjectWidth, ImgInfo.ColumnAndRaw.Y * StaticObjectHeight };
    if(IsInDisplayRange(ImgPos, ImgInfo.OriginalSize.Width, ImgInfo.OriginalSize.Height, 0, EditorOriginalBitmapWidth, EditorOriginalBitmapHeight))
    {
        for (int Y = ImgPos.Y; Y < ImgPos.Y + ImgInfo.OriginalSize.Height; Y++)
        {
            if (Y < 0)
                Y = 0;
            if (Y == EditorOriginalBitmapHeight)
                break;
            for (int X = ImgPos.X; X < ImgPos.X + ImgInfo.OriginalSize.Width; X++)
            {
                if (X == EditorOriginalBitmapWidth)
                    break;
                if (X < 0)
                    X = 0;
                int Ysrc = Y - ImgPos.Y;
                int Xsrc = X - ImgPos.X;

                if (Highlight)
                {
                    if (Y == ImgPos.Y || Y == ImgPos.Y + ImgInfo.OriginalSize.Height - 1)
                    {
                        EditorOriginalBitmap[Y * EditorOriginalBitmapWidth + X] = White;
                        continue;
                    }
                    if (X == ImgPos.X || X == ImgPos.X + ImgInfo.OriginalSize.Width - 1)
                    {
                        EditorOriginalBitmap[Y * EditorOriginalBitmapWidth + X] = White;
                        continue;
                    }
                }

                uint32 TransparencyMask = 0xff000000;
                uint32 Pixel = OriginalSprite[(Ysrc + SrcImgPos.Y) * SpriteWidth + Xsrc + SrcImgPos.X];
                if (Pixel & TransparencyMask)
                {
                    uint32 Red = Pixel & 0x000000ff;
                    uint32 Green = Pixel & 0x0000ff00;
                    uint32 Blue = Pixel & 0x00ff0000;
                    EditorOriginalBitmap[Y * EditorOriginalBitmapWidth + X] = (Red << 16) | Green | (Blue >> 16);
                }

            }
        }
    }
}

internal void
EditorMainGridInsertImage(uint32* OriginalSprite, int SpriteWidth, ExtractAndDisplayInfo ImgInfo, Point ImgPos, bool IsStaticObject = false, bool Highlight = false)
{
    Point SrcImgPos;
    if (!IsStaticObject)
        SrcImgPos = { ImgInfo.ColumnAndRaw.X * MarioOriginalWidth, ImgInfo.ColumnAndRaw.Y * 2 * MarioOriginalHeight };
    else
        SrcImgPos = { ImgInfo.ColumnAndRaw.X * StaticObjectWidth, ImgInfo.ColumnAndRaw.Y * StaticObjectHeight };

    if (ImgPos.X + ImgInfo.OriginalSize.Width - int(MainGridXPosition) > 0 && ImgPos.X < int(MainGridXPosition) + EditorOriginalBitmapWidth - (SpritesWidth + DividingLineWidth))
    {
        if (ImgPos.Y + ImgInfo.OriginalSize.Height > 0 && ImgPos.Y < EditorOriginalBitmapHeight)
        {
            for (int Y = ImgPos.Y; Y < ImgPos.Y + ImgInfo.OriginalSize.Height; Y++)
            {
                for (int X = ImgPos.X - int(MainGridXPosition) + (SpritesWidth + DividingLineWidth); X < ImgPos.X - int(MainGridXPosition) + (SpritesWidth + DividingLineWidth) + ImgInfo.OriginalSize.Width; X++)
                {
                    if (X == EditorOriginalBitmapWidth)
                        break;
                    if (X < SpritesWidth + DividingLineWidth)
                        X = SpritesWidth + DividingLineWidth;

                    int Ysrc = Y - ImgPos.Y;
                    int Xsrc = X - (ImgPos.X - int(MainGridXPosition) + (SpritesWidth + DividingLineWidth));
                    uint32 TransparencyMask = 0xff000000;
                    uint32 Pixel = OriginalSprite[(Ysrc + SrcImgPos.Y) * SpriteWidth + Xsrc + SrcImgPos.X];


                    if (Pixel & TransparencyMask)
                    {
                        uint32 Red = Pixel & 0x000000ff;
                        uint32 Green = Pixel & 0x0000ff00;
                        uint32 Blue = Pixel & 0x00ff0000;
                        EditorOriginalBitmap[Y * EditorOriginalBitmapWidth + X] = (Red << 16) | Green | (Blue >> 16);
                    }

                }
            }
        }
    }
}

internal void 
HandleHilightAndClick(int i , int j, int ImgWidth, int ImgHeight)
{
    if (MouseRealCoords.x > i * 16 * StretchWidthRatio && MouseRealCoords.x <= i * 16 * StretchWidthRatio + ImgWidth &&
        MouseRealCoords.y > j * 16 * StretchHeightRatio && MouseRealCoords.y < j * 16 * StretchHeightRatio + ImgHeight)
        SpritesGrid[i][j].IsHighlighted = true;
    if (SpritesGrid[i][j].IsHighlighted && MyMouseState.LeftButtonDown && !MyMouseState.LeftButtonHeld)
        SpritesGrid[i][j].IsClickedOn = true;
    if (!MyMouseState.LeftButtonHeld && !MyMouseState.LeftButtonDown)
    {
        SpritesGrid[i][j].IsClickedOn = false;
        SpriteSelectedItem = { -1, -1 };
    }
}

internal void 
HandleImgAndInsert(int i, int j, int ImgWidth, int ImgHeight)
{

    SpriteEletInfo& SpriteElet = SpritesGrid[i][j];
    Point ImgPos = { i * 16, j * 16 };
    SpriteElet.IsHighlighted = false;
    HandleHilightAndClick(i, j, ImgWidth, ImgHeight);
    int OriginalYIndex;
    int OriginalXIndex;
    switch (SpriteElet.Type)
    {
        case BACKGROUND:
            OriginalYIndex = int(float(SpriteElet.Identifier * 16) / BackgroundsImageWidth);
            OriginalXIndex = ((SpriteElet.Identifier * 16) % BackgroundsImageWidth) / 16;
            EditorInsertImage(AllTheSprites[BACKGROUND], BackgroundsImageWidth, { {OriginalXIndex, OriginalYIndex}, {StaticObjectWidth, StaticObjectHeight} }, ImgPos, true, (SpriteElet.IsHighlighted && !SpriteElet.IsClickedOn));
            break;
        case ENEMY: case TILE: case MARIO:
            EditorInsertImage(AllTheSprites[SpriteElet.Type], AllTheSpritesWidth[SpriteElet.Type], AllImagesInfo[SpriteElet.Type][SpriteElet.Identifier], ImgPos, (SpriteElet.Type == TILE), (SpriteElet.IsHighlighted && !SpriteElet.IsClickedOn));
            break;
    }
    if (SpriteElet.IsClickedOn)
    {
        Point MouseCentredPos = { int((MouseRealCoords.x - ImgWidth / 2) / StretchWidthRatio) ,int((MouseRealCoords.y - ImgHeight / 2) / StretchHeightRatio) };

        SpriteSelectedItem = { i, j };
        switch (SpriteElet.Type)
        {
            case BACKGROUND:
                EditorInsertImage(AllTheSprites[BACKGROUND], BackgroundsImageWidth, { {OriginalXIndex, OriginalYIndex}, {StaticObjectWidth, StaticObjectHeight} }, MouseCentredPos, true, SpriteElet.IsHighlighted);
                break;
            case TILE: case ENEMY:
                EditorInsertImage(AllTheSprites[SpriteElet.Type], AllTheSpritesWidth[SpriteElet.Type], AllImagesInfo[SpriteElet.Type][SpriteElet.Identifier], MouseCentredPos, (SpriteElet.Type == TILE), SpriteElet.IsHighlighted);

        }
        if (SpriteElet.Type == MARIO && !MarioInsertedInEditor)
            EditorInsertImage(AllTheSprites[SpriteElet.Type], AllTheSpritesWidth[SpriteElet.Type], AllImagesInfo[SpriteElet.Type][SpriteElet.Identifier], MouseCentredPos, (SpriteElet.Type == TILE), SpriteElet.IsHighlighted);
    }
}

internal bool 
IsMouseInMainGrid()
{
    if (MouseRealCoords.x / StretchWidthRatio - (SpritesWidth + DividingLineWidth) >= 0 && MouseRealCoords.y / StretchHeightRatio >= 0)
        return true;
    return false;
}


internal void
LoadMainGridFromFIle()
{
    char MyObjType = 0;
    int  ObjectIdentifier = 0;
    int  XPos = 0;
    int  YPos = 0;
    int StatObjNbro = 0;
    int EnemiesNbro = 0;
    int BgndNbro = 0;

    LPVOID  MyObjTypePtr = (void*)(&MyObjType);
    LPVOID  ObjectIdentifierPtr = (void*)(&ObjectIdentifier);
    LPVOID  XPosPtr = (void*)(&XPos);
    LPVOID  YPosPtr = (void*)(&YPos);
    LPVOID ShroomsNbrPtr = (void*)&ShroomsNbr;
    LPVOID EnemiesNbroPtr = (void*)&EnemiesNbro;
    LPVOID StatObjNbroPtr = (void*)&StatObjNbro;
    LPVOID BgndNbroPtr = (void*)&BgndNbro;


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
            sizeof(EnemiesNbro),
            &BytesRead,
            NULL);
        ReadFile(ReadFileHnd,
            StatObjNbroPtr,
            sizeof(StatObjNbro),
            &BytesRead,
            NULL);
        ReadFile(ReadFileHnd,
            BgndNbroPtr,
            sizeof(BgndNbro),
            &BytesRead,
            NULL);
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

        while (i < EnemiesNbro + StatObjNbro + BgndNbro + 1)
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

            int k = XPos / 16;
            int l = (YPos % 16 == 0) ? YPos / 16 : YPos / 16 + 1;
            MainGridInfo[k][l].IsEmpty = false;
            if (MyObjType == 2)
                MainGridInfo[k][l].SpriteElet = { 0, SpritesBlocksNbrV - 1 };
            else
                MainGridInfo[k][l].SpriteElet = mapping[ObjectIdentifier];
            i++;
        }

    }
    CloseHandle(ReadFileHnd);
}

internal void
Initialise()
{
    AllImagesInfo[MARIO] = new ExtractAndDisplayInfo[MarioImagesNbr];
    AllImagesInfo[TILE] = new ExtractAndDisplayInfo[StaticObjectsImgsNbr];
    AllImagesInfo[ENEMY] = new ExtractAndDisplayInfo[EnemiesImgsNbr];

    for (int i = 0; i < 4; i++)
        AllTheSprites[i] = NULL;

    LoadEditorImages();

    AllTheSpritesWidth[TILE] = StaticObjectsImageWidth;
    AllTheSpritesWidth[MARIO] = MarioSpritesWidth;
    AllTheSpritesWidth[ENEMY] = EnemiesImageWidth;
    AllTheSpritesWidth[BACKGROUND] = BackgroundsImageWidth;

    // Mario Image in there
    AllImagesInfo[MARIO][STANDING] = { {6, 1}, {MarioOriginalWidth, MarioOriginalWidth} };

    //Initialising Enemies Images info
    AllImagesInfo[ENEMY][WORM] = { {0, 0}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[ENEMY][WORM + 1] = { {1, 0}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[ENEMY][WORM + 2] = { {2, 0}, {StaticObjectWidth, StaticObjectHeight / 2} };

    AllImagesInfo[ENEMY][TURTLE] = { {6, 0}, {StaticObjectWidth, 23} };
    AllImagesInfo[ENEMY][TURTLE + 1] = { {7, 0}, {StaticObjectWidth, 23} };
    AllImagesInfo[ENEMY][TURTLE + 2] = { {10, 0}, {StaticObjectWidth, 14} };

    // Initialising StaticObjectsInfo
    AllImagesInfo[TILE][BRICK] = { {1, 0}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][BRICK2] = { {0, 0}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][BRICK3] = { {0, 1}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][QM] = { {24, 0}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][VTUBETL] = { {0, 2}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][VTUBETR] = { {1, 2}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][VTUBEBL] = { {0, 3}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][VTUBEBR] = { {1, 3}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][HTUBETL] = { {2, 2}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][HTUBETR] = { {3, 2}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][HTUBEBL] = { {2, 3}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][HTUBEBR] = { {3, 3}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][SHROOM] = { {32, 0}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][POLEUP] = { {26, 2}, {StaticObjectWidth, StaticObjectHeight} };
    AllImagesInfo[TILE][POLEDOWN] = { {26, 3}, {StaticObjectWidth, StaticObjectHeight} };

    // Initialising background images
    for (int i = 0; i < BackgroundImgsNbr; i++)
    {
        int EditorYPos = i / 8;
        int EditorXPos = i % 8;
        SpritesGrid[EditorXPos][EditorYPos].Type = BACKGROUND;
        SpritesGrid[EditorXPos][EditorYPos].Identifier = i;
        SpritesGrid[EditorXPos][EditorYPos].IsEmpty = false;

    }

    SpritesGrid[1][10].Type = TILE;
    SpritesGrid[1][10].Identifier = POLEUP;
    SpritesGrid[1][10].IsEmpty = false;
    mapping[POLEUP] = { 1,10 };

    SpritesGrid[2][10].Type = TILE;
    SpritesGrid[2][10].Identifier = POLEDOWN;
    SpritesGrid[2][10].IsEmpty = false;
    mapping[POLEDOWN] = { 2,10 };

    SpritesGrid[0][10].Type = TILE;
    SpritesGrid[0][10].Identifier = SHROOM;
    SpritesGrid[0][10].IsEmpty = false;
    mapping[SHROOM] = { 0,10 };

    SpritesGrid[0][9].Type = TILE;
    SpritesGrid[0][9].Identifier = BRICK;
    SpritesGrid[0][9].IsEmpty = false;
    mapping[BRICK] = { 0,9 };

    SpritesGrid[1][9].Type = TILE;
    SpritesGrid[1][9].Identifier = BRICK2;
    SpritesGrid[1][9].IsEmpty = false;
    mapping[BRICK2] = { 1,9 };

    SpritesGrid[2][9].Type = TILE;
    SpritesGrid[2][9].Identifier = BRICK3;
    SpritesGrid[2][9].IsEmpty = false;
    mapping[BRICK3] = { 2,9 };

    SpritesGrid[3][9].Type = TILE;
    SpritesGrid[3][9].Identifier = QM;
    SpritesGrid[3][9].IsEmpty = false;
    mapping[QM] = { 3,9 };

    SpritesGrid[4][9].Type = TILE;
    SpritesGrid[4][9].Identifier = VTUBETL;
    SpritesGrid[4][9].IsEmpty = false;
    mapping[VTUBETL] = { 4,9 };

    SpritesGrid[5][9].Type = TILE;
    SpritesGrid[5][9].Identifier = VTUBETR;
    SpritesGrid[5][9].IsEmpty = false;
    mapping[VTUBETR] = { 5,9 };

    SpritesGrid[4][10].Type = TILE;
    SpritesGrid[4][10].Identifier = VTUBEBL;
    SpritesGrid[4][10].IsEmpty = false;
    mapping[VTUBEBL] = { 4,10 };

    SpritesGrid[5][10].Type = TILE;
    SpritesGrid[5][10].Identifier = VTUBEBR;
    SpritesGrid[5][10].IsEmpty = false;
    mapping[VTUBEBR] = { 5,10 };

    SpritesGrid[6][9].Type = TILE;
    SpritesGrid[6][9].Identifier = HTUBETL;
    SpritesGrid[6][9].IsEmpty = false;
    mapping[HTUBETL] = { 6,9 };

    SpritesGrid[7][9].Type = TILE;
    SpritesGrid[7][9].Identifier = HTUBETR;
    SpritesGrid[7][9].IsEmpty = false;
    mapping[HTUBETR] = { 7,9 };

    SpritesGrid[6][10].Type = TILE;
    SpritesGrid[6][10].Identifier = HTUBEBL;
    SpritesGrid[6][10].IsEmpty = false;
    mapping[HTUBEBL] = { 6,10 };

    SpritesGrid[7][10].Type = TILE;
    SpritesGrid[7][10].Identifier = HTUBEBR;
    SpritesGrid[7][10].IsEmpty = false;
    mapping[HTUBEBR] = { 7,10 };

    SpritesGrid[0][14].Type = ENEMY;
    SpritesGrid[0][14].Identifier = WORM;
    SpritesGrid[0][14].IsEmpty = false;
    mapping[WORM] = { 0,14 };

    SpritesGrid[1][14].Type = ENEMY;
    SpritesGrid[1][14].Identifier = TURTLE;
    SpritesGrid[1][14].IsEmpty = false;
    mapping[TURTLE] = { 1,14 };

    SpritesGrid[0][SpritesBlocksNbrV - 1].Type = MARIO;
    SpritesGrid[0][SpritesBlocksNbrV - 1].Identifier = STANDING;
    SpritesGrid[0][SpritesBlocksNbrV - 1].IsEmpty = false;
}

internal void 
GetUserInput()
{
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        if (Message.message == WM_QUIT)
        {
            EditorRunning = false;
        }

        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }

}

internal void GetMouseCoords(HWND EditorWindow)
{
    if (GetCursorPos(&MouseRealCoords))
        if (ScreenToClient(EditorWindow, &MouseRealCoords))
            MouseCoords = { int(MouseRealCoords.x / StretchWidthRatio) / 16,int(MouseRealCoords.y / StretchHeightRatio) / 16 };
        else
            OutputDebugStringA("Error while getting mouse coor rel to client area\n");
    else
        OutputDebugStringA("Error while getting mouse coor rel to screen\n");
}

internal void 
InsertBackground()
{
    // Insert background colors
    InsertHorizontalLine(ZoneColor, 0, { SpritesWidth, EditorOriginalBitmapWidth }, EditorOriginalBitmapHeight);
    InsertHorizontalLine(Black, EditorOriginalBitmapHeight / 2, { SpritesWidth, EditorOriginalBitmapWidth }, EditorOriginalBitmapHeight / 2);
    InsertHorizontalLine(Black, 0, { 0, SpritesWidth }, EditorOriginalBitmapHeight);

    // Insert SpacesDividingLine 
    InsertVerticalLine(Black, SpritesWidth, { 0, EditorOriginalBitmapHeight }, DividingLineWidth);

    // Insert Main Grid
    for (int Y = 0; Y < EditorOriginalBitmapHeight; Y += 16)
    {
        uint32 Color = Y < EditorOriginalBitmapHeight / 2 ? Black : Grey;
        InsertHorizontalLine(Color, Y, { SpritesWidth + DividingLineWidth, EditorOriginalBitmapWidth }, 1);
    }

    for (int X = SpritesWidth + DividingLineWidth + 16 * (int(MainGridXPosition) / 16 + 1) - int(MainGridXPosition); X < EditorOriginalBitmapWidth; X += 16)
        InsertVerticalLine(Black, X, { 0, EditorOriginalBitmapHeight }, 1);

    //Insert SAve Button
    InsertStretchImageToZone(SaveButtonImage, SaveButtonImageWidth, SaveButtonImageHeight, EditorOriginalBitmap, EditorOriginalBitmapWidth, EditorOriginalBitmapHeight, true, { 0, SpritesBlocksNbrV * 16 }, { SpritesBlocksNbrH * 16, SpritesBlocksNbrV * 16 + 32 });
}

internal void 
InsertSpriteElets()
{
    for (int i = 0; i < SpritesBlocksNbrH; i++)
        for (int j = 0; j < SpritesBlocksNbrV; j++)
            if (!SpritesGrid[i][j].IsEmpty)
            {
                int ImgHeight;
                int ImgWidth;
                switch (SpritesGrid[i][j].Type)
                {
                case TILE:
                    ImgHeight = StaticObjectWidth * StretchHeightRatio;
                    ImgWidth = StaticObjectHeight * StretchWidthRatio;
                    break;
                case ENEMY:
                    ImgHeight = AllImagesInfo[ENEMY][SpritesGrid[i][j].Identifier].OriginalSize.Height * StretchHeightRatio;;
                    ImgWidth = AllImagesInfo[ENEMY][SpritesGrid[i][j].Identifier].OriginalSize.Width * StretchWidthRatio;
                    break;
                case MARIO:
                    ImgHeight = AllImagesInfo[MARIO][SpritesGrid[i][j].Identifier].OriginalSize.Height * StretchHeightRatio;
                    ImgWidth = AllImagesInfo[MARIO][SpritesGrid[i][j].Identifier].OriginalSize.Width * StretchWidthRatio;
                    break;
                case BACKGROUND:
                    ImgHeight = StaticObjectWidth * StretchHeightRatio;
                    ImgWidth = StaticObjectHeight * StretchWidthRatio;
                }

                HandleImgAndInsert(i, j, ImgWidth, ImgHeight);
            }
}
internal int 
HandleUserInput()
{
    //Detecting if An image is deposited into main grid 
    Point MouseBlockCoords = { int((MouseRealCoords.x / StretchWidthRatio - (SpritesWidth + DividingLineWidth) + MainGridXPosition) / 16), int((MouseRealCoords.y / StretchHeightRatio) / 16) };
    if (IsMouseInMainGrid())
    {
        if (!MyMouseState.LeftButtonDown && MyMouseState.LeftButtonHeld)
            if (SpriteSelectedItem.X != -1)
            {
                if (!MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsEmpty && SpritesGrid[SpriteSelectedItem.X][SpriteSelectedItem.Y].Identifier == SHROOM)
                {
                    Point SpriteEletCoords = MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].SpriteElet;
                    if (SpritesGrid[SpriteEletCoords.X][SpriteEletCoords.Y].Identifier == BRICK || SpritesGrid[SpriteEletCoords.X][SpriteEletCoords.Y].Identifier == QM)
                    {
                        ShroomsPositions[ShroomsNbr] = { MouseBlockCoords.X * 16, MouseBlockCoords.Y * 16 };
                        ShroomsNbr++;
                    }
                }
                else
                {
                    MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsEmpty = false;
                    MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].SpriteElet = SpriteSelectedItem;
                    if (MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsPaired)
                    {
                        MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y + 1].IsEmpty = true;
                        MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsPaired = false;
                    }
                }

            }

        if (!MyMouseState.RightButtonDown && MyMouseState.RightButtonHeld)
            SuppressionMode = false;
        // RightMouseButtonDown Means that the image should be deleted from the grid

        if ((MyMouseState.RightButtonDown && !MyMouseState.RightButtonHeld) || SuppressionMode)
        {
            MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsEmpty = true;
            if (MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsPaired)
            {
                MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y + 1].IsEmpty = true;
                MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsPaired = false;
            }
            if (MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y - 1].IsPaired)
            {
                MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y - 1].IsEmpty = true;
                MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y - 1].IsPaired = false;
            }
            if (SpritesGrid[MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].SpriteElet.X][MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].SpriteElet.Y].Type == MARIO)
                MarioInsertedInEditor = false;
            SuppressionMode = true;
        }

        // Adding elements like blocks 

        if (MyMouseState.LeftButtonDown && !MyMouseState.LeftButtonHeld)
        {
            if (!MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsEmpty && SpritesGrid[MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].SpriteElet.X][MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].SpriteElet.Y].Type != MARIO)
                GridSelectedItem = MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].SpriteElet;
        }
        if (!MyMouseState.LeftButtonDown)
        {
            GridSelectedItem = { -1, -1 };
        }
        if (GridSelectedItem.X != -1)
        {
            MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].IsEmpty = false;
            MainGridInfo[MouseBlockCoords.X][MouseBlockCoords.Y].SpriteElet = GridSelectedItem;

        }
    }

    // Checking if save was clicked 
    if (MyMouseState.LeftButtonHeld && !MyMouseState.LeftButtonDown)
    {
        if (MouseRealCoords.x / StretchWidthRatio < SpritesWidth && MouseRealCoords.x / StretchWidthRatio > 0 &&
            MouseRealCoords.y / StretchHeightRatio > EditorOriginalBitmapHeight - SaveButtonImageHeight && MouseRealCoords.y / StretchHeightRatio < EditorOriginalBitmapHeight)
        {
            EditorSaveButtonClicked = true;
        }
    }
    if (EditorSaveButtonClicked)
    {
        HANDLE FileHnd = CreateFileA((LPCSTR)MarioLevelFileName,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (FileHnd == INVALID_HANDLE_VALUE)
        {
            OutputDebugStringA("couldn't create the file, FILE CREATE ERROR");
            return 0;
        }

        DWORD NumBytesWritten;
        LPVOID ShroomsNbrPtr = (void*)&ShroomsNbr;
        WriteFile(
            FileHnd,
            ShroomsNbrPtr,
            sizeof(ShroomsNbr),
            &NumBytesWritten,
            NULL
        );

        for (int i = 0; i < ShroomsNbr; i++)
        {
            LPVOID ShroomPosPtr = (void*)(ShroomsPositions + i);
            WriteFile(
                FileHnd,
                ShroomPosPtr,
                sizeof(Point),
                &NumBytesWritten,
                NULL
            );
        }

        for (int i = 0; i < 300; i++)
            for (int j = 0; j < 30; j++)
            {
                if (!MainGridInfo[i][j].IsEmpty && !MainGridInfo[i][j].IsPaired)
                {
                    Point EletCoords = MainGridInfo[i][j].SpriteElet;
                    char ObjType = 0;
                    int HeightOffset = 0;
                    if (SpritesGrid[EletCoords.X][EletCoords.Y].Type == TILE)
                        EditorStaticObjectsNbr++;
                    else if (SpritesGrid[EletCoords.X][EletCoords.Y].Type == ENEMY)
                    {
                        EditorEnemiesNbr++;
                        ObjType = 1;
                        if (SpritesGrid[EletCoords.X][EletCoords.Y].Identifier == TURTLE)
                            HeightOffset = AllImagesInfo[ENEMY][TURTLE].OriginalSize.Height - 16;
                    }
                    else if (SpritesGrid[EletCoords.X][EletCoords.Y].Type == MARIO)
                        ObjType = 2;
                    else if (SpritesGrid[EletCoords.X][EletCoords.Y].Type == BACKGROUND)
                    {
                        ObjType = 3;
                        EditorBgndNbr++;
                    }


                    DWORD NumBytesWritten;

                    LPVOID ObjTypePtr = (void*)(&ObjType);
                    WriteFile(
                        FileHnd,
                        ObjTypePtr,
                        sizeof(ObjType),
                        &NumBytesWritten,
                        NULL
                    );

                    int ObjectIdentifier = SpritesGrid[EletCoords.X][EletCoords.Y].Identifier;
                    LPVOID ObjectIdentifierPtr = (void*)(&ObjectIdentifier);
                    WriteFile(
                        FileHnd,
                        ObjectIdentifierPtr,
                        sizeof(ObjectIdentifier),
                        &NumBytesWritten,
                        NULL
                    );

                    int XPos = i * 16;
                    LPVOID XPosPtr = (void*)(&XPos);
                    WriteFile(
                        FileHnd,
                        XPosPtr,
                        sizeof(int),
                        &NumBytesWritten,
                        NULL
                    );


                    int YPos = j * 16 - HeightOffset;
                    LPVOID YPosPtr = (void*)(&YPos);
                    WriteFile(
                        FileHnd,
                        YPosPtr,
                        sizeof(int),
                        &NumBytesWritten,
                        NULL
                    );
                }
            }

        LPVOID EditorEnemiesNbrPtr = (void*)&EditorEnemiesNbr;
        WriteFile(
            FileHnd,
            EditorEnemiesNbrPtr,
            sizeof(EditorEnemiesNbr),
            &NumBytesWritten,
            NULL
        );

        LPVOID EditorStaticObjectsNbrPtr = (void*)&EditorStaticObjectsNbr;
        WriteFile(
            FileHnd,
            EditorStaticObjectsNbrPtr,
            sizeof(EditorStaticObjectsNbr),
            &NumBytesWritten,
            NULL
        );

        LPVOID EditorBgndNbrPtr = (void*)&EditorBgndNbr;
        WriteFile(
            FileHnd,
            EditorBgndNbrPtr,
            sizeof(EditorBgndNbr),
            &NumBytesWritten,
            NULL
        );

        CloseHandle(FileHnd);
        EditorSaveButtonClicked = false;
    }
    if (MyMouseState.LeftButtonDown)
        MyMouseState.LeftButtonHeld = true;
    else
        MyMouseState.LeftButtonHeld = false;

    if (MyMouseState.RightButtonDown)
        MyMouseState.RightButtonHeld = true;
    else
        MyMouseState.RightButtonHeld = false;
    if (MouseRealCoords.x > EditorBitmapWidth || MouseRealCoords.x < 0 || MouseRealCoords.y > EditorBitmapHeight || MouseRealCoords.y < 0)
        MyMouseState = { false, false, false, false };
}

internal void
InsertMainGrid()
{
    for (int i = 0; i < 300; i++)
        for (int j = 0; j < 30; j++)
        {
            int OriginalYIndex;
            int OriginalXIndex;
            if (!MainGridInfo[i][j].IsEmpty && !MainGridInfo[i][j].IsPaired)
            {
                Point ImgPos;
                ImgPos.X = 16 * i;
                int HeightOffset = 0;

                int k = MainGridInfo[i][j].SpriteElet.X, l = MainGridInfo[i][j].SpriteElet.Y;
                SpriteEletInfo SpriteElet = SpritesGrid[k][l];
                switch (SpriteElet.Type) {
                case BACKGROUND:
                    OriginalYIndex = int(float(SpriteElet.Identifier * 16) / BackgroundsImageWidth);
                    OriginalXIndex = ((SpriteElet.Identifier * 16) % BackgroundsImageWidth) / 16;
                    HeightOffset = 0;
                    ImgPos.Y = j * 16 - HeightOffset;
                    EditorMainGridInsertImage(AllTheSprites[BACKGROUND], BackgroundsImageWidth, { {OriginalXIndex, OriginalYIndex}, {StaticObjectWidth, StaticObjectHeight} }, ImgPos, true);
                    break;
                case TILE: case MARIO: case ENEMY:
                    HeightOffset = AllImagesInfo[SpriteElet.Type][SpriteElet.Identifier].OriginalSize.Height - 16;
                    ImgPos.Y = j * 16 - HeightOffset;
                    EditorMainGridInsertImage(AllTheSprites[SpriteElet.Type], AllTheSpritesWidth[SpriteElet.Type], AllImagesInfo[SpriteElet.Type][SpriteElet.Identifier], ImgPos, (SpriteElet.Type == TILE));
                    if (SpriteElet.Type == MARIO)
                        MarioInsertedInEditor = true;
                    break;
                }
                if (HeightOffset != 0)
                {
                    MainGridInfo[i][j - 1].IsPaired = true;
                    MainGridInfo[i][j - 1].IsEmpty = false;
                    if (MainGridInfo[i][j - 2].IsPaired)
                    {
                        MainGridInfo[i][j - 2].IsEmpty = true;
                        MainGridInfo[i][j - 2].IsPaired = false;
                    }
                }

            }

        }
}

internal void
Win32UpdateEditorWindow(HDC DeviceContext, RECT* ClientRect, int X, int Y, int Width, int Height)
{
    int EditorWindowWidth = ClientRect->right - ClientRect->left;
    int EditorWindowHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(DeviceContext,
        /*
        X, Y, Width, Height,
        X, Y, Width, Height,
        */
        0, 0, EditorBitmapWidth, EditorBitmapHeight,
        0, 0, EditorWindowWidth, EditorWindowHeight,
        EditorBitmapMemory,
        &EditorBitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);
}

internal void
Win32ResizeEditorDIBSection(int Width, int Height)
{

    if (EditorBitmapMemory)
    {
        VirtualFree(EditorBitmapMemory, 0, MEM_RELEASE);
    }

    EditorBitmapWidth = Width;
    EditorBitmapHeight = Height;

    EditorBitmapInfo.bmiHeader.biSize = sizeof(EditorBitmapInfo.bmiHeader);
    EditorBitmapInfo.bmiHeader.biWidth = EditorBitmapWidth;
    EditorBitmapInfo.bmiHeader.biHeight = -EditorBitmapHeight;
    EditorBitmapInfo.bmiHeader.biPlanes = 1;
    EditorBitmapInfo.bmiHeader.biBitCount = 32;
    EditorBitmapInfo.bmiHeader.biCompression = BI_RGB;

    int EditorBitmapMemorySize = (EditorBitmapWidth * EditorBitmapHeight) * BytesPerPixel;
    EditorBitmapMemory = VirtualAlloc(0, EditorBitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
CopyToDisplay(HWND EditorWindow)
{
    HDC DeviceContext = GetDC(EditorWindow);
    RECT ClientRect;
    GetClientRect(EditorWindow, &ClientRect);
    int EditorWindowWidth = ClientRect.right - ClientRect.left;
    int EditorWindowHeight = ClientRect.bottom - ClientRect.top;
    InsertStretchImageToZone(EditorOriginalBitmap, EditorOriginalBitmapWidth, EditorOriginalBitmapHeight, (uint32*)EditorBitmapMemory, EditorBitmapWidth, EditorBitmapHeight);

    Win32UpdateEditorWindow(DeviceContext, &ClientRect, 0, 0, EditorWindowWidth, EditorWindowHeight);
    ReleaseDC(EditorWindow, DeviceContext);
}

LRESULT CALLBACK
Win32EditorWindowCallback(HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_LBUTTONDOWN:
    {
        MyMouseState.LeftButtonDown = true;
    }break;
    case WM_LBUTTONUP:
    {
        MyMouseState.LeftButtonDown = false;
    }break;
    case WM_RBUTTONDOWN:
    {
        MyMouseState.RightButtonDown = true;
    }break;
    case WM_RBUTTONUP:
    {
        MyMouseState.RightButtonDown = false;
    }break;

    case WM_MOUSEWHEEL:
    {
        int zDelta = int(GET_WHEEL_DELTA_WPARAM(WParam));
        MainGridXPosition += zDelta / 4;
        if (MainGridXPosition < 0)
            MainGridXPosition = 0;
    }break;

    case WM_SIZE:
    {
        OutputDebugStringA("WM_SIZE \n");

        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        int Width = ClientRect.right - ClientRect.left;
        int Height = ClientRect.bottom - ClientRect.top;
        Win32ResizeEditorDIBSection(Width, Height);

    } break;

    case WM_CLOSE:
    {
        // TODO(casey): Handle this with a message to the user?
        EditorRunning = false;
    } break;

    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;

    case WM_DESTROY:
    {
        // TODO(casey): Handle this as an error - recreate window?
        EditorRunning = false;
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
        Win32UpdateEditorWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
        EndPaint(Window, &Paint);
    } break;

    default:
    {
        //OutputDebugStringA("default\n");
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
    QueryPerformanceFrequency(&PerformanceFrequencyL);
    int64 PerformanceFrequency = PerformanceFrequencyL.QuadPart;

    // Create the window class gor the Editor
    WNDCLASS EditorWindowClass = {};
    EditorWindowClass.lpfnWndProc = Win32EditorWindowCallback;
    EditorWindowClass.hInstance = Instance;
    EditorWindowClass.lpszClassName = "EditorWindowClass";

    if (RegisterClassA(&EditorWindowClass))
    {
        
        RECT ClientAreaRect = { 20, 20, 20 + EditorOriginalBitmapWidth * 2 ,20 + EditorOriginalBitmapHeight * 2 };
        AdjustWindowRect(&ClientAreaRect, WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE, false);
        HWND EditorWindow =
            CreateWindowExA(
                0,
                EditorWindowClass.lpszClassName,
                "MarioEditor",
                WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                ClientAreaRect.right - ClientAreaRect.left,
                ClientAreaRect.bottom - ClientAreaRect.top,
                0,
                0,
                Instance,
                0);
       


        EditorRunning = true;
        if (EditorWindow)
        {
            Initialise();
            LoadMainGridFromFIle();

            while (EditorRunning)
            {
                GetUserInput();
                InsertBackground();
                GetMouseCoords(EditorWindow);
                InsertSpriteElets();
                HandleUserInput();
                InsertMainGrid();
                CopyToDisplay(EditorWindow);
            }
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


// RotatingConsoleCube.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <cmath>
#include <tchar.h>
#include<iomanip>
#include <cstdint> // Ensure bit size regardless of CPU Architecture
#include<Windows.h>
#include<chrono>
#include <string>

using namespace std;

/*
3D Rotation Equations From:
	https://mathworld.wolfram.com/RotationMatrix.html
Results from Runnig Matlab Code:
		[cos(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) - sin(C)*(j*cos(A) - k*sin(A)),
		sin(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) + cos(C)*(j*cos(A) - k*sin(A)),
		cos(B)*(k*cos(A) + j*sin(A)) - i*sin(B)]
Where:
	i, j, k: Coordinate to Transform
	A, B, C: Angle of Rotation around the axis X, Y and Z respectively
*/

// Rotation Angle
float A{0}, B{0}, C{0};
float x, y, z;
float xNormal, yNormal, zNormal;
float ooz;

int xp, yp;
int idx;


// Define Shading Character
char bgASCII_Code = ' '; // Filling Character -bits

// Define Dimensions of each side of the cube and the area that we will use to display (Called Screen)
float cubeSide{ 40.0F };
int screenWidth{ 110 }, screenHeight{ 30 };

// Based on Screen size we'll get the FOV (Field Of View)
const float FOV{3.0F}; // Field of View is in Degrees
// Also called as near-Plane. Represents your screen and is the distance from yours eyes (camera) to the screen
float focal_length{1.0F / tan((FOV*3.1416F / 180.0F) / 2)};

// Create Dynamic Array
// Buffers to Store the depth of each "pixel" (zBuffer) and the character (buffer) on a screen of size screenWidth * screenHeight
char* screenBuffer = new char[screenWidth * screenHeight]; // 1 Bytes per char * (110 * 30) pixels = 3300 Bytes
float* screenZBuffer = new float[screenWidth * screenHeight]; // 4 Bytes per Float * (110 * 30) pixels = 13200 Bytes

// Normal Vectors RIGHT HAND REFERENCE SYSTEM
float* normFront{ new float[3]{ 0.0F, 0.0F, 1.0F} }; // {x, y, z}
float* normBack{ new float[3]{ 0.0F, 0.0F, -1.0F} }; // {x, y, z}
float* normUp{ new float[3]{ 0.0F, 1.0F, 0.0F} }; // {x, y, z}
float* normDown{ new float[3]{ 0.0F, -1.0F, 0.0F} }; // {x, y, z}
float* normLeft{ new float[3]{ 1.0F, 0.0F, 0.0F} }; // {x, y, z}
float* normRight{ new float[3]{ -1.0F, 0.0F, 0.0F} }; // {x, y, z}

float* lightVector{ new float[3]{ 0.0F, 0.2F, 1.0F, } }; // {x, y, z}


// 3D Perspective Rendering
//const float K1{10.0F };
float distanceFromCam{ 190.0 }; // same as K1
float incrementSpeed{ 1.5F };

float calculateX(float i, float j, float k) {
    return cos(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) - sin(C)*(j*cos(A) - k*sin(A));
}

float calculateY(float i, float j, float k) {
    return sin(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) + cos(C)*(j*cos(A) - k*sin(A));
}


float calculateZ(float i, float j, float k) {
    return (cos(B)*((k*cos(A)) + (j*sin(A))) -(i*sin(B)));
}

float dotProduct(float xNorm, float yNorm, float zNorm,float light[]){
    return (xNorm*light[0] + yNorm*light[1] + zNorm*light[2]);
}

float calculatexNormal(float param[]) {
    float i = param[0];
    float j = param[1];
    float k = param[2];
    float operation = cos(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) - sin(C)*(j*cos(A) - k*sin(A));
    return operation;
}
float calculateYNormal(float param[]) {
    float i = param[0];
    float j = param[1];
    float k = param[2];
    float operation = sin(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) + cos(C)*(j*cos(A) - k*sin(A));
    return operation;
}
float calculateZNormal(float param[]) {
    float i = param[0];
    float j = param[1];
    float k = param[2];
    float operation = (cos(B)*((k*cos(A)) + (j*sin(A))) -(i*sin(B)));
    return operation;
}

void floatToconsoleTitle(float value)
{
    //float L = normFront[3];
    // To get a float to String
    std::stringstream stream;
    stream.precision(6); // Set precision level
    stream << fixed;

    // Convert float to string
    stream << value;
    string strL  = stream.str();

    std::cout << "\033]0;" << strL << "\007";

}


void getSurface(float cubeX, float cubeY, float cubeZ, string face)
{
    x = calculateX(cubeX, cubeY, cubeZ);
    y = calculateY(cubeX, cubeY, cubeZ);
    z = calculateZ(cubeX, cubeY, cubeZ) + distanceFromCam; // Cube axis is on the screen, we have to push it into 3D space to be able to see it

    ooz = 1/z;

    // Calculate the Coordinates Prime to Put them on Screen
    xp = static_cast<int>(std::lround((screenWidth / 2) + (2 * x * focal_length * ooz))); // Multiply by 2 because horizontal spacing in console is smaller than vertical, to make it look squared
    // y is negated here, because Y goes up in 3D space but down on 2D displays.
    yp = static_cast<int>(std::lround((screenHeight / 2) - (y * focal_length * ooz)));

    char ch = ' ';
    float L = 0.0;

    // Get Normal Illumination
    if (face == "front")
    {
        ch = '#';
        // Front
        xNormal = calculatexNormal(normFront);
        yNormal = calculateYNormal(normFront);
        zNormal = calculateZNormal(normFront);

        L = dotProduct(xNormal,yNormal,zNormal, lightVector);
    }

    else if (face == "back")
    {
        ch = '.';
        // Front
        xNormal = calculatexNormal(normBack);
        yNormal = calculateYNormal(normBack);
        zNormal = calculateZNormal(normBack);

        L = dotProduct(xNormal,yNormal,zNormal, lightVector);
    }
    else if (face == "up")
    {
        ch = '^';
        // Front
        xNormal = calculatexNormal(normUp);
        yNormal = calculateYNormal(normUp);
        zNormal = calculateZNormal(normUp);

        L = dotProduct(xNormal,yNormal,zNormal, lightVector);
    }
    else if (face == "down")
    {
        ch = '$';
        // Front
        xNormal = calculatexNormal(normDown);
        yNormal = calculateYNormal(normDown);
        zNormal = calculateZNormal(normDown);

        L = dotProduct(xNormal,yNormal,zNormal, lightVector);

    }

    else if (face == "left")
    {
        ch = '~';
        // Front
        xNormal = calculatexNormal(normLeft);
        yNormal = calculateYNormal(normLeft);
        zNormal = calculateZNormal(normLeft);

        L = dotProduct(xNormal,yNormal,zNormal, lightVector);

    }

    else if (face == "right")
    {
        ch = '+';
        // Front
        xNormal = calculatexNormal(normRight);
        yNormal = calculateYNormal(normRight);
        zNormal = calculateZNormal(normRight);

        L = dotProduct(xNormal,yNormal,zNormal, lightVector);
    }

    idx = xp + (yp * screenWidth);

    if( L > 0.0)
    {
        //cout << "L Plotting: " << L << "\n";
        if (idx >= 0 && idx < screenWidth * screenHeight) {
            // If z is small, closer to the viewer and we have to show it
            if (ooz > screenZBuffer[idx]) {
                screenZBuffer[idx] = ooz;
                screenBuffer[idx] = ch;
            }
        }
    }
}

int main() {
    // Measure Frames to Ensure Smooth Run
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Create Console To Write and Clear Data
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // To Constantly execute
    while (1){
        // We'll need time differential per frame to calculate modification
        // to movement speeds, to ensure consistant movement, as ray-tracing is non-deterministic
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Fill at Byte level
        memset(screenBuffer, bgASCII_Code, sizeof(char) * screenWidth * screenHeight);
        // Set the zBuffer Array with for the Number of Bytes * 4 (each float has 4 bytes)
        memset(screenZBuffer, 0.0, sizeof(float) * screenWidth * screenHeight);

        // For loops with unary minus from -10 t0 +10 because origin is in the center of the cube and screen
        for (float cubeX = -cubeSide; cubeX < cubeSide; cubeX += incrementSpeed) {
        	for (float cubeY = -cubeSide; cubeY < cubeSide; cubeY += incrementSpeed) {
                getSurface(cubeX, cubeY, -cubeSide, "front");
                getSurface(cubeX, cubeY, cubeSide, "back");
                getSurface(-cubeSide, cubeY, cubeX, "left");
                getSurface(cubeSide, cubeY, cubeX, "right");
                getSurface(cubeX, cubeSide, cubeY, "down");
                getSurface(cubeX, -cubeSide, cubeY, "up");
        	}
        }

        // Display Frame
        // Writing in the {0,0} position avoids the console screen to keep scrolling
        WriteConsoleOutputCharacter(hConsole, screenBuffer, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);

        B += 0.5 *fElapsedTime;
        C += 0.5 *fElapsedTime;

    }

    return 0;
}

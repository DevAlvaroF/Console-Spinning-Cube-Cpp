/* ROTATION CONSOLE CUBE C++
 *          Developer: Alvaro Flores-Romero
 *          Version: C++ 17
 *
 *          References:
 *              1) servetgulnaroglu with .C approach (Heavily modified and added Illumination Vector approach)
 *              2) javidx9 on YouTube to sync code to execution time and Writing on Windows Console
 *              3) a1k0n for the basic theory to replicate on a Cube
 */

#include <iostream>
#include <cmath>
#include <tchar.h>
#include<iomanip>
#include<Windows.h>
#include<chrono>
#include <string>

using namespace std;

// =================================================================
// Global Variables. Easiest approach for Console "Graphics"
// =================================================================

float A{0}, B{0}, C{0}; // Rotation Angle
float xRotated, yRotated, zRotated;  // 3D coordinates Rotated
float xNormalRotated, yNormalRotated, zNormalRotated; // Normal Vector Location after Rotation
float ooz; // One Over Z (to reduce computations)

int xp, yp; // Projected Coordinates from 3D to 2D
int idx; // Index position of each point on screen


// Define Shading Character
char bgASCII_Code = ' '; // Filling Character


// Define Dimensions of each side of the cube and the area that we will use to display (Called Screen)
float cubeSide{ 40.0F };
int screenWidth{ 110 }, screenHeight{ 30 };


const float FOV{3.0F}; // Field of View is in Degrees. Based on Screen size
float focal_length{1.0F / tan((FOV*3.1416F / 180.0F) / 2)}; // Also called as near-Plane. Represents your screen.

// Create Dynamic Array
// Buffers to Store the depth of each "pixel" (zBuffer) and the character (buffer) on a screen of size screenWidth * screenHeight
char* screenBuffer = new char[screenWidth * screenHeight]; // 1 Bytes per char * (110 * 30) pixels = 3300 Bytes
float* screenZBuffer = new float[screenWidth * screenHeight]; // 4 Bytes per Float * (110 * 30) pixels = 13200 Bytes

// Normal Vectors for each Cube Face based on the RIGHT HAND REFERENCE SYSTEM
float* normFront{ new float[3]{ 0.0F, 0.0F, 1.0F} }; // {x, y, z}
float* normBack{ new float[3]{ 0.0F, 0.0F, -1.0F} }; // {x, y, z}
float* normUp{ new float[3]{ 0.0F, 1.0F, 0.0F} }; // {x, y, z}
float* normDown{ new float[3]{ 0.0F, -1.0F, 0.0F} }; // {x, y, z}
float* normLeft{ new float[3]{ 1.0F, 0.0F, 0.0F} }; // {x, y, z}
float* normRight{ new float[3]{ -1.0F, 0.0F, 0.0F} }; // {x, y, z}

// Direction of the "light" to know if cube pixels are Visible
float* lightVector{ new float[3]{ 0.0F, 0.2F, 1.0F, } }; // {x, y, z}


// 3D Perspective Rendering
float distanceFromCam{ 190.0 }; // same as K1
float incrementSpeed{ 1.5F };

/*
3D Rotation Equations From:
	https://mathworld.wolfram.com/RotationMatrix.html
Results from Running Matlab Symbolic Computation Code:
		[cos(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) - sin(C)*(j*cos(A) - k*sin(A)),
		sin(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) + cos(C)*(j*cos(A) - k*sin(A)),
		cos(B)*(k*cos(A) + j*sin(A)) - i*sin(B)]
Where:
	i, j, k: Coordinate to Transform
	A, B, C: Angle of Rotation around the axis X, Y and Z respectively
*/

float calculateX_Rotation(float i, float j, float k) {
    return cos(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) - sin(C)*(j*cos(A) - k*sin(A));
}

float calculateY_Rotation(float i, float j, float k) {
    return sin(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) + cos(C)*(j*cos(A) - k*sin(A));
}


float calculateZ_Rotation(float i, float j, float k) {
    return (cos(B)*((k*cos(A)) + (j*sin(A))) -(i*sin(B)));
}

float dotProduct(float xNorm, float yNorm, float zNorm,float light[]){
    return (xNorm*light[0] + yNorm*light[1] + zNorm*light[2]);
}

float calculatexNormal_Rotation(float param[]) {
    float i = param[0];
    float j = param[1];
    float k = param[2];
    float operation = cos(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) - sin(C)*(j*cos(A) - k*sin(A));
    return operation;
}
float calculateYNormal_Rotation(float param[]) {
    float i = param[0];
    float j = param[1];
    float k = param[2];
    float operation = sin(C)*(sin(B)*(k*cos(A) + j*sin(A)) + i*cos(B)) + cos(C)*(j*cos(A) - k*sin(A));
    return operation;
}
float calculateZNormal_Rotation(float param[]) {
    float i = param[0];
    float j = param[1];
    float k = param[2];
    float operation = (cos(B)*((k*cos(A)) + (j*sin(A))) -(i*sin(B)));
    return operation;
}

// To print string into Console Window (Good for debugging when entire screen is being used)
void floatToconsoleTitle(float value)
{
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
    // Calculate Cube XYZ after Rotation
    xRotated = calculateX_Rotation(cubeX, cubeY, cubeZ);
    yRotated = calculateY_Rotation(cubeX, cubeY, cubeZ);
    zRotated = calculateZ_Rotation(cubeX, cubeY, cubeZ) + distanceFromCam; // Cube axis is on the screen, we have to push it into 3D space to be able to see it

    ooz = 1/zRotated;

    // Calculate the Coordinates of the 2D Projection
    xp = static_cast<int>(std::lround((screenWidth / 2) + (2 * xRotated * focal_length * ooz))); // Multiply by 2 because horizontal spacing in console is smaller than vertical, to make it look squared
    // y is negated here, because Y goes up in 3D space but down on 2D displays.
    yp = static_cast<int>(std::lround((screenHeight / 2) - (yRotated * focal_length * ooz)));

    char ch = ' '; // Character initialization of face  Character
    float L = 0.0; // Illumination

    // Get each face Normal Vector Rotation and Dot Product with Illumination Vector
    if (face == "front")
    {
        ch = '#';
        // Front
        xNormalRotated = calculatexNormal_Rotation(normFront);
        yNormalRotated = calculateYNormal_Rotation(normFront);
        zNormalRotated = calculateZNormal_Rotation(normFront);

        L = dotProduct(xNormalRotated,yNormalRotated,zNormalRotated, lightVector);
    }

    else if (face == "back")
    {
        ch = '.';
        // Back
        xNormalRotated = calculatexNormal_Rotation(normBack);
        yNormalRotated = calculateYNormal_Rotation(normBack);
        zNormalRotated = calculateZNormal_Rotation(normBack);

        L = dotProduct(xNormalRotated,yNormalRotated,zNormalRotated, lightVector);
    }
    else if (face == "up")
    {
        ch = '^';
        // Up
        xNormalRotated = calculatexNormal_Rotation(normUp);
        yNormalRotated = calculateYNormal_Rotation(normUp);
        zNormalRotated = calculateZNormal_Rotation(normUp);

        L = dotProduct(xNormalRotated,yNormalRotated,zNormalRotated, lightVector);
    }
    else if (face == "down")
    {
        ch = '$';
        // Down
        xNormalRotated = calculatexNormal_Rotation(normDown);
        yNormalRotated = calculateYNormal_Rotation(normDown);
        zNormalRotated = calculateZNormal_Rotation(normDown);

        L = dotProduct(xNormalRotated,yNormalRotated,zNormalRotated, lightVector);

    }

    else if (face == "left")
    {
        ch = '~';
        // Left
        xNormalRotated = calculatexNormal_Rotation(normLeft);
        yNormalRotated = calculateYNormal_Rotation(normLeft);
        zNormalRotated = calculateZNormal_Rotation(normLeft);

        L = dotProduct(xNormalRotated,yNormalRotated,zNormalRotated, lightVector);

    }

    else if (face == "right")
    {
        ch = '+';
        // Right
        xNormalRotated = calculatexNormal_Rotation(normRight);
        yNormalRotated = calculateYNormal_Rotation(normRight);
        zNormalRotated = calculateZNormal_Rotation(normRight);

        L = dotProduct(xNormalRotated,yNormalRotated,zNormalRotated, lightVector);
    }

    // Get Index based on 2D Projection and Screen Width
    idx = xp + (yp * screenWidth);

    if( L > 0.0)
    {
        // If idx in screen range
        if (idx >= 0 && idx < screenWidth * screenHeight) {
            // If z is small, closer to the viewer and we have to show it
            if (ooz > screenZBuffer[idx]) {
                screenZBuffer[idx] = ooz; // Add Pixel Depth
                screenBuffer[idx] = ch; // Add Pixel Character to show on screen
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
        // Time Delta to ensure constant cube sping (ray-tracing is non-deterministic)
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Fill at Byte level the console screen with "Blank" Char
        memset(screenBuffer, bgASCII_Code, sizeof(char) * screenWidth * screenHeight);
        // Set the zBuffer Array to 0.0 with for the Number of Bytes * 4 (each float has 4 bytes)
        memset(screenZBuffer, 0.0, sizeof(float) * screenWidth * screenHeight);

        // For loops with unary minus from -10 t0 +10 because origin is in the center of the cube in 3D Space
        for (float cubeX = -cubeSide; cubeX < cubeSide; cubeX += incrementSpeed) {
        	for (float cubeY = -cubeSide; cubeY < cubeSide; cubeY += incrementSpeed) {
                // Plot faces based on which axis remains constant and which changes
                getSurface(cubeX, cubeY, -cubeSide, "front");
                getSurface(cubeX, cubeY, cubeSide, "back");
                getSurface(-cubeSide, cubeY, cubeX, "left");
                getSurface(cubeSide, cubeY, cubeX, "right");
                getSurface(cubeX, cubeSide, cubeY, "down");
                getSurface(cubeX, -cubeSide, cubeY, "up");
        	}
        }

        // Display screenBuffer: Writing in the {0,0} position avoids the console screen to keep scrolling
        WriteConsoleOutputCharacter(hConsole, screenBuffer, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);

        // A Angle Not Changed
        B += 0.5 *fElapsedTime;
        C += 0.5 *fElapsedTime;
    }
    return 0;
}

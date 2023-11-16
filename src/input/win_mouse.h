#pragma once

class Mouse
{
public:
    static void leftInput(bool left, float x, float y);
    static void mouseMove(float x, float y);

public:
    static const float getX() { return position.x; }
    static const float getY() { return position.y; }

    static const bool isLeftDown() { return buttons.left; }
    static const bool isRightDown() { return buttons.right; }

private:
    static struct Buttons
    {
        bool left = false;
        bool right = false;
    } buttons;

    static struct Position
    {
        float x = 0;
        float y = 0;
    } position;

    static bool test;
};
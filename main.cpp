#include <windows.h>
#include <iostream>
#include <ctime>

#define WINDOW_TITLE "WinAPI - Sprite Movement"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define SPRITE_WIDTH 48
#define SPRITE_HEIGHT 64

#define MAX_SPEED 16
#define STEP_POWER 8
#define FRICTION 1

#define ID_TIMER 1

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HBITMAP LoadBitMap();
void DrawRectangle(HDC hdc, HBRUSH brush, RECT& _rect, int x, int y);
void DrawBitmap(HDC winDC, HBITMAP hBmp, int x, int y);
void ChangeTexture(WPARAM wParam);
void KeyboardButtonDownListener(WPARAM wParam, POINT& _speed, POINT& _dir);
void EnableMouseControl(LPARAM lParam, POINT _coord);
void DisableMouseControl();
void MouseLKMListener(LPARAM lParam, POINT& _mouse, POINT& _coord);
void CheckShift(WPARAM wParam, bool pressed);
void MouseWheelListener(WPARAM wParam, POINT& _speed, POINT& _dir);
void GetRoomSize(HWND hWnd, int& width, int& height);
void Move(POINT& _coord, POINT _speed, POINT _dir);
void RestrictMovement(POINT& _coord, POINT& _dir);
void ReduceSpeed(POINT& _speed);

void ChangeAutoMovement(WPARAM wParam);
void AutoMove(POINT& _speed, POINT& _dir);

char szClassName[] = "SpriteMovementClass";

int roomWidth;
int roomHeight;

POINT coord = {0, 0};
POINT speed = {0, 0};
POINT dir = {0, 0};

POINT mouse = {0, 0};

HBRUSH hBrush;
RECT rect;
HBITMAP hBitmap;

bool withTexture = false;

bool shiftPressed = false;
bool mouseControl = false;
bool autoMovement = false;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    MSG msg; HWND hWnd; WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = szClassName;
    wc.hIconSm = wc.hIcon;

    if (!RegisterClassEx(&wc))
    {
        return EXIT_FAILURE;
    }

    if (hWnd = CreateWindow
                (
                        wc.lpszClassName, WINDOW_TITLE,
                        WS_OVERLAPPEDWINDOW, 0, 0,
                        WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, wc.hInstance, nullptr
                );
            hWnd == INVALID_HANDLE_VALUE)
    {
        return EXIT_FAILURE;
    }

    GetRoomSize(hWnd, roomWidth, roomHeight);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;

    switch (uMsg)
    {
        case WM_CREATE:
        {
            srand(time(nullptr));
            hBrush = CreateSolidBrush(RGB(200,100,0));
            hBitmap = LoadBitMap();
            SetTimer(hWnd, ID_TIMER, 15, nullptr);
            break;
        }
        case WM_PAINT:
        {
            HDC hdc = BeginPaint(hWnd, &ps);

            if (withTexture)
            {
                DrawBitmap(hdc, hBitmap, coord.x, coord.y);
            }
            else
            {
                DrawRectangle(hdc, hBrush, rect, coord.x, coord.y);
            }

            ReleaseDC(hWnd, hdc);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_SIZE:
        {
            GetRoomSize(hWnd, roomWidth, roomHeight);
            break;
        }
        case WM_TIMER:
        {
            Move(coord, speed, dir);
            ReduceSpeed(speed);
            RestrictMovement(coord, dir);

            AutoMove(speed, dir);

            if (!mouseControl)
            {
                InvalidateRect(hWnd, nullptr, true);
            }

            break;
        }
        case WM_KEYDOWN:
        {
            KeyboardButtonDownListener(wParam, speed, dir);
            CheckShift(wParam, true);
            ChangeTexture(wParam);
            ChangeAutoMovement(wParam);
            break;
        }
        case WM_KEYUP:
        {
            CheckShift(wParam, false);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            EnableMouseControl(lParam, coord);
            break;
        }
        case WM_MOUSEMOVE:
        {
            MouseLKMListener(lParam, mouse, coord);
            RestrictMovement(coord, dir);
            InvalidateRect(hWnd, nullptr, true);
            break;
        }
        case WM_LBUTTONUP:
        {
            DisableMouseControl();
            break;
        }
        case WM_MOUSEWHEEL:
        {
            MouseWheelListener(wParam, speed, dir);
            break;
        }
        case WM_DESTROY:
        {
            KillTimer(hWnd, ID_TIMER);
            PostQuitMessage(EXIT_SUCCESS);
            break;
        }
        default:
        {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

    }

    return 0;
}

HBITMAP LoadBitMap()
{
    return (HBITMAP)LoadImage(nullptr, R"(..\coin.bmp)", IMAGE_BITMAP, SPRITE_WIDTH, SPRITE_HEIGHT, LR_LOADFROMFILE);
}

void DrawRectangle(HDC hdc, HBRUSH brush, RECT& _rect, int x, int y)
{
    _rect.left = x;
    _rect.right = x + SPRITE_WIDTH;
    _rect.top = y;
    _rect.bottom = y + SPRITE_HEIGHT;

    Rectangle(hdc, _rect.left, _rect.top, _rect.right, _rect.bottom);
    FillRect(hdc, &_rect, brush);
}

void DrawBitmap(HDC winDC, HBITMAP hBmp, int x, int y)
{
    HDC memDC = CreateCompatibleDC(winDC);
    auto hOldBmp = (HBITMAP)SelectObject(memDC, hBmp);

    BitBlt(winDC, x, y, SPRITE_WIDTH, SPRITE_HEIGHT, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, hOldBmp);
    DeleteDC(memDC);
}

void ChangeTexture(WPARAM wParam)
{
    if (wParam == 0x51) // btn Q
    {
        withTexture = !withTexture;
    }
}

void GetRoomSize(HWND hWnd, int& width, int& height)
{
    RECT rect;
    if(GetClientRect(hWnd, &rect))
    {
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }
}

void Move(POINT& _coord, POINT _speed, POINT _dir)
{
    _coord.x += _dir.x * _speed.x;
    _coord.y += _dir.y * _speed.y;
}

void RestrictMovement(POINT& _coord, POINT& _dir)
{
    if (_coord.x < 0 || _coord.x > (long)roomWidth - SPRITE_WIDTH)
    {
        _dir.x = -_dir.x;
    }

    if (_coord.y < 0 || _coord.y > (long)roomHeight - SPRITE_HEIGHT)
    {
        _dir.y = -_dir.y;
    }

    _coord.x = std::min((long)roomWidth - SPRITE_WIDTH, std::max(0L, _coord.x));
    _coord.y = std::min((long)roomHeight - SPRITE_HEIGHT, std::max(0L, _coord.y));
}

void ReduceSpeed(POINT& _speed)
{
    _speed.x = _speed.x > 0 ? _speed.x - FRICTION : _speed.x;
    _speed.y = _speed.y > 0 ? _speed.y - FRICTION : _speed.y;
}

void KeyboardButtonDownListener(WPARAM wParam, POINT& _speed, POINT& _dir)
{
    switch (wParam)
    {
        case 0x41: // btn A
            _speed.x += STEP_POWER;
            _dir.x = -1;
            break;
        case 0x44: // btn D
            _speed.x += STEP_POWER;
            _dir.x = 1;
            break;
        case 0x57: // btn W
            _speed.y += STEP_POWER;
            _dir.y = -1;
            break;
        case 0x53: // btn S
            _speed.y += STEP_POWER;
            _dir.y = 1;
            break;
        default:
            return;
    }

    _speed.x = std::min((long)MAX_SPEED, std::max(0L, _speed.x));
    _speed.y = std::min((long)MAX_SPEED, std::max(0L, _speed.y));
}

void EnableMouseControl(LPARAM lParam, POINT _coord)
{
    POINT _mouse;
    _mouse.x = LOWORD(lParam);
    _mouse.y = HIWORD(lParam);

    if (_mouse.x >= _coord.x
        && _mouse.x <= _coord.x + SPRITE_WIDTH
        && _mouse.y >= _coord.y
        && _mouse.y <= _coord.y + SPRITE_HEIGHT)
    {
        mouseControl = true;
    }

}

void DisableMouseControl()
{
    mouseControl = false;
}

void MouseLKMListener(LPARAM lParam, POINT& _mouse, POINT& _coord)
{
    if (mouseControl)
    {
        _mouse.x = LOWORD(lParam);
        _mouse.y = HIWORD(lParam);

        _coord.x = _mouse.x - SPRITE_WIDTH / 2;
        _coord.y = _mouse.y - SPRITE_HEIGHT / 2;
    }
}

void CheckShift(WPARAM wParam, bool pressed)
{
    if (wParam == VK_SHIFT)
    {
        shiftPressed = pressed;
    }
}

void MouseWheelListener(WPARAM wParam, POINT& _speed, POINT& _dir)
{
    int delta = GET_WHEEL_DELTA_WPARAM(wParam);

    if (shiftPressed)
    {
        _dir.x = delta > 0 ? -1 : delta < 0 ? 1 : 0;
        _speed.x += STEP_POWER;

        _speed.x = std::min((long)MAX_SPEED, std::max(0L, _speed.x));
    }
    else
    {
        _dir.y = delta > 0 ? -1 : delta < 0 ? 1 : 0;
        _speed.y += STEP_POWER;

        _speed.y = std::min((long)MAX_SPEED, std::max(0L, _speed.y));
    }
}

void ChangeAutoMovement(WPARAM wParam)
{
    if (wParam == 0x45) // btn E
    {
        autoMovement = !autoMovement;
    }
}

void AutoMove(POINT& _speed, POINT& _dir)
{
    if (autoMovement)
    {
        _dir.x = _dir.x == 0 ? rand() % 3 - 1 : _dir.x;
        _dir.y = _dir.y == 0 ? rand() % 3 - 1 : _dir.y;

        _speed.x = MAX_SPEED / 2;
        _speed.y = MAX_SPEED / 2;
    }
}
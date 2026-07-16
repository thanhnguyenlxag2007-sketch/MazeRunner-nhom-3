// MAZE RUNNER - SẢN PHẨM HOÀN CHỈNH
// Bắt buộc sử dụng trực tiếp ảnh characters.png cho sáu nhân vật.
// Chương trình sử dụng hàm và struct, không sử dụng lập trình hướng đối tượng.

#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <random>
#include <string>
#include <vector>

using namespace std;

// ========================= CẤU HÌNH CHUNG =========================
// Kích thước bản đồ, ô vuông, vùng giao diện và cửa sổ trò chơi.
const int ROWS = 21;
const int COLS = 31;
const int TILE = 28;
const int UI_HEIGHT = 90;
const int SCREEN_W = COLS * TILE;
const int SCREEN_H = ROWS * TILE + UI_HEIGHT;
const int ITEM_COUNT = 5;
const int MAP_COUNT = 12;
const int CHARACTER_COUNT = 6;
const float ENEMY_DELAY = 0.42f;

// ========================= KIỂU DỮ LIỆU =========================
// Các trạng thái giúp main() biết cần cập nhật và vẽ màn hình nào.
enum ScreenState {
    MAIN_MENU,
    GUIDE_SCREEN,
    CHARACTER_SCREEN,
    PLAYING_SCREEN
}; // Quản lý màn hình hiện tại của trò chơi.

enum CharacterType {
    NOBITA,
    JAIAN,
    SUNEO,
    SHIZUKA,
    DORAEMON,
    DEKISUGI
}; // Mã số đại diện cho từng nhân vật.

struct Enemy {
    int r, c;
    float timer;
}; // Lưu vị trí và thời gian di chuyển của quỷ.

struct Item {
    int r, c;
    bool collected;
}; // Lưu vị trí và trạng thái của vật phẩm.

struct MapTheme {
    Color wall;
    Color wallTop;
    Color wallShadow;
    Color floorA;
    Color floorB;
    Color accent;
    Color background;
}; // Lưu bảng màu dùng để vẽ một bản đồ.

struct CharacterSheet {
    Texture2D texture;
    bool loaded;
}; // Quản lý texture chứa sáu nhân vật.

// ========================= DỮ LIỆU GIAO DIỆN =========================
const char* CHARACTER_NAMES[CHARACTER_COUNT] = {
    u8"Nobita", u8"Jaian", u8"Suneo",
    u8"Shizuka", u8"Doraemon", u8"Dekisugi"
}; // Tên nhân vật hiển thị trên giao diện.

const Color CHARACTER_COLORS[CHARACTER_COUNT] = {
    {250, 208, 45, 255}, {239, 112, 35, 255}, {77, 180, 110, 255},
    {241, 112, 160, 255}, {32, 151, 219, 255}, {82, 156, 220, 255}
}; // Màu nhấn riêng của từng nhân vật.

const char* MAP_NAMES[MAP_COUNT] = {
    u8"Thành phố xanh", u8"Thung lũng tím", u8"Rừng ngọc bích",
    u8"Sa mạc hoàng hôn", u8"Đại dương sâu", u8"Đền cổ",
    u8"Vương quốc kẹo", u8"Núi lửa đỏ", u8"Băng nguyên",
    u8"Khu vườn đêm", u8"Phòng thí nghiệm", u8"Lâu đài bóng tối"
}; // Tên của 12 bản đồ.

vector<MapTheme> themes = {
    {{35, 78, 150, 255},  {75, 145, 228, 255}, {18, 43, 91, 255},   {225, 241, 255, 255}, {202, 225, 246, 255}, {255, 211, 65, 255},  {11, 27, 55, 255}},
    {{93, 48, 143, 255},  {174, 99, 224, 255}, {49, 24, 82, 255},   {245, 230, 255, 255}, {225, 201, 244, 255}, {255, 137, 207, 255}, {31, 14, 49, 255}},
    {{24, 112, 83, 255},  {63, 186, 130, 255}, {10, 59, 45, 255},   {222, 247, 231, 255}, {195, 229, 208, 255}, {255, 189, 67, 255},  {7, 41, 32, 255}},
    {{172, 70, 48, 255},  {234, 132, 79, 255}, {90, 33, 25, 255},   {255, 237, 207, 255}, {244, 211, 168, 255}, {255, 209, 65, 255},  {61, 23, 18, 255}},
    {{16, 105, 131, 255}, {53, 185, 205, 255}, {5, 55, 72, 255},    {218, 248, 249, 255}, {184, 226, 230, 255}, {255, 108, 93, 255},  {5, 38, 49, 255}},
    {{105, 70, 44, 255},  {181, 131, 79, 255}, {54, 34, 20, 255},   {245, 233, 203, 255}, {223, 205, 169, 255}, {111, 190, 242, 255}, {39, 25, 15, 255}},
    {{190, 69, 126, 255}, {247, 138, 180, 255},{103, 32, 70, 255},  {255, 240, 246, 255}, {249, 210, 226, 255}, {111, 222, 198, 255}, {66, 21, 46, 255}},
    {{129, 36, 30, 255},  {228, 77, 39, 255},  {62, 15, 17, 255},   {247, 218, 184, 255}, {224, 181, 139, 255}, {255, 190, 37, 255},  {42, 9, 13, 255}},
    {{62, 111, 159, 255}, {151, 209, 236, 255},{29, 59, 96, 255},   {235, 250, 255, 255}, {205, 235, 246, 255}, {122, 241, 255, 255}, {19, 42, 70, 255}},
    {{46, 83, 65, 255},   {102, 155, 109, 255},{23, 43, 36, 255},   {224, 235, 218, 255}, {196, 216, 190, 255}, {205, 125, 243, 255}, {16, 29, 25, 255}},
    {{69, 79, 94, 255},   {128, 150, 169, 255},{31, 37, 47, 255},   {226, 236, 238, 255}, {198, 216, 220, 255}, {77, 230, 207, 255},  {20, 25, 34, 255}},
    {{45, 35, 72, 255},   {91, 68, 135, 255},  {20, 13, 39, 255},   {215, 207, 225, 255}, {183, 171, 200, 255}, {232, 63, 92, 255},   {12, 8, 25, 255}}
}; // Danh sách 12 bảng màu tương ứng với 12 bản đồ.

// ========================= FONT VÀ TIẾNG VIỆT =========================
// Tạo danh sách ký tự Unicode cần dùng cho tiếng Việt.
vector<int> CreateVietnameseCodepoints() {
    vector<int> codepoints;
    for (int i = 32; i <= 126; i++) codepoints.push_back(i);
    for (int i = 0x00C0; i <= 0x024F; i++) codepoints.push_back(i);
    for (int i = 0x1E00; i <= 0x1EFF; i++) codepoints.push_back(i);
    return codepoints;
}

// Tự tìm font có sẵn trên Windows để file exe không cần mang font rời.
Font LoadVietnameseFont(bool& loadedCustomFont) {
    const char* fontPaths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "../assets/NotoSans-Regular.ttf",
        "assets/NotoSans-Regular.ttf"
    }; // Các đường dẫn font tiếng Việt có thể sử dụng.

    vector<int> codepoints = CreateVietnameseCodepoints();
    for (const char* path : fontPaths) {
        if (FileExists(path)) {
            Font font = LoadFontEx(path, 38, codepoints.data(), (int)codepoints.size());
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            loadedCustomFont = true;
            return font;
        }
    } // Nếu tìm thấy font phù hợp thì tải và trả về ngay.

    loadedCustomFont = false;
    return GetFontDefault();
}

// Vẽ văn bản bằng font Unicode; centered=true sẽ căn giữa theo trục X.
void DrawVN(Font font, const char* text, float x, float y,
            float size, Color color, bool centered = false) {
    Vector2 textSize = MeasureTextEx(font, text, size, 1.0f);
    if (centered) x -= textSize.x / 2.0f;
    DrawTextEx(font, text, {x, y}, size, 1.0f, color);
} // Kết thúc hàm vẽ chữ tiếng Việt.

// Tạo mê cung hoàn chỉnh bằng DFS. Mỗi seed cho ra một bản đồ cố định khác nhau.
vector<string> GenerateMaze(unsigned int seed) {
    vector<string> maze(ROWS, string(COLS, '#')); // Khởi tạo bản đồ toàn tường.
    mt19937 rng(seed); // Seed giúp mỗi bản đồ được tạo giống nhau ở mọi lần chạy.
    vector<pair<int, int>> stack; // Ngăn xếp dùng cho thuật toán DFS.
    stack.push_back({1, 1}); // Bắt đầu tạo đường tại ô (1, 1).
    maze[1][1] = '.'; // Biến ô bắt đầu thành đường đi.

    while (!stack.empty()) {
        int r = stack.back().first;
        int c = stack.back().second;
        vector<pair<int, int>> directions = {
            {-2, 0}, {2, 0}, {0, -2}, {0, 2}
        }; // Bốn hướng cách nhau hai ô: lên, xuống, trái, phải.
        shuffle(directions.begin(), directions.end(), rng); // Xáo trộn hướng để map đa dạng.

        bool moved = false; // Đánh dấu DFS có mở được đường mới hay không.
        for (const pair<int, int>& direction : directions) {
            int nr = r + direction.first;
            int nc = c + direction.second;
            if (nr > 0 && nr < ROWS - 1 && nc > 0 && nc < COLS - 1 &&
                maze[nr][nc] == '#') {
                maze[r + direction.first / 2][c + direction.second / 2] = '.';
                maze[nr][nc] = '.';
                stack.push_back({nr, nc});
                moved = true;
                break;
            }
        } 
        if (!moved) stack.pop_back(); // Hết đường thì quay lại ô trước đó.
    }

    // Mở thêm một số vách để bản đồ có nhiều đường vòng và đẹp tự nhiên hơn.
    vector<pair<int, int>> removableWalls; // Danh sách các vách có thể mở thêm.
    for (int r = 2; r < ROWS - 2; r++) {
        for (int c = 2; c < COLS - 2; c++) {
            if (maze[r][c] != '#') continue;
            bool vertical = maze[r - 1][c] == '.' && maze[r + 1][c] == '.';
            bool horizontal = maze[r][c - 1] == '.' && maze[r][c + 1] == '.';
            if (vertical || horizontal) removableWalls.push_back({r, c});
        } // Chỉ lấy vách nằm giữa hai ô đường đi.
    }
    shuffle(removableWalls.begin(), removableWalls.end(), rng); // Chọn vách ngẫu nhiên.
    int openings = min(18, (int)removableWalls.size()); // Không mở quá 18 vách.
    for (int i = 0; i < openings; i++) {
        maze[removableWalls[i].first][removableWalls[i].second] = '.';
    } // Hoàn tất việc tạo thêm đường vòng.

    maze[1][1] = 'S'; // Ký hiệu vị trí xuất phát.
    maze[ROWS - 2][COLS - 2] = 'E'; // Ký hiệu cổng thoát.
    return maze; // Trả về bản đồ hoàn chỉnh.
}

// Tạo trước 12 bản đồ từ 12 seed để người chơi có nhiều màn khác nhau.
vector<vector<string>> BuildMazeList() {
    unsigned int seeds[MAP_COUNT] = {
        41, 73, 109, 151, 197, 251, 313, 401, 503, 617, 733, 887
    }; // Mỗi seed tạo ra một bản đồ cố định khác nhau.
    vector<vector<string>> maps;
    for (int i = 0; i < MAP_COUNT; i++) {
        maps.push_back(GenerateMaze(seeds[i]));
    }
    return maps;
}

// ========================= BẢN ĐỒ VÀ TÌM ĐƯỜNG =========================
// Kiểm tra một tọa độ có nằm ngoài map hoặc là tường hay không.
bool IsWall(const vector<string>& maze, int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return true;
    return maze[r][c] == '#';
}

// Người chơi và quỷ chỉ được đi vào ô không phải tường.
bool CanMove(const vector<string>& maze, int r, int c) {
    return !IsWall(maze, r, c);
}

// Dùng BFS để tìm đường ngắn nhất từ (sr, sc) tới (tr, tc).
vector<pair<int, int>> FindPath(const vector<string>& maze,
                                int sr, int sc, int tr, int tc) {
    vector<vector<int>> visited(ROWS, vector<int>(COLS, 0)); // Đánh dấu ô đã duyệt.
    vector<vector<pair<int, int>>> parent(
        ROWS, vector<pair<int, int>>(COLS, {-1, -1})
    );
    queue<pair<int, int>> cells; // Hàng đợi đặc trưng của BFS.
    cells.push({sr, sc});
    visited[sr][sc] = 1;

    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};
    while (!cells.empty()) {
        pair<int, int> current = cells.front();
        cells.pop();
        int r = current.first;
        int c = current.second;
        if (r == tr && c == tc) break;

        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
                !visited[nr][nc] && CanMove(maze, nr, nc)) {
                visited[nr][nc] = 1;
                parent[nr][nc] = {r, c};
                cells.push({nr, nc});
            }
        }
    }

    // Truy ngược mảng parent để khôi phục đường đi hoàn chỉnh.
    vector<pair<int, int>> path;
    if (!visited[tr][tc]) return path;
    int r = tr;
    int c = tc;
    while (!(r == sr && c == sc)) {
        path.push_back({r, c});
        pair<int, int> previous = parent[r][c];
        r = previous.first;
        c = previous.second;
    }
    reverse(path.begin(), path.end());
    return path;
}

// Quét map để lấy tọa độ ký hiệu S (bắt đầu) và E (cổng thoát).
void FindStartAndExit(const vector<string>& maze,
                      int& playerR, int& playerC, int& exitR, int& exitC) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (maze[r][c] == 'S') {
                playerR = r;
                playerC = c;
            } else if (maze[r][c] == 'E') {
                exitR = r;
                exitC = c;
            }
        }
    }
}

// Đặt quỷ gần cổng thoát nhưng vẫn nằm trên một ô đường đi hợp lệ.
void PlaceEnemy(const vector<string>& maze, Enemy& enemy,
                int playerR, int playerC, int exitR, int exitC) {
    vector<pair<int, int>> path = FindPath(maze, exitR, exitC, playerR, playerC);
    int position = min(4, (int)path.size() - 1);
    if (path.empty()) {
        enemy.r = exitR;
        enemy.c = exitC;
    } else {
        enemy.r = path[position].first;
        enemy.c = path[position].second;
    }
    enemy.timer = 0.0f;
}

// ========================= VẬT PHẨM VÀ TRẠNG THÁI GAME =========================
// Chọn ngẫu nhiên các ô trống để đặt vật phẩm, tránh trùng vị trí đặc biệt.
void AddRandomItems(const vector<string>& maze, vector<Item>& items,
                    int playerR, int playerC, int exitR, int exitC,
                    int enemyR, int enemyC) {
    items.clear();
    while ((int)items.size() < ITEM_COUNT) {
        int r = GetRandomValue(1, ROWS - 2);
        int c = GetRandomValue(1, COLS - 2);
        if (!CanMove(maze, r, c)) continue;
        if ((r == playerR && c == playerC) ||
            (r == exitR && c == exitC) ||
            (r == enemyR && c == enemyC)) continue;

        bool duplicated = false;
        for (const Item& item : items) {
            if (item.r == r && item.c == c) duplicated = true;
        }
        if (!duplicated) items.push_back({r, c, false});
    }
}

// Khởi tạo lại toàn bộ dữ liệu khi bắt đầu, chơi lại hoặc đổi bản đồ.
void ResetGame(const vector<vector<string>>& mazeList,
               vector<string>& maze, int& playerR, int& playerC,
               int& exitR, int& exitC, Enemy& enemy, vector<Item>& items,
               int& score, int& itemsCollected, int& currentMap,
               bool& win, bool& gameOver, float& playerSpeed,
               float& moveCharge, float& boostMessageTimer, int selectedMap) {
    // selectedMap < 0 nghĩa là chọn map ngẫu nhiên.
    if (selectedMap < 0) {
        currentMap = GetRandomValue(0, (int)mazeList.size() - 1);
    } else {
        currentMap = selectedMap % (int)mazeList.size();
    }

    maze = mazeList[currentMap];
    FindStartAndExit(maze, playerR, playerC, exitR, exitC);
    PlaceEnemy(maze, enemy, playerR, playerC, exitR, exitC);
    AddRandomItems(maze, items, playerR, playerC, exitR, exitC, enemy.r, enemy.c);
    // Đưa các chỉ số của ván chơi về giá trị ban đầu.
    score = 0;
    itemsCollected = 0;
    playerSpeed = 1.0f;
    moveCharge = 0.0f;
    boostMessageTimer = 0.0f;
    win = false;
    gameOver = false;
}

// Kiểm tra nhặt vật phẩm; mỗi món cộng 10 điểm và tăng 0,2 tốc độ.
bool CheckCollectItem(vector<Item>& items, int playerR, int playerC,
                      int& score, int& itemsCollected, float& playerSpeed) {
    for (Item& item : items) {
        if (!item.collected && item.r == playerR && item.c == playerC) {
            item.collected = true;
            score += 10;
            itemsCollected++;
            playerSpeed = 1.0f + itemsCollected * 0.2f;
            return true;
        }
    }
    return false;
}

// ========================= ẢNH NHÂN VẬT =========================
// Nhận diện những pixel có màu gần giống nền kem của ảnh gốc.
bool IsConnectedBackground(Color color) {
    int maximum = max((int)color.r, max((int)color.g, (int)color.b));
    int minimum = min((int)color.r, min((int)color.g, (int)color.b));
    return color.r > 220 && color.g > 215 && color.b > 190 &&
           maximum - minimum < 60;
}

// Khai báo trước hàm pixel dự phòng, dùng khi thiếu file ảnh.
void DrawCharacter(int character, float x, float y, float scale);

// Chỉ xóa màu nền kem nối với biên ảnh. Những vùng trắng nằm trong
// nhân vật như mặt Doraemon, mắt, tất và giày vẫn được giữ nguyên.
void RemoveCharacterSheetBackground(Image& image) {
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Color* pixels = (Color*)image.data;
    int width = image.width;
    int height = image.height;
    vector<unsigned char> visited(width * height, 0);
    queue<int> pending;

    auto AddBackgroundPixel = [&](int x, int y) {
        int index = y * width + x;
        if (!visited[index] && IsConnectedBackground(pixels[index])) {
            visited[index] = 1;
            pending.push(index);
        }
    };

    for (int x = 0; x < width; x++) {
        AddBackgroundPixel(x, 0);
        AddBackgroundPixel(x, height - 1);
    }
    for (int y = 0; y < height; y++) {
        AddBackgroundPixel(0, y);
        AddBackgroundPixel(width - 1, y);
    }

    int dx[4] = {-1, 1, 0, 0};
    int dy[4] = {0, 0, -1, 1};
    while (!pending.empty()) {
        int index = pending.front();
        pending.pop();
        int x = index % width;
        int y = index / width;
        pixels[index] = BLANK;

        for (int direction = 0; direction < 4; direction++) {
            int nx = x + dx[direction];
            int ny = y + dy[direction];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                AddBackgroundPixel(nx, ny);
            }
        }
    }
}

// Tìm characters.png ở các vị trí thường dùng rồi tải thành texture.
CharacterSheet LoadCharacterSheet() {
    const char* imagePaths[] = {
        "characters.png",
        "../characters.png",
        "assets/characters.png",
        "../assets/characters.png",
        "../../characters.png"
    };

    CharacterSheet sheet = {};
    sheet.loaded = false;
    for (const char* path : imagePaths) {
        if (!FileExists(path)) continue;
        Image image = LoadImage(path);
        if (image.data == nullptr) continue;

        RemoveCharacterSheetBackground(image);
        sheet.texture = LoadTextureFromImage(image);
        SetTextureFilter(sheet.texture, TEXTURE_FILTER_POINT);
        UnloadImage(image);
        sheet.loaded = true;
        break;
    }
    return sheet;
}

// Vùng cắt được đo từ ảnh gốc 1774x887 mà người dùng cung cấp.
Rectangle GetCharacterSource(const CharacterSheet& sheet, int character) {
    Rectangle originalSources[CHARACTER_COUNT] = {
        {384, 224, 210, 465},   // Nobita
        {878, 175, 338, 514},   // Jaian
        {1226, 230, 218, 459},  // Suneo
        {647, 236, 217, 453},   // Shizuka
        {53, 278, 286, 411},    // Doraemon
        {1498, 207, 206, 482}   // Dekisugi
    };
    float scaleX = sheet.texture.width / 1774.0f;
    float scaleY = sheet.texture.height / 887.0f;
    Rectangle source = originalSources[character];
    source.x *= scaleX;
    source.y *= scaleY;
    source.width *= scaleX;
    source.height *= scaleY;
    return source;
}

// Cắt đúng nhân vật, tự tính tỉ lệ và căn nhân vật vào vùng cần vẽ.
void DrawCharacterFromSheet(const CharacterSheet& sheet, int character,
                            float areaX, float areaY,
                            float areaWidth, float areaHeight) {
    if (!sheet.loaded) return;

    Rectangle source = GetCharacterSource(sheet, character);
    float scale = min(areaWidth / source.width, areaHeight / source.height);
    float drawWidth = source.width * scale;
    float drawHeight = source.height * scale;
    Rectangle destination = {
        areaX + (areaWidth - drawWidth) / 2.0f,
        areaY + areaHeight - drawHeight,
        drawWidth,
        drawHeight
    };
    DrawTexturePro(sheet.texture, source, destination, {0, 0}, 0, WHITE);
}

// Vẽ một hình chữ nhật theo tọa độ pixel trên khung 28x28.
void PixelRect(float originX, float originY, float scale,
               int x, int y, int width, int height, Color color) {
    DrawRectangle(
        (int)(originX + x * scale),
        (int)(originY + y * scale),
        max(1, (int)(width * scale)),
        max(1, (int)(height * scale)),
        color
    );
}

// Vẽ bóng nhỏ dưới chân sprite pixel dự phòng.
void DrawPixelShadow(float x, float y, float scale) {
    Color shadow = {8, 12, 24, 95};
    PixelRect(x, y, scale, 7, 26, 14, 2, shadow);
    PixelRect(x, y, scale, 5, 27, 18, 1, shadow);
}

// Các hàm sau vẽ sprite dự phòng bằng hình chữ nhật nếu cần chỉnh sửa về sau.
void DrawNobitaPixel(float x, float y, float scale) {
    Color outline = {31, 31, 39, 255};
    Color hair = {27, 25, 31, 255};
    Color skin = {255, 205, 157, 255};
    Color skinShade = {231, 166, 124, 255};
    Color shirt = {250, 207, 47, 255};
    Color shirtLight = {255, 230, 83, 255};
    Color shortsColor = {35, 78, 151, 255};

    DrawPixelShadow(x, y, scale);
    PixelRect(x, y, scale, 9, 1, 10, 2, hair);
    PixelRect(x, y, scale, 7, 3, 14, 3, hair);
    PixelRect(x, y, scale, 6, 5, 16, 8, outline);
    PixelRect(x, y, scale, 7, 6, 14, 8, skin);
    PixelRect(x, y, scale, 5, 8, 2, 4, skinShade);
    PixelRect(x, y, scale, 21, 8, 2, 4, skinShade);
    PixelRect(x, y, scale, 7, 5, 4, 2, hair);
    PixelRect(x, y, scale, 10, 3, 2, 3, hair);
    PixelRect(x, y, scale, 17, 4, 3, 2, hair);

    // Kính vuông bo theo phong cách pixel.
    PixelRect(x, y, scale, 7, 7, 6, 1, outline);
    PixelRect(x, y, scale, 7, 8, 1, 5, outline);
    PixelRect(x, y, scale, 12, 8, 1, 5, outline);
    PixelRect(x, y, scale, 7, 12, 6, 1, outline);
    PixelRect(x, y, scale, 15, 7, 6, 1, outline);
    PixelRect(x, y, scale, 15, 8, 1, 5, outline);
    PixelRect(x, y, scale, 20, 8, 1, 5, outline);
    PixelRect(x, y, scale, 15, 12, 6, 1, outline);
    PixelRect(x, y, scale, 13, 9, 2, 1, outline);
    PixelRect(x, y, scale, 10, 9, 1, 2, outline);
    PixelRect(x, y, scale, 17, 9, 1, 2, outline);
    PixelRect(x, y, scale, 13, 11, 2, 1, skinShade);
    PixelRect(x, y, scale, 12, 13, 4, 1, {143, 63, 66, 255});

    PixelRect(x, y, scale, 8, 15, 12, 1, outline);
    PixelRect(x, y, scale, 7, 16, 14, 7, outline);
    PixelRect(x, y, scale, 8, 16, 12, 6, shirt);
    PixelRect(x, y, scale, 9, 16, 10, 2, shirtLight);
    PixelRect(x, y, scale, 12, 16, 4, 2, WHITE);
    PixelRect(x, y, scale, 4, 17, 3, 6, outline);
    PixelRect(x, y, scale, 5, 18, 2, 4, skin);
    PixelRect(x, y, scale, 21, 17, 3, 6, outline);
    PixelRect(x, y, scale, 21, 18, 2, 4, skin);
    PixelRect(x, y, scale, 8, 22, 12, 4, shortsColor);
    PixelRect(x, y, scale, 13, 23, 2, 3, outline);
    PixelRect(x, y, scale, 8, 26, 5, 2, WHITE);
    PixelRect(x, y, scale, 15, 26, 5, 2, WHITE);
}

// Vẽ Jaian dạng pixel.
void DrawJaianPixel(float x, float y, float scale) {
    Color outline = {37, 29, 32, 255};
    Color hair = {24, 22, 25, 255};
    Color skin = {219, 158, 104, 255};
    Color skinShade = {188, 119, 78, 255};
    Color shirt = {235, 101, 31, 255};
    Color shirtLight = {255, 144, 47, 255};
    Color pants = {48, 39, 72, 255};

    DrawPixelShadow(x, y, scale);
    PixelRect(x, y, scale, 7, 1, 14, 2, hair);
    PixelRect(x, y, scale, 5, 3, 18, 4, hair);
    PixelRect(x, y, scale, 4, 6, 20, 8, outline);
    PixelRect(x, y, scale, 5, 6, 18, 9, skin);
    PixelRect(x, y, scale, 5, 5, 18, 3, hair);
    PixelRect(x, y, scale, 3, 8, 2, 5, skinShade);
    PixelRect(x, y, scale, 23, 8, 2, 5, skinShade);
    PixelRect(x, y, scale, 8, 8, 5, 2, hair);
    PixelRect(x, y, scale, 16, 8, 5, 2, hair);
    PixelRect(x, y, scale, 10, 10, 2, 2, BLACK);
    PixelRect(x, y, scale, 17, 10, 2, 2, BLACK);
    PixelRect(x, y, scale, 13, 11, 3, 2, skinShade);
    PixelRect(x, y, scale, 9, 13, 11, 2, {104, 42, 47, 255});
    PixelRect(x, y, scale, 11, 13, 7, 1, WHITE);

    PixelRect(x, y, scale, 4, 15, 20, 9, outline);
    PixelRect(x, y, scale, 5, 16, 18, 7, shirt);
    PixelRect(x, y, scale, 6, 16, 16, 2, shirtLight);
    PixelRect(x, y, scale, 10, 16, 8, 1, {255, 192, 61, 255});
    PixelRect(x, y, scale, 1, 17, 4, 7, outline);
    PixelRect(x, y, scale, 2, 18, 3, 5, skin);
    PixelRect(x, y, scale, 23, 17, 4, 7, outline);
    PixelRect(x, y, scale, 23, 18, 3, 5, skin);
    PixelRect(x, y, scale, 6, 23, 16, 3, pants);
    PixelRect(x, y, scale, 7, 26, 6, 2, {54, 53, 62, 255});
    PixelRect(x, y, scale, 15, 26, 6, 2, {54, 53, 62, 255});
}

// Vẽ Suneo dạng pixel.
void DrawSuneoPixel(float x, float y, float scale) {
    Color outline = {31, 31, 39, 255};
    Color hair = {30, 29, 34, 255};
    Color skin = {255, 205, 157, 255};
    Color skinShade = {226, 158, 118, 255};
    Color shirt = {67, 172, 102, 255};
    Color shirtLight = {102, 205, 128, 255};
    Color pants = {47, 69, 126, 255};

    DrawPixelShadow(x, y, scale);
    PixelRect(x, y, scale, 6, 3, 4, 3, hair);
    PixelRect(x, y, scale, 8, 1, 5, 4, hair);
    PixelRect(x, y, scale, 12, 0, 5, 5, hair);
    PixelRect(x, y, scale, 16, 2, 5, 4, hair);
    PixelRect(x, y, scale, 6, 5, 15, 2, hair);
    PixelRect(x, y, scale, 7, 6, 15, 8, outline);
    PixelRect(x, y, scale, 8, 6, 13, 9, skin);
    PixelRect(x, y, scale, 6, 8, 2, 4, skinShade);
    PixelRect(x, y, scale, 21, 8, 3, 4, skin);
    PixelRect(x, y, scale, 9, 8, 4, 1, hair);
    PixelRect(x, y, scale, 16, 8, 4, 1, hair);
    PixelRect(x, y, scale, 11, 9, 1, 2, BLACK);
    PixelRect(x, y, scale, 17, 9, 1, 2, BLACK);
    PixelRect(x, y, scale, 20, 11, 3, 1, skinShade);
    PixelRect(x, y, scale, 13, 13, 5, 1, {137, 60, 63, 255});

    PixelRect(x, y, scale, 8, 15, 13, 1, outline);
    PixelRect(x, y, scale, 7, 16, 15, 8, outline);
    PixelRect(x, y, scale, 8, 16, 13, 7, shirt);
    PixelRect(x, y, scale, 9, 16, 11, 2, shirtLight);
    PixelRect(x, y, scale, 12, 16, 4, 2, WHITE);
    PixelRect(x, y, scale, 4, 17, 3, 6, outline);
    PixelRect(x, y, scale, 5, 18, 2, 4, skin);
    PixelRect(x, y, scale, 22, 17, 3, 6, outline);
    PixelRect(x, y, scale, 22, 18, 2, 4, skin);
    PixelRect(x, y, scale, 8, 23, 13, 3, pants);
    PixelRect(x, y, scale, 12, 24, 2, 2, outline);
    PixelRect(x, y, scale, 8, 26, 5, 2, WHITE);
    PixelRect(x, y, scale, 15, 26, 5, 2, WHITE);
}

// Vẽ Shizuka dạng pixel.
void DrawShizukaPixel(float x, float y, float scale) {
    Color outline = {43, 31, 35, 255};
    Color hair = {91, 55, 38, 255};
    Color hairLight = {129, 78, 48, 255};
    Color skin = {255, 211, 170, 255};
    Color skinShade = {232, 169, 132, 255};
    Color dress = {238, 103, 153, 255};
    Color dressLight = {255, 153, 190, 255};

    DrawPixelShadow(x, y, scale);
    PixelRect(x, y, scale, 8, 1, 12, 2, hair);
    PixelRect(x, y, scale, 6, 3, 16, 4, hair);
    PixelRect(x, y, scale, 5, 6, 18, 8, outline);
    PixelRect(x, y, scale, 7, 6, 14, 9, skin);
    PixelRect(x, y, scale, 5, 7, 3, 9, hair);
    PixelRect(x, y, scale, 20, 7, 3, 9, hair);
    PixelRect(x, y, scale, 4, 11, 4, 4, hairLight);
    PixelRect(x, y, scale, 20, 11, 4, 4, hairLight);
    PixelRect(x, y, scale, 8, 5, 12, 2, hair);
    PixelRect(x, y, scale, 10, 9, 1, 2, BLACK);
    PixelRect(x, y, scale, 17, 9, 1, 2, BLACK);
    PixelRect(x, y, scale, 13, 11, 2, 1, skinShade);
    PixelRect(x, y, scale, 12, 13, 4, 1, {151, 65, 83, 255});
    PixelRect(x, y, scale, 8, 15, 12, 1, outline);
    PixelRect(x, y, scale, 7, 16, 14, 7, dress);
    PixelRect(x, y, scale, 9, 16, 10, 2, dressLight);
    PixelRect(x, y, scale, 12, 16, 4, 3, WHITE);
    PixelRect(x, y, scale, 5, 17, 3, 6, skin);
    PixelRect(x, y, scale, 20, 17, 3, 6, skin);
    PixelRect(x, y, scale, 6, 22, 16, 3, dress);
    PixelRect(x, y, scale, 8, 25, 4, 2, skin);
    PixelRect(x, y, scale, 16, 25, 4, 2, skin);
    PixelRect(x, y, scale, 7, 27, 6, 1, {197, 46, 86, 255});
    PixelRect(x, y, scale, 15, 27, 6, 1, {197, 46, 86, 255});
}

// Vẽ Doraemon dạng pixel.
void DrawDoraemonPixel(float x, float y, float scale) {
    Color outline = {22, 43, 58, 255};
    Color blue = {31, 151, 218, 255};
    Color blueLight = {73, 187, 234, 255};

    DrawPixelShadow(x, y, scale);
    PixelRect(x, y, scale, 9, 1, 10, 1, outline);
    PixelRect(x, y, scale, 6, 2, 16, 2, outline);
    PixelRect(x, y, scale, 4, 4, 20, 4, outline);
    PixelRect(x, y, scale, 3, 7, 22, 9, outline);
    PixelRect(x, y, scale, 5, 3, 18, 12, blue);
    PixelRect(x, y, scale, 7, 4, 14, 2, blueLight);
    PixelRect(x, y, scale, 6, 6, 16, 9, WHITE);
    PixelRect(x, y, scale, 9, 3, 5, 5, WHITE);
    PixelRect(x, y, scale, 14, 3, 5, 5, WHITE);
    PixelRect(x, y, scale, 12, 6, 1, 2, BLACK);
    PixelRect(x, y, scale, 16, 6, 1, 2, BLACK);
    PixelRect(x, y, scale, 13, 8, 3, 3, RED);
    PixelRect(x, y, scale, 14, 10, 1, 4, outline);
    PixelRect(x, y, scale, 8, 10, 4, 1, outline);
    PixelRect(x, y, scale, 17, 10, 4, 1, outline);
    PixelRect(x, y, scale, 7, 12, 5, 1, outline);
    PixelRect(x, y, scale, 17, 12, 5, 1, outline);
    PixelRect(x, y, scale, 10, 14, 9, 1, {141, 45, 50, 255});
    PixelRect(x, y, scale, 5, 15, 18, 10, outline);
    PixelRect(x, y, scale, 6, 16, 16, 8, blue);
    PixelRect(x, y, scale, 6, 16, 16, 2, RED);
    PixelRect(x, y, scale, 8, 18, 12, 6, WHITE);
    PixelRect(x, y, scale, 13, 17, 3, 3, GOLD);
    PixelRect(x, y, scale, 14, 19, 1, 2, outline);
    PixelRect(x, y, scale, 3, 17, 3, 6, blue);
    PixelRect(x, y, scale, 22, 17, 3, 6, blue);
    PixelRect(x, y, scale, 6, 24, 7, 3, WHITE);
    PixelRect(x, y, scale, 15, 24, 7, 3, WHITE);
    PixelRect(x, y, scale, 13, 23, 2, 4, outline);
}

// Vẽ Dekisugi dạng pixel.
void DrawDekisugiPixel(float x, float y, float scale) {
    Color outline = {30, 32, 39, 255};
    Color hair = {35, 31, 34, 255};
    Color hairLight = {65, 57, 57, 255};
    Color skin = {255, 207, 161, 255};
    Color skinShade = {230, 163, 122, 255};
    Color shirt = {68, 148, 214, 255};
    Color shirtLight = {112, 190, 236, 255};
    Color pants = {39, 64, 116, 255};

    DrawPixelShadow(x, y, scale);
    PixelRect(x, y, scale, 8, 1, 12, 2, hair);
    PixelRect(x, y, scale, 6, 3, 16, 4, hair);
    PixelRect(x, y, scale, 6, 6, 16, 8, outline);
    PixelRect(x, y, scale, 7, 6, 14, 9, skin);
    PixelRect(x, y, scale, 7, 5, 7, 2, hair);
    PixelRect(x, y, scale, 15, 3, 2, 4, hairLight);
    PixelRect(x, y, scale, 17, 5, 4, 2, hair);
    PixelRect(x, y, scale, 5, 8, 2, 4, skinShade);
    PixelRect(x, y, scale, 21, 8, 2, 4, skinShade);
    PixelRect(x, y, scale, 9, 8, 4, 1, hair);
    PixelRect(x, y, scale, 16, 8, 4, 1, hair);
    PixelRect(x, y, scale, 11, 9, 1, 2, BLACK);
    PixelRect(x, y, scale, 17, 9, 1, 2, BLACK);
    PixelRect(x, y, scale, 13, 11, 2, 1, skinShade);
    PixelRect(x, y, scale, 12, 13, 5, 1, {135, 61, 65, 255});
    PixelRect(x, y, scale, 8, 15, 12, 1, outline);
    PixelRect(x, y, scale, 7, 16, 14, 7, outline);
    PixelRect(x, y, scale, 8, 16, 12, 6, shirt);
    PixelRect(x, y, scale, 9, 16, 10, 2, shirtLight);
    PixelRect(x, y, scale, 11, 16, 6, 3, WHITE);
    PixelRect(x, y, scale, 13, 17, 2, 3, {43, 83, 139, 255});
    PixelRect(x, y, scale, 4, 17, 3, 6, outline);
    PixelRect(x, y, scale, 5, 18, 2, 4, skin);
    PixelRect(x, y, scale, 21, 17, 3, 6, outline);
    PixelRect(x, y, scale, 21, 18, 2, 4, skin);
    PixelRect(x, y, scale, 8, 22, 12, 4, pants);
    PixelRect(x, y, scale, 13, 23, 2, 3, outline);
    PixelRect(x, y, scale, 8, 26, 5, 2, WHITE);
    PixelRect(x, y, scale, 15, 26, 5, 2, WHITE);
}

// Chọn đúng hàm vẽ pixel dựa trên mã nhân vật.
void DrawCharacter(int character, float x, float y, float scale) {
    if (character == NOBITA) DrawNobitaPixel(x, y, scale);
    else if (character == JAIAN) DrawJaianPixel(x, y, scale);
    else if (character == SUNEO) DrawSuneoPixel(x, y, scale);
    else if (character == SHIZUKA) DrawShizukaPixel(x, y, scale);
    else if (character == DORAEMON) DrawDoraemonPixel(x, y, scale);
    else DrawDekisugiPixel(x, y, scale);
}

// ========================= VẼ ĐỐI TƯỢNG TRONG GAME =========================
// Vẽ quỷ có sừng, cánh, nanh, đuôi và cây đinh ba.
void DrawHornedDemon(int r, int c) {
    int x = c * TILE;
    int y = r * TILE + UI_HEIGHT;
    Color outline = {45, 13, 34, 255};
    Color body = {137, 31, 73, 255};
    Color bodyLight = {207, 52, 91, 255};
    Color bodyDark = {83, 24, 72, 255};
    Color horn = {255, 224, 159, 255};
    Color trident = {244, 181, 42, 255};
    Color tridentLight = {255, 231, 107, 255};

    // Bóng và đôi cánh nằm phía sau cơ thể.
    DrawRectangle(x + 5, y + 26, 19, 2, Fade(BLACK, 0.32f));
    DrawTriangle({(float)x + 7, (float)y + 14},
                 {(float)x - 3, (float)y + 8},
                 {(float)x + 3, (float)y + 22}, bodyDark);
    DrawTriangle({(float)x + 21, (float)y + 14},
                 {(float)x + 29, (float)y + 8},
                 {(float)x + 25, (float)y + 21}, bodyDark);
    DrawLine(x + 2, y + 11, x + 5, y + 18, Fade(bodyLight, 0.55f));

    // Hai chiếc sừng lớn màu ngà.
    DrawTriangle({(float)x + 8, (float)y + 8},
                 {(float)x + 3, (float)y - 2},
                 {(float)x + 12, (float)y + 5}, outline);
    DrawTriangle({(float)x + 8, (float)y + 7},
                 {(float)x + 4, (float)y - 1},
                 {(float)x + 11, (float)y + 5}, horn);
    DrawTriangle({(float)x + 20, (float)y + 8},
                 {(float)x + 25, (float)y - 2},
                 {(float)x + 16, (float)y + 5}, outline);
    DrawTriangle({(float)x + 20, (float)y + 7},
                 {(float)x + 24, (float)y - 1},
                 {(float)x + 17, (float)y + 5}, horn);

    // Đầu, tai, mặt dữ và hai chiếc nanh.
    DrawCircle(x + 14, y + 10, 10, outline);
    DrawCircle(x + 14, y + 10, 8, body);
    DrawCircle(x + 5, y + 11, 3, bodyLight);
    DrawCircle(x + 23, y + 11, 3, bodyLight);
    DrawTriangle({(float)x + 7, (float)y + 8},
                 {(float)x + 13, (float)y + 9},
                 {(float)x + 8, (float)y + 12}, YELLOW);
    DrawTriangle({(float)x + 21, (float)y + 8},
                 {(float)x + 15, (float)y + 9},
                 {(float)x + 20, (float)y + 12}, YELLOW);
    DrawCircle(x + 10, y + 10, 1, BLACK);
    DrawCircle(x + 18, y + 10, 1, BLACK);
    DrawRectangle(x + 12, y + 14, 5, 2, outline);
    DrawTriangle({(float)x + 11, (float)y + 15},
                 {(float)x + 13, (float)y + 19},
                 {(float)x + 14, (float)y + 15}, WHITE);
    DrawTriangle({(float)x + 17, (float)y + 15},
                 {(float)x + 15, (float)y + 19},
                 {(float)x + 14, (float)y + 15}, WHITE);

    // Thân, tay, chân và chiếc đuôi nhọn.
    DrawRectangle(x + 5, y + 16, 18, 9, outline);
    DrawRectangle(x + 7, y + 16, 14, 8, bodyLight);
    DrawRectangle(x + 3, y + 17, 4, 7, body);
    DrawRectangle(x + 21, y + 17, 4, 6, body);
    DrawCircle(x + 24, y + 20, 2, bodyLight);
    DrawRectangle(x + 7, y + 24, 6, 4, bodyDark);
    DrawRectangle(x + 16, y + 24, 6, 4, bodyDark);
    DrawLine(x + 5, y + 23, x + 0, y + 26, bodyDark);
    DrawTriangle({(float)x - 2, (float)y + 24},
                 {(float)x + 2, (float)y + 26},
                 {(float)x - 1, (float)y + 29}, bodyLight);

    // Cây đinh ba: cán dài, ba mũi nhọn và tay đang cầm cán.
    DrawRectangle(x + 26, y + 4, 3, 24, outline);
    DrawRectangle(x + 27, y + 5, 1, 23, trident);
    DrawRectangle(x + 21, y + 4, 13, 3, outline);
    DrawRectangle(x + 22, y + 4, 11, 1, tridentLight);
    DrawTriangle({(float)x + 20, (float)y + 5},
                 {(float)x + 23, (float)y - 2},
                 {(float)x + 25, (float)y + 5}, trident);
    DrawTriangle({(float)x + 25, (float)y + 5},
                 {(float)x + 27, (float)y - 4},
                 {(float)x + 30, (float)y + 5}, tridentLight);
    DrawTriangle({(float)x + 30, (float)y + 5},
                 {(float)x + 33, (float)y - 2},
                 {(float)x + 35, (float)y + 5}, trident);
    DrawCircle(x + 27, y + 20, 2, bodyLight);
}

// Vẽ vật phẩm phát sáng và hiệu ứng nhấp nháy theo thời gian.
void DrawItemPixel(const Item& item, int index, Color accent) {
    int x = item.c * TILE;
    int y = item.r * TILE + UI_HEIGHT;
    Color colors[ITEM_COUNT] = {GOLD, SKYBLUE, LIME, PINK, ORANGE};
    float pulse = 1.0f + 0.12f * sinf((float)GetTime() * 5.0f + index);
    DrawCircle(x + 14, y + 14, 10 * pulse, Fade(accent, 0.20f));
    DrawCircle(x + 14, y + 14, 6 * pulse, colors[index]);
    DrawTriangle({(float)x + 14, (float)y + 7},
                 {(float)x + 10, (float)y + 15},
                 {(float)x + 14, (float)y + 14}, WHITE);
    DrawTriangle({(float)x + 14, (float)y + 14},
                 {(float)x + 18, (float)y + 13},
                 {(float)x + 12, (float)y + 22}, WHITE);
}

// Vẽ cổng thoát với hiệu ứng co giãn nhẹ.
void DrawExit(int r, int c, Color accent) {
    int x = c * TILE + TILE / 2;
    int y = r * TILE + UI_HEIGHT + TILE / 2;
    float pulse = 1.0f + 0.10f * sinf((float)GetTime() * 4.0f);
    DrawCircle(x, y, 13 * pulse, Fade(accent, 0.20f));
    DrawCircle(x, y, 10, accent);
    DrawCircle(x, y, 6, Fade(WHITE, 0.75f));
    DrawCircle(x, y, 3, accent);
}

// Vẽ toàn bộ tường, sàn, đường viền và chi tiết trang trí của map.
void DrawMaze(const vector<string>& maze, const MapTheme& theme) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int x = c * TILE;
            int y = r * TILE + UI_HEIGHT;
            if (maze[r][c] == '#') {
                DrawRectangle(x, y + 3, TILE, TILE - 3, theme.wallShadow);
                DrawRectangle(x + 1, y + 1, TILE - 2, TILE - 5, theme.wall);
                DrawRectangle(x + 3, y + 3, TILE - 6, 5, theme.wallTop);
                if ((r + c) % 2 == 0) {
                    DrawLine(x + 2, y + 15, x + TILE - 3, y + 15,
                             Fade(theme.wallTop, 0.55f));
                    DrawLine(x + 14, y + 15, x + 14, y + TILE - 5,
                             Fade(theme.wallShadow, 0.50f));
                } else {
                    DrawLine(x + 2, y + 18, x + TILE - 3, y + 18,
                             Fade(theme.wallTop, 0.45f));
                }
                if ((r * COLS + c) % 47 == 0) {
                    DrawCircle(x + 14, y + 13, 5, Fade(ORANGE, 0.20f));
                    DrawCircle(x + 14, y + 13, 2, GOLD);
                }
            } else {
                Color floorColor = ((r + c) % 2 == 0) ? theme.floorA : theme.floorB;
                DrawRectangle(x, y, TILE, TILE, floorColor);
                DrawRectangleLines(x, y, TILE, TILE, Fade(theme.wall, 0.12f));
                if ((r * COLS + c) % 13 == 0) {
                    DrawCircle(x + 6, y + 6, 2, Fade(theme.accent, 0.22f));
                }
                if ((r * 7 + c * 3) % 29 == 0) {
                    DrawLine(x + 18, y + 20, x + 23, y + 17,
                             Fade(theme.wall, 0.18f));
                }
            }
        }
    }
}

// ========================= GIAO DIỆN =========================
// Vẽ nền chuyển màu, hạt sáng, tiêu đề và phụ đề của menu.
void DrawBackground(Font font, const char* subtitle) {
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {20, 26, 52, 255}, {8, 12, 27, 255});
    for (int i = 0; i < 28; i++) {
        float x = (float)((i * 97 + (int)(GetTime() * 12)) % SCREEN_W);
        float y = (float)((i * 53) % SCREEN_H);
        DrawCircle((int)x, (int)y, 2 + i % 3, Fade(SKYBLUE, 0.18f));
    }
    DrawVN(font, u8"MAZE RUNNER", SCREEN_W / 2.0f, 55, 54,
           {255, 221, 91, 255}, true);
    DrawVN(font, subtitle, SCREEN_W / 2.0f, 118, 23, LIGHTGRAY, true);
}

// Vẽ nút thường; trả về true khi người dùng nhấp chuột trái vào nút.
bool DrawButton(Font font, Rectangle rect, const char* text,
                Color normal, Color hover, bool enabled = true) {
    bool pointed = enabled && CheckCollisionPointRec(GetMousePosition(), rect);
    Color buttonColor = enabled ? (pointed ? hover : normal) : Fade(normal, 0.35f);
    DrawRectangleRounded({rect.x + 4, rect.y + 5, rect.width, rect.height},
                         0.22f, 8, Fade(BLACK, 0.45f));
    DrawRectangleRounded(rect, 0.22f, 8, buttonColor);
    DrawRectangleLinesEx(rect, 2, Fade(WHITE, pointed ? 0.80f : 0.25f));
    DrawVN(font, text, rect.x + rect.width / 2.0f,
           rect.y + rect.height / 2.0f - 12, 23,
           enabled ? WHITE : GRAY, true);
    return pointed && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Vẽ nút menu chính có bóng, gradient, số thứ tự và hiệu ứng hover.
bool DrawMenuButton(Font font, Rectangle rect, const char* text,
                    const char* number, Color accent) {
    bool pointed = CheckCollisionPointRec(GetMousePosition(), rect);
    float hoverShift = pointed ? 5.0f : 0.0f;
    Rectangle moved = {rect.x + hoverShift, rect.y, rect.width, rect.height};
    float pulse = 0.12f + 0.05f * sinf((float)GetTime() * 4.0f);

    if (pointed) {
        DrawRectangleRounded({moved.x - 8, moved.y - 7,
                              moved.width + 16, moved.height + 14},
                             0.18f, 8, Fade(accent, 0.18f + pulse));
    }
    DrawRectangleRounded({moved.x + 7, moved.y + 8,
                          moved.width, moved.height},
                         0.18f, 8, Fade(BLACK, 0.58f));
    DrawRectangleRounded(moved, 0.18f, 8,
                         pointed ? Color{34, 42, 70, 255}
                                 : Color{25, 32, 56, 255});

    Rectangle inner = {moved.x + 6, moved.y + 6,
                       moved.width - 12, moved.height - 12};
    DrawRectangleGradientH((int)inner.x, (int)inner.y,
                           (int)inner.width, (int)inner.height,
                           Fade(accent, pointed ? 0.70f : 0.45f),
                           {24, 31, 54, 255});
    DrawRectangleRoundedLines(moved, 0.18f, 8,
                              pointed ? Fade(accent, 0.95f)
                                      : Fade(WHITE, 0.25f));
    DrawRectangle((int)moved.x + 12, (int)moved.y + 12,
                  5, (int)moved.height - 24, accent);

    DrawCircle((int)moved.x + 43,
               (int)(moved.y + moved.height / 2), 20,
               Fade(BLACK, 0.35f));
    DrawCircleLines((int)moved.x + 43,
                    (int)(moved.y + moved.height / 2), 20,
                    Fade(accent, 0.90f));
    DrawVN(font, number, moved.x + 43,
           moved.y + moved.height / 2 - 11, 19, WHITE, true);
    DrawVN(font, text, moved.x + 82,
           moved.y + moved.height / 2 - 14, 26, WHITE);

    Color arrowColor = pointed ? accent : Fade(WHITE, 0.35f);
    DrawLine((int)(moved.x + moved.width - 42),
             (int)(moved.y + 23),
             (int)(moved.x + moved.width - 28),
             (int)(moved.y + moved.height / 2), arrowColor);
    DrawLine((int)(moved.x + moved.width - 28),
             (int)(moved.y + moved.height / 2),
             (int)(moved.x + moved.width - 42),
             (int)(moved.y + moved.height - 23), arrowColor);
    return pointed && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Vẽ menu chính và xử lý ba lựa chọn: bắt đầu, hướng dẫn, thoát.
void DrawMainMenu(Font font, ScreenState& screen, bool& quitRequested) {
    DrawBackground(font, u8"Cuộc chạy trốn khỏi mê cung của quỷ");
    Rectangle menuPanel = {229, 202, 410, 337};
    DrawRectangleRounded({menuPanel.x + 10, menuPanel.y + 12,
                          menuPanel.width, menuPanel.height},
                         0.08f, 8, Fade(BLACK, 0.42f));
    DrawRectangleRounded(menuPanel, 0.08f, 8, {17, 23, 43, 225});
    DrawRectangleRoundedLines(menuPanel, 0.08f, 8, Fade(SKYBLUE, 0.25f));
    DrawVN(font, u8"CHỌN CHỨC NĂNG", SCREEN_W / 2.0f,
           220, 18, Fade(WHITE, 0.65f), true);
    DrawLine(275, 250, 593, 250, Fade(SKYBLUE, 0.25f));

    Rectangle startButton = {259, 270, 350, 68};
    Rectangle guideButton = {259, 360, 350, 68};
    Rectangle exitButton = {259, 450, 350, 68};

    if (DrawMenuButton(font, startButton, u8"BẮT ĐẦU", "01",
                       {46, 190, 147, 255})) {
        screen = CHARACTER_SCREEN;
    }
    if (DrawMenuButton(font, guideButton, u8"HƯỚNG DẪN", "02",
                       {102, 141, 244, 255})) {
        screen = GUIDE_SCREEN;
    }
    if (DrawMenuButton(font, exitButton, u8"THOÁT", "03",
                       {232, 79, 104, 255})) {
        quitRequested = true;
    }

}

// Hiển thị hướng dẫn phím bấm và nút quay lại menu.
void DrawGuide(Font font, ScreenState& screen) {
    DrawBackground(font, u8"Hướng dẫn chơi");
    Rectangle panel = {115, 180, 638, 340};
    DrawRectangleRounded(panel, 0.08f, 8, {28, 37, 69, 245});
    DrawRectangleLinesEx(panel, 2, Fade(SKYBLUE, 0.40f));

    DrawVN(font, u8"WASD hoặc phím mũi tên: Di chuyển", 160, 220, 23, WHITE);
    DrawVN(font, u8"Ăn vật phẩm: Tăng 0,2 tốc độ", 160, 266, 23, WHITE);
    DrawVN(font, u8"Đi đến cổng sáng: Chiến thắng", 160, 312, 23, WHITE);
    DrawVN(font, u8"Tránh quỷ có sừng đang đuổi theo", 160, 358, 23, WHITE);
    DrawVN(font, u8"R: Bản đồ ngẫu nhiên   N: Bản đồ tiếp theo", 160, 404, 21, LIGHTGRAY);
    DrawVN(font, u8"M: Trở về menu", 160, 446, 21, LIGHTGRAY);

    if (DrawButton(font, {309, 552, 250, 58}, u8"QUAY LẠI",
                   {54, 92, 151, 255}, {71, 132, 201, 255})) {
        screen = MAIN_MENU;
    }
}

// Hiển thị sáu thẻ nhân vật và ghi nhận nhân vật được người chơi chọn.
void DrawCharacterSelection(Font font, const CharacterSheet& characterSheet,
                            int& selectedCharacter,
                            ScreenState& screen, bool& startGameRequested) {
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {24, 31, 61, 255}, {9, 14, 30, 255});
    DrawVN(font, u8"CHỌN NHÂN VẬT", SCREEN_W / 2.0f, 25, 39,
           {255, 221, 91, 255}, true);
    DrawVN(font, u8"Nhấp vào nhân vật mà bạn muốn sử dụng",
           SCREEN_W / 2.0f, 76, 20, LIGHTGRAY, true);

    for (int i = 0; i < CHARACTER_COUNT; i++) {
        int col = i % 3;
        int row = i / 3;
        Rectangle card = {(float)(91 + col * 230), (float)(120 + row * 205), 180, 174};
        bool hover = CheckCollisionPointRec(GetMousePosition(), card);
        bool selected = selectedCharacter == i;
        Color characterAccent = CHARACTER_COLORS[i];
        Color cardColor = selected ? Fade(characterAccent, 0.42f)
                                   : (hover ? Color{42, 54, 92, 255}
                                            : Color{29, 38, 70, 255});
        DrawRectangleRounded(card, 0.10f, 8, cardColor);
        for (int py = 0; py < 5; py++) {
            for (int px = 0; px < 6; px++) {
                if ((px + py) % 2 == 0) {
                    DrawRectangle((int)card.x + 8 + px * 27,
                                  (int)card.y + 8 + py * 24,
                                  13, 12, Fade(characterAccent, 0.07f));
                }
            }
        }
        DrawRectangleLinesEx(card, selected ? 4 : 2,
                             selected ? characterAccent : Fade(WHITE, 0.25f));
        DrawCharacterFromSheet(characterSheet, i,
                               card.x + 25, card.y + 10, 130, 120);
        DrawVN(font, CHARACTER_NAMES[i], card.x + card.width / 2.0f,
               card.y + 138, 21, selected ? characterAccent : WHITE, true);
        if (selected) {
            DrawVN(font, u8"ĐÃ CHỌN", card.x + card.width / 2.0f,
                   card.y + 160, 12, WHITE, true);
        }

        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedCharacter = i;
        }
    }

    if (DrawButton(font, {115, 552, 230, 58}, u8"QUAY LẠI",
                   {67, 72, 104, 255}, {91, 101, 142, 255})) {
        screen = MAIN_MENU;
    }
    if (DrawButton(font, {523, 552, 230, 58}, u8"VÀO MÊ CUNG",
                   {35, 133, 90, 255}, {49, 181, 118, 255})) {
        startGameRequested = true;
        screen = PLAYING_SCREEN;
    }
}

// Vẽ thanh thông tin: map, điểm, vật phẩm, tốc độ và phím điều khiển.
void DrawGameHud(Font font, const MapTheme& theme, int currentMap,
                 int score, int itemsCollected, float playerSpeed,
                 int selectedCharacter, float boostMessageTimer) {
    DrawRectangle(0, 0, SCREEN_W, UI_HEIGHT, theme.background);
    DrawRectangle(0, UI_HEIGHT - 5, SCREEN_W, 5, theme.accent);
    DrawVN(font, u8"MAZE RUNNER", 15, 10, 25, WHITE);
    DrawVN(font, CHARACTER_NAMES[selectedCharacter], 17, 47, 19, theme.accent);
    DrawVN(font, TextFormat(u8"Bản đồ %d/%d: %s", currentMap + 1,
                           MAP_COUNT, MAP_NAMES[currentMap]),
           245, 8, 20, LIGHTGRAY);
    DrawVN(font, TextFormat(u8"Điểm: %d   Vật phẩm: %d/%d   Tốc độ: x%.1f",
                           score, itemsCollected, ITEM_COUNT, playerSpeed),
           245, 39, 20, theme.accent);
    DrawVN(font, u8"R: Ngẫu nhiên   N: Tiếp   M: Menu", 540, 66, 15,
           Fade(WHITE, 0.70f));
    if (boostMessageTimer > 0.0f) {
        DrawVN(font, u8"+0,2 TỐC ĐỘ", 710, 39, 17, GREEN);
    }
}

// Hiện thông báo rõ ràng nếu chương trình không tìm thấy characters.png.
void DrawMissingCharacterImage(Font font) {
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {53, 20, 31, 255}, {16, 10, 22, 255});
    DrawVN(font, u8"KHÔNG TÌM THẤY ẢNH NHÂN VẬT",
           SCREEN_W / 2.0f, 150, 36, RED, true);
    DrawVN(font, u8"Hãy đặt file characters.png cạnh MazeRunner_Final.cpp",
           SCREEN_W / 2.0f, 235, 23, WHITE, true);
    DrawVN(font, u8"hoặc đặt cạnh MazeRunner_Final.exe.",
           SCREEN_W / 2.0f, 275, 23, WHITE, true);
    DrawVN(font, u8"Cấu trúc đề xuất:", SCREEN_W / 2.0f,
           350, 21, {255, 209, 84, 255}, true);
    DrawVN(font, u8"Cà Rốt/characters.png", SCREEN_W / 2.0f,
           390, 21, LIGHTGRAY, true);
    DrawVN(font, u8"Cà Rốt/MazeRunner_Final.cpp", SCREEN_W / 2.0f,
           425, 21, LIGHTGRAY, true);
    DrawVN(font, u8"Cà Rốt/output/MazeRunner_Final.exe", SCREEN_W / 2.0f,
           460, 21, LIGHTGRAY, true);
    DrawVN(font, u8"Game không còn sử dụng hình nhân vật dự phòng cũ.",
           SCREEN_W / 2.0f, 535, 19, Fade(WHITE, 0.70f), true);
}

// ========================= HÀM MAIN =========================
// Khởi tạo tài nguyên, cập nhật luật chơi, vẽ màn hình và giải phóng bộ nhớ.
int main() {
    // Tạo cửa sổ và giới hạn game ở 60 khung hình mỗi giây.
    InitWindow(SCREEN_W, SCREEN_H, "Maze Runner");
    SetTargetFPS(60);

    // Tải font, ảnh nhân vật và tạo danh sách bản đồ một lần khi mở game.
    bool loadedCustomFont = false;
    Font vietnameseFont = LoadVietnameseFont(loadedCustomFont);
    CharacterSheet characterSheet = LoadCharacterSheet();
    vector<vector<string>> mazeList = BuildMazeList();

    // Trạng thái điều khiển việc chuyển giữa các màn hình.
    ScreenState screen = MAIN_MENU;
    bool quitRequested = false;
    bool startGameRequested = false;
    int selectedCharacter = NOBITA;

    // Các biến lưu toàn bộ dữ liệu của ván chơi hiện tại.
    vector<string> maze;
    int playerR = 0, playerC = 0, exitR = 0, exitC = 0;
    Enemy enemy = {0, 0, 0.0f};
    vector<Item> items;
    int score = 0;
    int itemsCollected = 0;
    int currentMap = 0;
    bool win = false;
    bool gameOver = false;
    float playerSpeed = 1.0f;
    float moveCharge = 0.0f;
    float boostMessageTimer = 0.0f;

    // Vòng lặp chính chạy cho tới khi đóng cửa sổ hoặc nhấn nút Thoát.
    while (!WindowShouldClose() && !quitRequested) {
        // Không cho vào game nếu thiếu ảnh nhân vật; thay bằng màn hình báo lỗi.
        if (!characterSheet.loaded) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawMissingCharacterImage(vietnameseFont);
            EndDrawing();
            continue;
        }

        // Người chơi vừa nhấn "Vào mê cung" nên cần tạo một ván mới.
        if (startGameRequested) {
            ResetGame(mazeList, maze, playerR, playerC, exitR, exitC,
                      enemy, items, score, itemsCollected, currentMap,
                      win, gameOver, playerSpeed, moveCharge,
                      boostMessageTimer, -1);
            startGameRequested = false;
        }

        // Chỉ nhận phím điều khiển gameplay khi đang ở màn hình chơi.
        if (screen == PLAYING_SCREEN) {
            // M về menu, R tạo map ngẫu nhiên, N chuyển sang map kế tiếp.
            if (IsKeyPressed(KEY_M)) screen = MAIN_MENU;

            if (IsKeyPressed(KEY_R)) {
                ResetGame(mazeList, maze, playerR, playerC, exitR, exitC,
                          enemy, items, score, itemsCollected, currentMap,
                          win, gameOver, playerSpeed, moveCharge,
                          boostMessageTimer, -1);
            }
            if (IsKeyPressed(KEY_N)) {
                ResetGame(mazeList, maze, playerR, playerC, exitR, exitC,
                          enemy, items, score, itemsCollected, currentMap,
                          win, gameOver, playerSpeed, moveCharge,
                          boostMessageTimer, (currentMap + 1) % MAP_COUNT);
            }

            // Ngừng cập nhật di chuyển sau khi ván chơi đã kết thúc.
            if (!win && !gameOver) {
                int dr = 0;
                int dc = 0;
                bool movePressed = false;
                // Chuyển phím bấm thành độ thay đổi hàng và cột.
                if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
                    dr = -1;
                    movePressed = true;
                } else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
                    dr = 1;
                    movePressed = true;
                } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                    dc = -1;
                    movePressed = true;
                } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                    dc = 1;
                    movePressed = true;
                }

                // moveCharge tích lũy phần tốc độ lẻ, ví dụ x1.2 sẽ có thêm
                // một bước sau mỗi năm lần bấm phím.
                if (movePressed) {
                    moveCharge += playerSpeed;
                    int moveSteps = (int)moveCharge;
                    moveCharge -= moveSteps;
                    for (int step = 0; step < moveSteps; step++) {
                        int nr = playerR + dr;
                        int nc = playerC + dc;
                        // Gặp tường thì dừng, nếu không sẽ cập nhật vị trí.
                        if (!CanMove(maze, nr, nc)) break;
                        playerR = nr;
                        playerC = nc;

                        // Nhặt vật phẩm và bật thông báo tăng tốc trong 1,2 giây.
                        if (CheckCollectItem(items, playerR, playerC, score,
                                             itemsCollected, playerSpeed)) {
                            boostMessageTimer = 1.2f;
                        }
                        // Chạm quỷ thì thua; tới cổng thoát thì thắng.
                        if (enemy.r == playerR && enemy.c == playerC) {
                            gameOver = true;
                            break;
                        }
                        if (playerR == exitR && playerC == exitC) {
                            win = true;
                            break;
                        }
                    }
                }

                // Cập nhật quỷ theo thời gian, không phụ thuộc số lần nhấn phím.
                if (!win && !gameOver) {
                    enemy.timer += GetFrameTime();
                    if (enemy.timer >= ENEMY_DELAY) {
                        enemy.timer = 0.0f;
                        // BFS trả về đường ngắn nhất; quỷ đi một ô đầu tiên.
                        vector<pair<int, int>> path = FindPath(
                            maze, enemy.r, enemy.c, playerR, playerC
                        );
                        if (!path.empty()) {
                            enemy.r = path[0].first;
                            enemy.c = path[0].second;
                        }
                    }
                    if (enemy.r == playerR && enemy.c == playerC) {
                        gameOver = true;
                    }
                }
            }
        }

        // Đếm lùi thời gian hiển thị dòng "+0,2 TỐC ĐỘ".
        if (boostMessageTimer > 0.0f) boostMessageTimer -= GetFrameTime();

        // Bắt đầu vẽ một khung hình mới.
        BeginDrawing();
        ClearBackground(BLACK);

        // Chọn hàm vẽ tương ứng với màn hình hiện tại.
        if (screen == MAIN_MENU) {
            DrawMainMenu(vietnameseFont, screen, quitRequested);
        } else if (screen == GUIDE_SCREEN) {
            DrawGuide(vietnameseFont, screen);
        } else if (screen == CHARACTER_SCREEN) {
            DrawCharacterSelection(vietnameseFont, characterSheet,
                                   selectedCharacter,
                                   screen, startGameRequested);
        } else if (screen == PLAYING_SCREEN) {
            const MapTheme& theme = themes[currentMap];
            // Vẽ HUD, map, cổng, vật phẩm, người chơi rồi mới vẽ quỷ.
            DrawGameHud(vietnameseFont, theme, currentMap, score,
                        itemsCollected, playerSpeed, selectedCharacter,
                        boostMessageTimer);
            DrawMaze(maze, theme);
            DrawExit(exitR, exitC, theme.accent);
            for (int i = 0; i < (int)items.size(); i++) {
                if (!items[i].collected) DrawItemPixel(items[i], i, theme.accent);
            }
            DrawCharacterFromSheet(characterSheet, selectedCharacter,
                                   playerC * TILE - 2,
                                   playerR * TILE + UI_HEIGHT - 8,
                                   TILE + 4, TILE + 8);
            DrawHornedDemon(enemy.r, enemy.c);

            // Khi kết thúc, phủ lớp tối và hiển thị bảng kết quả.
            if (win || gameOver) {
                DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.62f));
                Rectangle resultPanel = {184, 190, 500, 290};
                DrawRectangleRounded(resultPanel, 0.08f, 8,
                                     {24, 32, 60, 250});
                DrawRectangleLinesEx(resultPanel, 3,
                                     win ? GREEN : RED);
                DrawVN(vietnameseFont,
                       win ? u8"BẠN ĐÃ THOÁT KHỎI MÊ CUNG!"
                           : u8"BẠN ĐÃ BỊ QUỶ BẮT!",
                       SCREEN_W / 2.0f, 230, 29,
                       win ? GREEN : RED, true);
                DrawVN(vietnameseFont,
                       TextFormat(u8"Điểm đạt được: %d", score),
                       SCREEN_W / 2.0f, 286, 23, WHITE, true);

                // Chơi lại giữ nguyên map; Về menu chỉ đổi trạng thái màn hình.
                if (DrawButton(vietnameseFont, {224, 355, 195, 58},
                               u8"CHƠI LẠI", {39, 121, 184, 255},
                               {56, 159, 222, 255})) {
                    ResetGame(mazeList, maze, playerR, playerC, exitR, exitC,
                              enemy, items, score, itemsCollected, currentMap,
                              win, gameOver, playerSpeed, moveCharge,
                              boostMessageTimer, currentMap);
                }
                if (DrawButton(vietnameseFont, {449, 355, 195, 58},
                               u8"VỀ MENU", {105, 62, 137, 255},
                               {144, 83, 182, 255})) {
                    screen = MAIN_MENU;
                }
            }
        }

        EndDrawing(); // Kết thúc và đưa khung hình lên cửa sổ.
    }

    // Giải phóng tài nguyên trước khi đóng chương trình.
    if (characterSheet.loaded) UnloadTexture(characterSheet.texture);
    if (loadedCustomFont) UnloadFont(vietnameseFont);
    CloseWindow();
    return 0;
}
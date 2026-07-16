// ============================================================================
// PHẦN CODE NGƯỜI 4 - TÁCH NGUYÊN TỪ Pasted code(13).cpp
// ============================================================================
// Chỉ có các đoạn thuộc phần: menu, chọn nhân vật và giao diện thắng/thua.
// Không thêm chức năng, hàm demo, phím bấm hoặc luật chơi mới.
// Các dòng lệnh bên dưới giữ nguyên logic của code gốc; chỉ thêm comment.
//
// Đây là FILE HỌC RIÊNG, không build riêng vì màn hình thắng/thua vốn nằm
// bên trong main() của chương trình đầy đủ.

#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>

using namespace std;

// Các hằng số giao diện được lấy nguyên từ code gốc.
const int ROWS = 21;
const int COLS = 31;
const int TILE = 28;
const int UI_HEIGHT = 90;
const int SCREEN_W = COLS * TILE;
const int SCREEN_H = ROWS * TILE + UI_HEIGHT;
const int CHARACTER_COUNT = 6;

// ======================= 1. KIỂU DỮ LIỆU GIAO DIỆN =========================

// Biến ScreenState trong main() dùng kiểu này để ghi nhớ màn hình hiện tại.
enum ScreenState {
    MAIN_MENU,         // Menu chính.
    GUIDE_SCREEN,      // Màn hình hướng dẫn.
    CHARACTER_SCREEN,  // Màn hình chọn nhân vật.
    PLAYING_SCREEN     // Màn hình chơi.
};

// Mỗi tên nhân vật tương ứng một số từ 0 đến 5.
// Số đó được dùng làm chỉ số truy cập các mảng phía dưới.
enum CharacterType {
    NOBITA,
    JAIAN,
    SUNEO,
    SHIZUKA,
    DORAEMON,
    DEKISUGI
};

// Gộp texture nhân vật và trạng thái tải ảnh vào cùng một biến.
struct CharacterSheet {
    Texture2D texture; // Ảnh đã được đưa vào bộ nhớ đồ họa.
    bool loaded;       // true nếu tìm và tải được characters.png.
};

// Tên hiển thị dưới từng thẻ nhân vật.
const char* CHARACTER_NAMES[CHARACTER_COUNT] = {
    u8"Nobita", u8"Jaian", u8"Suneo",
    u8"Shizuka", u8"Doraemon", u8"Dekisugi"
};

// Màu nhấn của từng nhân vật, có cùng thứ tự với CHARACTER_NAMES.
const Color CHARACTER_COLORS[CHARACTER_COUNT] = {
    {250, 208, 45, 255}, {239, 112, 35, 255}, {77, 180, 110, 255},
    {241, 112, 160, 255}, {32, 151, 219, 255}, {82, 156, 220, 255}
};

// ======================= 2. HỖ TRỢ CHỮ TIẾNG VIỆT ==========================

// Tạo danh sách mã ký tự mà raylib cần tải từ file font.
vector<int> CreateVietnameseCodepoints() {
    vector<int> codepoints;

    // Ký tự ASCII cơ bản: chữ không dấu, số và dấu câu.
    for (int i = 32; i <= 126; i++) codepoints.push_back(i);

    // Hai vùng Unicode dưới chứa các chữ tiếng Việt có dấu.
    for (int i = 0x00C0; i <= 0x024F; i++) codepoints.push_back(i);
    for (int i = 0x1E00; i <= 0x1EFF; i++) codepoints.push_back(i);

    return codepoints;
}

// Tìm font có sẵn trên Windows hoặc trong thư mục assets.
Font LoadVietnameseFont(bool& loadedCustomFont) {
    const char* fontPaths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "../assets/NotoSans-Regular.ttf",
        "assets/NotoSans-Regular.ttf"
    };

    vector<int> codepoints = CreateVietnameseCodepoints();

    // Thử lần lượt từng đường dẫn; gặp font tồn tại thì tải và trả về ngay.
    for (const char* path : fontPaths) {
        if (FileExists(path)) {
            Font font = LoadFontEx(path, 38, codepoints.data(),
                                   (int)codepoints.size());
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            loadedCustomFont = true;
            return font;
        }
    }

    // Không tìm thấy font riêng thì dùng font mặc định của raylib.
    loadedCustomFont = false;
    return GetFontDefault();
}

// Hàm vẽ chữ dùng chung cho tất cả màn hình.
// centered=true: tham số x là vị trí chính giữa của dòng chữ.
void DrawVN(Font font, const char* text, float x, float y,
            float size, Color color, bool centered = false) {
    // Đo kích thước dòng chữ để tính vị trí căn giữa.
    Vector2 textSize = MeasureTextEx(font, text, size, 1.0f);
    if (centered) x -= textSize.x / 2.0f;

    // Vẽ dòng chữ tại tọa độ x, y.
    DrawTextEx(font, text, {x, y}, size, 1.0f, color);
}

// ========================= 3. TẢI ẢNH NHÂN VẬT =============================

// Kiểm tra một pixel có gần giống màu nền kem của ảnh gốc hay không.
bool IsConnectedBackground(Color color) {
    int maximum = max((int)color.r, max((int)color.g, (int)color.b));
    int minimum = min((int)color.r, min((int)color.g, (int)color.b));

    // Nền kem là màu sáng và ba thành phần màu khá gần nhau.
    return color.r > 220 && color.g > 215 && color.b > 190 &&
           maximum - minimum < 60;
}

// Chỉ xóa phần nền kem nối với biên ảnh.
// Vùng trắng bên trong mắt, mặt, tất và giày vẫn được giữ lại.
void RemoveCharacterSheetBackground(Image& image) {
    // Chuyển ảnh sang RGBA để có thể đặt pixel thành trong suốt.
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Color* pixels = (Color*)image.data;
    int width = image.width;
    int height = image.height;

    // visited đánh dấu pixel đã xét; pending chứa pixel đang chờ xử lý.
    vector<unsigned char> visited(width * height, 0);
    queue<int> pending;

    // Hàm nhỏ thêm một pixel nền chưa xét vào hàng đợi.
    auto AddBackgroundPixel = [&](int x, int y) {
        int index = y * width + x;
        if (!visited[index] && IsConnectedBackground(pixels[index])) {
            visited[index] = 1;
            pending.push(index);
        }
    };

    // Bắt đầu từ cạnh trên và cạnh dưới của ảnh.
    for (int x = 0; x < width; x++) {
        AddBackgroundPixel(x, 0);
        AddBackgroundPixel(x, height - 1);
    }

    // Tiếp tục với cạnh trái và cạnh phải.
    for (int y = 0; y < height; y++) {
        AddBackgroundPixel(0, y);
        AddBackgroundPixel(width - 1, y);
    }

    // Bốn hướng dùng để lan sang pixel bên cạnh.
    int dx[4] = {-1, 1, 0, 0};
    int dy[4] = {0, 0, -1, 1};

    while (!pending.empty()) {
        int index = pending.front();
        pending.pop();

        // Đổi chỉ số một chiều thành tọa độ x, y.
        int x = index % width;
        int y = index / width;

        // BLANK làm pixel nền trở thành trong suốt.
        pixels[index] = BLANK;

        // Xét bốn pixel nằm cạnh pixel hiện tại.
        for (int direction = 0; direction < 4; direction++) {
            int nx = x + dx[direction];
            int ny = y + dy[direction];

            // Chỉ xử lý nếu tọa độ vẫn nằm bên trong ảnh.
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

    // Khởi tạo sheet rỗng và mặc định là chưa tải được ảnh.
    CharacterSheet sheet = {};
    sheet.loaded = false;

    // Thử lần lượt từng đường dẫn.
    for (const char* path : imagePaths) {
        if (!FileExists(path)) continue; // Không có file thì thử đường dẫn sau.

        Image image = LoadImage(path);
        if (image.data == nullptr) continue; // Tải lỗi thì thử đường dẫn sau.

        RemoveCharacterSheetBackground(image);      // Xóa nền kem.
        sheet.texture = LoadTextureFromImage(image); // Chuyển Image thành Texture.
        SetTextureFilter(sheet.texture, TEXTURE_FILTER_POINT); // Giữ nét pixel.
        UnloadImage(image);                          // Giải phóng ảnh tạm.

        sheet.loaded = true;
        break; // Đã tải thành công nên không cần thử tiếp.
    }

    return sheet;
}

// Lấy vùng chữ nhật chứa đúng một nhân vật trong ảnh lớn.
Rectangle GetCharacterSource(const CharacterSheet& sheet, int character) {
    // Các tọa độ dưới được đo trên ảnh gốc 1774x887.
    Rectangle originalSources[CHARACTER_COUNT] = {
        {384, 224, 210, 465},   // Nobita
        {878, 175, 338, 514},   // Jaian
        {1226, 230, 218, 459},  // Suneo
        {647, 236, 217, 453},   // Shizuka
        {53, 278, 286, 411},    // Doraemon
        {1498, 207, 206, 482}   // Dekisugi
    };

    // Tính tỉ lệ nếu kích thước texture khác kích thước ảnh gốc.
    float scaleX = sheet.texture.width / 1774.0f;
    float scaleY = sheet.texture.height / 887.0f;

    Rectangle source = originalSources[character];
    source.x *= scaleX;
    source.y *= scaleY;
    source.width *= scaleX;
    source.height *= scaleY;

    return source;
}

// Cắt đúng nhân vật, giữ tỉ lệ và căn nhân vật vào vùng cần vẽ.
void DrawCharacterFromSheet(const CharacterSheet& sheet, int character,
                            float areaX, float areaY,
                            float areaWidth, float areaHeight) {
    // Chưa tải được ảnh thì dừng hàm.
    if (!sheet.loaded) return;

    Rectangle source = GetCharacterSource(sheet, character);

    // Chọn tỉ lệ nhỏ hơn để ảnh không vượt chiều rộng hoặc chiều cao vùng vẽ.
    float scale = min(areaWidth / source.width, areaHeight / source.height);
    float drawWidth = source.width * scale;
    float drawHeight = source.height * scale;

    // destination là vị trí và kích thước nhân vật trên màn hình.
    Rectangle destination = {
        areaX + (areaWidth - drawWidth) / 2.0f, // Căn giữa theo chiều ngang.
        areaY + areaHeight - drawHeight,        // Đặt chân sát đáy vùng vẽ.
        drawWidth,
        drawHeight
    };

    DrawTexturePro(sheet.texture, source, destination, {0, 0}, 0, WHITE);
}

// ======================= 4. NỀN VÀ NÚT GIAO DIỆN ===========================

// Vẽ nền menu gồm gradient, chấm sáng, tiêu đề và phụ đề.
void DrawBackground(Font font, const char* subtitle) {
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {20, 26, 52, 255}, {8, 12, 27, 255});

    // GetTime() làm các chấm sáng di chuyển theo thời gian.
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
    // Kiểm tra nút đang bật và con trỏ chuột nằm trong rect.
    bool pointed = enabled && CheckCollisionPointRec(GetMousePosition(), rect);

    // Hover dùng màu hover; trạng thái thường dùng normal; bị khóa thì làm mờ.
    Color buttonColor = enabled ? (pointed ? hover : normal)
                                : Fade(normal, 0.35f);

    // Vẽ bóng, thân nút, viền và chữ.
    DrawRectangleRounded({rect.x + 4, rect.y + 5, rect.width, rect.height},
                         0.22f, 8, Fade(BLACK, 0.45f));
    DrawRectangleRounded(rect, 0.22f, 8, buttonColor);
    DrawRectangleLinesEx(rect, 2, Fade(WHITE, pointed ? 0.80f : 0.25f));
    DrawVN(font, text, rect.x + rect.width / 2.0f,
           rect.y + rect.height / 2.0f - 12, 23,
           enabled ? WHITE : GRAY, true);

    // Chỉ true đúng khung hình người dùng nhấn chuột trái.
    return pointed && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Vẽ nút lớn của menu: bóng, gradient, số thứ tự và hiệu ứng hover.
bool DrawMenuButton(Font font, Rectangle rect, const char* text,
                    const char* number, Color accent) {
    bool pointed = CheckCollisionPointRec(GetMousePosition(), rect);

    // Khi hover, nút dịch sang phải 5 pixel.
    float hoverShift = pointed ? 5.0f : 0.0f;
    Rectangle moved = {rect.x + hoverShift, rect.y, rect.width, rect.height};

    // sinf() tạo độ sáng tăng giảm liên tục.
    float pulse = 0.12f + 0.05f * sinf((float)GetTime() * 4.0f);

    // Chỉ vẽ quầng sáng khi chuột nằm trên nút.
    if (pointed) {
        DrawRectangleRounded({moved.x - 8, moved.y - 7,
                              moved.width + 16, moved.height + 14},
                             0.18f, 8, Fade(accent, 0.18f + pulse));
    }

    // Vẽ bóng và nền của nút.
    DrawRectangleRounded({moved.x + 7, moved.y + 8,
                          moved.width, moved.height},
                         0.18f, 8, Fade(BLACK, 0.58f));
    DrawRectangleRounded(moved, 0.18f, 8,
                         pointed ? Color{34, 42, 70, 255}
                                 : Color{25, 32, 56, 255});

    // Vẽ lớp gradient nằm bên trong nút.
    Rectangle inner = {moved.x + 6, moved.y + 6,
                       moved.width - 12, moved.height - 12};
    DrawRectangleGradientH((int)inner.x, (int)inner.y,
                           (int)inner.width, (int)inner.height,
                           Fade(accent, pointed ? 0.70f : 0.45f),
                           {24, 31, 54, 255});

    // Vẽ viền và thanh màu bên trái.
    DrawRectangleRoundedLines(moved, 0.18f, 8,
                              pointed ? Fade(accent, 0.95f)
                                      : Fade(WHITE, 0.25f));
    DrawRectangle((int)moved.x + 12, (int)moved.y + 12,
                  5, (int)moved.height - 24, accent);

    // Vẽ vòng tròn số thứ tự và tên nút.
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

    // Hai đoạn thẳng tạo thành dấu mũi tên ở cuối nút.
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

// ============================ 5. MENU CHÍNH ================================

// Vẽ menu và xử lý ba nút Bắt đầu, Hướng dẫn, Thoát.
void DrawMainMenu(Font font, ScreenState& screen, bool& quitRequested) {
    DrawBackground(font, u8"Cuộc chạy trốn khỏi mê cung của quỷ");

    // Khung lớn chứa ba nút.
    Rectangle menuPanel = {229, 202, 410, 337};
    DrawRectangleRounded({menuPanel.x + 10, menuPanel.y + 12,
                          menuPanel.width, menuPanel.height},
                         0.08f, 8, Fade(BLACK, 0.42f));
    DrawRectangleRounded(menuPanel, 0.08f, 8, {17, 23, 43, 225});
    DrawRectangleRoundedLines(menuPanel, 0.08f, 8, Fade(SKYBLUE, 0.25f));
    DrawVN(font, u8"CHỌN CHỨC NĂNG", SCREEN_W / 2.0f,
           220, 18, Fade(WHITE, 0.65f), true);
    DrawLine(275, 250, 593, 250, Fade(SKYBLUE, 0.25f));

    // Tọa độ và kích thước vùng bấm của ba nút.
    Rectangle startButton = {259, 270, 350, 68};
    Rectangle guideButton = {259, 360, 350, 68};
    Rectangle exitButton = {259, 450, 350, 68};

    // Bắt đầu: chuyển sang màn hình chọn nhân vật.
    if (DrawMenuButton(font, startButton, u8"BẮT ĐẦU", "01",
                       {46, 190, 147, 255})) {
        screen = CHARACTER_SCREEN;
    }

    // Hướng dẫn: chuyển sang màn hình hướng dẫn.
    if (DrawMenuButton(font, guideButton, u8"HƯỚNG DẪN", "02",
                       {102, 141, 244, 255})) {
        screen = GUIDE_SCREEN;
    }

    // Thoát: đặt cờ để vòng lặp chính kết thúc.
    if (DrawMenuButton(font, exitButton, u8"THOÁT", "03",
                       {232, 79, 104, 255})) {
        quitRequested = true;
    }
}

// ========================= 6. CHỌN NHÂN VẬT ================================

// Vẽ sáu thẻ nhân vật và lưu nhân vật được chọn.
void DrawCharacterSelection(Font font, const CharacterSheet& characterSheet,
                            int& selectedCharacter,
                            ScreenState& screen, bool& startGameRequested) {
    // Vẽ nền, tiêu đề và dòng hướng dẫn.
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {24, 31, 61, 255}, {9, 14, 30, 255});
    DrawVN(font, u8"CHỌN NHÂN VẬT", SCREEN_W / 2.0f, 25, 39,
           {255, 221, 91, 255}, true);
    DrawVN(font, u8"Nhấp vào nhân vật mà bạn muốn sử dụng",
           SCREEN_W / 2.0f, 76, 20, LIGHTGRAY, true);

    // Lặp sáu lần để tạo sáu thẻ.
    for (int i = 0; i < CHARACTER_COUNT; i++) {
        // Chia thẻ thành hai hàng, mỗi hàng ba cột.
        int col = i % 3;
        int row = i / 3;
        Rectangle card = {(float)(91 + col * 230),
                          (float)(120 + row * 205), 180, 174};

        // Kiểm tra trạng thái hover và trạng thái đã chọn.
        bool hover = CheckCollisionPointRec(GetMousePosition(), card);
        bool selected = selectedCharacter == i;
        Color characterAccent = CHARACTER_COLORS[i];

        // Thẻ đã chọn, đang hover và bình thường có màu khác nhau.
        Color cardColor = selected ? Fade(characterAccent, 0.42f)
                                   : (hover ? Color{42, 54, 92, 255}
                                            : Color{29, 38, 70, 255});
        DrawRectangleRounded(card, 0.10f, 8, cardColor);

        // Vẽ họa tiết ô vuông mờ trên nền thẻ.
        for (int py = 0; py < 5; py++) {
            for (int px = 0; px < 6; px++) {
                if ((px + py) % 2 == 0) {
                    DrawRectangle((int)card.x + 8 + px * 27,
                                  (int)card.y + 8 + py * 24,
                                  13, 12, Fade(characterAccent, 0.07f));
                }
            }
        }

        // Vẽ viền, hình ảnh và tên nhân vật.
        DrawRectangleLinesEx(card, selected ? 4 : 2,
                             selected ? characterAccent : Fade(WHITE, 0.25f));
        DrawCharacterFromSheet(characterSheet, i,
                               card.x + 25, card.y + 10, 130, 120);
        DrawVN(font, CHARACTER_NAMES[i], card.x + card.width / 2.0f,
               card.y + 138, 21, selected ? characterAccent : WHITE, true);

        // Chỉ thẻ đang được chọn mới có dòng ĐÃ CHỌN.
        if (selected) {
            DrawVN(font, u8"ĐÃ CHỌN", card.x + card.width / 2.0f,
                   card.y + 160, 12, WHITE, true);
        }

        // Nhấn chuột vào thẻ nào thì lưu chỉ số của nhân vật đó.
        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedCharacter = i;
        }
    }

    // Quay lại menu chính.
    if (DrawButton(font, {115, 552, 230, 58}, u8"QUAY LẠI",
                   {67, 72, 104, 255}, {91, 101, 142, 255})) {
        screen = MAIN_MENU;
    }

    // Yêu cầu tạo ván mới rồi chuyển sang màn hình chơi.
    if (DrawButton(font, {523, 552, 230, 58}, u8"VÀO MÊ CUNG",
                   {35, 133, 90, 255}, {49, 181, 118, 255})) {
        startGameRequested = true;
        screen = PLAYING_SCREEN;
    }
}

// ============================================================================
// 7. CÁC ĐOẠN CỦA NGƯỜI 4 NẰM BÊN TRONG main() GỐC
// ============================================================================
// Những đoạn dưới được giữ dưới dạng comment vì khi tách khỏi main() chúng
// không thể chạy độc lập. Nội dung lệnh được lấy từ code gốc, không thêm logic.

/*
// ----- Khởi tạo tài nguyên và trạng thái giao diện -----

bool loadedCustomFont = false;
Font vietnameseFont = LoadVietnameseFont(loadedCustomFont);
CharacterSheet characterSheet = LoadCharacterSheet();

// screen quyết định màn hình đang được vẽ.
ScreenState screen = MAIN_MENU;
bool quitRequested = false;
bool startGameRequested = false;
int selectedCharacter = NOBITA;


// ----- Chọn màn hình cần vẽ trong vòng lặp chính -----

if (screen == MAIN_MENU) {
    // Vẽ menu chính.
    DrawMainMenu(vietnameseFont, screen, quitRequested);
} else if (screen == GUIDE_SCREEN) {
    // Màn hình hướng dẫn do hàm DrawGuide() của code gốc vẽ.
    DrawGuide(vietnameseFont, screen);
} else if (screen == CHARACTER_SCREEN) {
    // Vẽ sáu thẻ và nhận lựa chọn nhân vật.
    DrawCharacterSelection(vietnameseFont, characterSheet,
                           selectedCharacter,
                           screen, startGameRequested);
} else if (screen == PLAYING_SCREEN) {
    // Vẽ nhân vật đã chọn tại vị trí của người chơi trên bản đồ.
    DrawCharacterFromSheet(characterSheet, selectedCharacter,
                           playerC * TILE - 2,
                           playerR * TILE + UI_HEIGHT - 8,
                           TILE + 4, TILE + 8);


    // ----- Màn hình thắng hoặc thua -----

    // Chỉ hiện bảng kết quả khi win hoặc gameOver bằng true.
    if (win || gameOver) {
        // Phủ màu đen mờ lên màn hình chơi.
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.62f));

        // Tạo bảng kết quả và viền xanh nếu thắng, đỏ nếu thua.
        Rectangle resultPanel = {184, 190, 500, 290};
        DrawRectangleRounded(resultPanel, 0.08f, 8,
                             {24, 32, 60, 250});
        DrawRectangleLinesEx(resultPanel, 3,
                             win ? GREEN : RED);

        // Toán tử ba ngôi chọn nội dung và màu theo kết quả.
        DrawVN(vietnameseFont,
               win ? u8"BẠN ĐÃ THOÁT KHỎI MÊ CUNG!"
                   : u8"BẠN ĐÃ BỊ QUỶ BẮT!",
               SCREEN_W / 2.0f, 230, 29,
               win ? GREEN : RED, true);

        // Hiển thị điểm đạt được.
        DrawVN(vietnameseFont,
               TextFormat(u8"Điểm đạt được: %d", score),
               SCREEN_W / 2.0f, 286, 23, WHITE, true);

        // CHƠI LẠI đặt lại dữ liệu và giữ nguyên bản đồ hiện tại.
        if (DrawButton(vietnameseFont, {224, 355, 195, 58},
                       u8"CHƠI LẠI", {39, 121, 184, 255},
                       {56, 159, 222, 255})) {
            ResetGame(mazeList, maze, playerR, playerC, exitR, exitC,
                      enemy, items, score, itemsCollected, currentMap,
                      win, gameOver, playerSpeed, moveCharge,
                      boostMessageTimer, currentMap);
        }

        // VỀ MENU chỉ đổi trạng thái màn hình.
        if (DrawButton(vietnameseFont, {449, 355, 195, 58},
                       u8"VỀ MENU", {105, 62, 137, 255},
                       {144, 83, 182, 255})) {
            screen = MAIN_MENU;
        }
    }
}


// ----- Giải phóng tài nguyên trước khi đóng chương trình -----

if (characterSheet.loaded) UnloadTexture(characterSheet.texture);
if (loadedCustomFont) UnloadFont(vietnameseFont);
*/

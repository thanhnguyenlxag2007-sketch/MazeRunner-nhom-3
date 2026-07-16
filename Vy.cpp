// ============================================================================
// PHẦN CODE CỦA NGƯỜI 4: MENU, NHÂN VẬT VÀ GIAO DIỆN
// ============================================================================
// Đây là bản demo tách riêng khỏi map, vật phẩm và AI của game Maze Runner.
// File có thể build và chạy độc lập để người 4 học, thuyết trình, thử giao diện.
//
// Chức năng chính:
// 1. Hiển thị tiếng Việt có dấu.
// 2. Tải ảnh characters.png và xóa nền kem.
// 3. Cắt sáu nhân vật từ một ảnh lớn.
// 4. Vẽ menu Bắt đầu - Hướng dẫn - Thoát.
// 5. Chọn Nobita, Jaian, Suneo, Shizuka, Doraemon hoặc Dekisugi.
// 6. Minh họa màn hình chơi và bảng thắng/thua.
//
// Lưu ý: đặt characters.png cạnh file .exe hoặc ở thư mục cha của file .exe.

#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>

using namespace std;

// ============================== CẤU HÌNH ====================================

// Kích thước cửa sổ giống game chính.
const int SCREEN_W = 868;
const int SCREEN_H = 678;

// Tổng số nhân vật trong ảnh characters.png.
const int CHARACTER_COUNT = 6;

// ========================= TRẠNG THÁI GIAO DIỆN =============================

// ScreenState cho biết chương trình đang ở màn hình nào.
// Nhờ biến này, main() chỉ cần vẽ đúng một màn hình trong mỗi khung hình.
enum ScreenState {
    MAIN_MENU,        // Menu có ba nút Bắt đầu, Hướng dẫn, Thoát.
    GUIDE_SCREEN,     // Trang hướng dẫn cách chơi.
    CHARACTER_SCREEN, // Trang chọn một trong sáu nhân vật.
    PLAYING_SCREEN    // Trang minh họa giao diện khi đang chơi.
};

// Mỗi tên nhân vật được gán một số từ 0 đến 5.
// Các số này cũng là chỉ số dùng trong mảng tên, màu và vùng cắt ảnh.
enum CharacterType {
    NOBITA = 0,
    JAIAN,
    SUNEO,
    SHIZUKA,
    DORAEMON,
    DEKISUGI
};

// Struct gom ảnh nhân vật và trạng thái tải ảnh vào cùng một biến.
struct CharacterSheet {
    Texture2D texture; // Texture là ảnh đã được đưa vào bộ nhớ đồ họa.
    bool loaded;       // true: tải thành công; false: chưa tìm thấy ảnh.
};

// Tên hiển thị dưới từng thẻ nhân vật.
// Thứ tự phải giống CharacterType và vùng cắt trong GetCharacterSource().
const char* CHARACTER_NAMES[CHARACTER_COUNT] = {
    u8"Nobita", u8"Jaian", u8"Suneo",
    u8"Shizuka", u8"Doraemon", u8"Dekisugi"
};

// Mỗi nhân vật có một màu nhấn riêng để vẽ viền thẻ.
const Color CHARACTER_COLORS[CHARACTER_COUNT] = {
    {250, 208, 45, 255}, // Nobita: vàng.
    {239, 112, 35, 255}, // Jaian: cam.
    {77, 180, 110, 255}, // Suneo: xanh lá.
    {241, 112, 160, 255},// Shizuka: hồng.
    {32, 151, 219, 255}, // Doraemon: xanh dương.
    {82, 156, 220, 255}  // Dekisugi: xanh nhạt.
};

// =========================== FONT TIẾNG VIỆT ================================

// Raylib cần biết trước các ký tự sẽ được nạp từ file font.
// Hàm này tạo danh sách ASCII và các vùng Unicode chứa chữ tiếng Việt.
vector<int> CreateVietnameseCodepoints() {
    vector<int> codepoints;

    // Ký tự ASCII: chữ không dấu, số và dấu câu.
    for (int code = 32; code <= 126; code++) {
        codepoints.push_back(code);
    }

    // Latin mở rộng: chứa nhiều chữ có dấu.
    for (int code = 0x00C0; code <= 0x024F; code++) {
        codepoints.push_back(code);
    }

    // Latin Extended Additional: chứa Ạ, Ả, Ấ, Ầ, Ế, Ệ...
    for (int code = 0x1E00; code <= 0x1EFF; code++) {
        codepoints.push_back(code);
    }

    return codepoints;
}

// Tìm một font có hỗ trợ tiếng Việt rồi tải font đó vào chương trình.
// loadedCustomFont được truyền bằng tham chiếu để main() biết có cần unload.
Font LoadVietnameseFont(bool& loadedCustomFont) {
    // Hai đường dẫn đầu là font thường có sẵn trên Windows.
    // Hai đường dẫn sau dùng khi nhóm có thư mục assets riêng.
    const char* fontPaths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "assets/NotoSans-Regular.ttf",
        "../assets/NotoSans-Regular.ttf"
    };

    vector<int> codepoints = CreateVietnameseCodepoints();

    // Thử lần lượt từng đường dẫn.
    for (const char* path : fontPaths) {
        if (!FileExists(path)) continue; // Không có file thì thử đường dẫn sau.

        // 38 là kích thước cơ sở dùng khi tạo font.
        Font font = LoadFontEx(path, 38, codepoints.data(),
                               (int)codepoints.size());

        // Bilinear giúp chữ khi phóng to nhìn mượt hơn.
        SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

        loadedCustomFont = true;
        return font;
    }

    // Nếu không tìm thấy font, dùng font mặc định để chương trình vẫn chạy.
    // Font mặc định có thể không hiển thị đủ chữ tiếng Việt.
    loadedCustomFont = false;
    return GetFontDefault();
}

// Hàm vẽ chữ dùng chung cho toàn bộ giao diện.
// centered=false: x là mép trái của chữ.
// centered=true : x là tâm ngang của chữ.
void DrawVN(Font font, const char* text, float x, float y,
            float size, Color color, bool centered = false) {
    // Đo chiều rộng của dòng chữ để có thể căn giữa.
    Vector2 textSize = MeasureTextEx(font, text, size, 1.0f);

    // Muốn căn giữa thì lùi x lại một nửa chiều rộng dòng chữ.
    if (centered) x -= textSize.x / 2.0f;

    // 1.0f là khoảng cách giữa các ký tự.
    DrawTextEx(font, text, {x, y}, size, 1.0f, color);
}

// ============================= ẢNH NHÂN VẬT ================================

// Kiểm tra một pixel có gần giống màu nền kem của ảnh gốc hay không.
bool IsConnectedBackground(Color color) {
    int largest = max((int)color.r, max((int)color.g, (int)color.b));
    int smallest = min((int)color.r, min((int)color.g, (int)color.b));

    // Nền kem có độ sáng cao và ba kênh màu khá gần nhau.
    return color.r > 220 && color.g > 215 && color.b > 190 &&
           largest - smallest < 60;
}

// Xóa phần nền kem nối với cạnh ảnh.
// Cách này giữ lại màu trắng bên trong mắt, mặt, tất và giày của nhân vật.
void RemoveCharacterSheetBackground(Image& image) {
    // Chuyển ảnh thành RGBA để mỗi pixel có kênh alpha trong suốt.
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Color* pixels = (Color*)image.data;
    int width = image.width;
    int height = image.height;

    // visited[index] = 1 nghĩa là pixel đó đã được đưa vào hàng đợi.
    vector<unsigned char> visited(width * height, 0);
    queue<int> pending;

    // Lambda này thêm một pixel nền hợp lệ vào hàng đợi.
    auto AddBackgroundPixel = [&](int x, int y) {
        int index = y * width + x;
        if (!visited[index] && IsConnectedBackground(pixels[index])) {
            visited[index] = 1;
            pending.push(index);
        }
    };

    // Chỉ bắt đầu từ bốn cạnh ảnh.
    // Vì vậy, thuật toán chỉ xóa nền bên ngoài nhân vật.
    for (int x = 0; x < width; x++) {
        AddBackgroundPixel(x, 0);
        AddBackgroundPixel(x, height - 1);
    }
    for (int y = 0; y < height; y++) {
        AddBackgroundPixel(0, y);
        AddBackgroundPixel(width - 1, y);
    }

    // Bốn hướng: trái, phải, trên, dưới.
    int dx[4] = {-1, 1, 0, 0};
    int dy[4] = {0, 0, -1, 1};

    // Duyệt lan từ nền ở cạnh ảnh vào phía trong.
    while (!pending.empty()) {
        int index = pending.front();
        pending.pop();

        int x = index % width;
        int y = index / width;

        // BLANK là màu trong suốt hoàn toàn của raylib.
        pixels[index] = BLANK;

        // Kiểm tra bốn pixel đứng cạnh pixel hiện tại.
        for (int direction = 0; direction < 4; direction++) {
            int nextX = x + dx[direction];
            int nextY = y + dy[direction];

            // Chỉ xét tọa độ còn nằm trong ảnh.
            if (nextX >= 0 && nextX < width &&
                nextY >= 0 && nextY < height) {
                AddBackgroundPixel(nextX, nextY);
            }
        }
    }
}

// Tìm characters.png rồi chuyển ảnh thành texture để raylib có thể vẽ.
CharacterSheet LoadCharacterSheet() {
    // Các vị trí thường gặp khi chạy file exe từ thư mục output.
    const char* imagePaths[] = {
        "characters.png",
        "../characters.png",
        "assets/characters.png",
        "../assets/characters.png",
        "../../characters.png"
    };

    // {} đặt toàn bộ dữ liệu ban đầu về 0.
    CharacterSheet sheet = {};
    sheet.loaded = false;

    // Thử từng đường dẫn cho tới khi tải được ảnh.
    for (const char* path : imagePaths) {
        if (!FileExists(path)) continue;

        Image image = LoadImage(path);
        if (image.data == nullptr) continue;

        RemoveCharacterSheetBackground(image);      // Xóa nền kem.
        sheet.texture = LoadTextureFromImage(image); // Đưa ảnh vào GPU.

        // TEXTURE_FILTER_POINT giữ cạnh pixel rõ, không làm ảnh bị nhòe.
        SetTextureFilter(sheet.texture, TEXTURE_FILTER_POINT);

        // Texture đã chứa dữ liệu cần thiết nên giải phóng Image tạm.
        UnloadImage(image);

        sheet.loaded = true;
        break;
    }

    return sheet;
}

// Lấy vùng chữ nhật chứa một nhân vật trong ảnh characters.png.
Rectangle GetCharacterSource(const CharacterSheet& sheet, int character) {
    // Tọa độ được đo trên ảnh gốc có kích thước 1774x887.
    Rectangle originalSources[CHARACTER_COUNT] = {
        {384, 224, 210, 465},  // Nobita.
        {878, 175, 338, 514},  // Jaian.
        {1226, 230, 218, 459}, // Suneo.
        {647, 236, 217, 453},  // Shizuka.
        {53, 278, 286, 411},   // Doraemon.
        {1498, 207, 206, 482}  // Dekisugi.
    };

    // Nếu người dùng thay bằng ảnh cùng bố cục nhưng kích thước khác,
    // scaleX và scaleY giúp đổi tọa độ cắt theo kích thước mới.
    float scaleX = sheet.texture.width / 1774.0f;
    float scaleY = sheet.texture.height / 887.0f;

    Rectangle source = originalSources[character];
    source.x *= scaleX;
    source.y *= scaleY;
    source.width *= scaleX;
    source.height *= scaleY;

    return source;
}

// Cắt đúng nhân vật rồi vẽ nhân vật đó vừa với một vùng cho trước.
void DrawCharacterFromSheet(const CharacterSheet& sheet, int character,
                            float areaX, float areaY,
                            float areaWidth, float areaHeight) {
    // Nếu chưa tải ảnh thì không thể vẽ.
    if (!sheet.loaded) return;

    Rectangle source = GetCharacterSource(sheet, character);

    // Chọn tỉ lệ nhỏ hơn để nhân vật không vượt chiều rộng hoặc chiều cao.
    float scale = min(areaWidth / source.width,
                      areaHeight / source.height);

    float drawWidth = source.width * scale;
    float drawHeight = source.height * scale;

    // Căn giữa nhân vật theo chiều ngang và đặt chân sát đáy vùng vẽ.
    Rectangle destination = {
        areaX + (areaWidth - drawWidth) / 2.0f,
        areaY + areaHeight - drawHeight,
        drawWidth,
        drawHeight
    };

    // WHITE giữ nguyên màu gốc của texture.
    DrawTexturePro(sheet.texture, source, destination, {0, 0}, 0, WHITE);
}

// ============================= NÚT VÀ NỀN ==================================

// Vẽ nền chung cho menu và màn hình hướng dẫn.
void DrawBackground(Font font, const char* subtitle) {
    // Nền chuyển màu từ xanh đậm xuống gần đen.
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {20, 26, 52, 255},
                           {8, 12, 27, 255});

    // Vẽ các chấm sáng. GetTime() làm chúng chuyển động theo thời gian.
    for (int i = 0; i < 28; i++) {
        float x = (float)((i * 97 + (int)(GetTime() * 12)) % SCREEN_W);
        float y = (float)((i * 53) % SCREEN_H);
        DrawCircle((int)x, (int)y, 2 + i % 3,
                   Fade(SKYBLUE, 0.18f));
    }

    DrawVN(font, u8"MAZE RUNNER", SCREEN_W / 2.0f, 55, 54,
           {255, 221, 91, 255}, true);
    DrawVN(font, subtitle, SCREEN_W / 2.0f, 118, 23,
           LIGHTGRAY, true);
}

// Vẽ nút thông thường.
// Giá trị trả về là true đúng lúc người dùng bấm chuột trái vào nút.
bool DrawButton(Font font, Rectangle rect, const char* text,
                Color normal, Color hover, bool enabled = true) {
    // GetMousePosition() lấy vị trí chuột.
    // CheckCollisionPointRec() kiểm tra chuột có nằm trong rect hay không.
    bool pointed = enabled &&
                   CheckCollisionPointRec(GetMousePosition(), rect);

    // Chọn màu theo trạng thái của nút.
    Color buttonColor = enabled
        ? (pointed ? hover : normal)
        : Fade(normal, 0.35f);

    // Bóng đổ nằm lệch xuống dưới và sang phải.
    DrawRectangleRounded(
        {rect.x + 4, rect.y + 5, rect.width, rect.height},
        0.22f, 8, Fade(BLACK, 0.45f)
    );

    // Thân nút và viền nút.
    DrawRectangleRounded(rect, 0.22f, 8, buttonColor);
    DrawRectangleLinesEx(rect, 2,
                         Fade(WHITE, pointed ? 0.80f : 0.25f));

    // Vẽ chữ ở chính giữa nút.
    DrawVN(font, text,
           rect.x + rect.width / 2.0f,
           rect.y + rect.height / 2.0f - 12,
           23, enabled ? WHITE : GRAY, true);

    return pointed && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Vẽ nút lớn của menu chính: có số, gradient, bóng và hiệu ứng hover.
bool DrawMenuButton(Font font, Rectangle rect, const char* text,
                    const char* number, Color accent) {
    bool pointed = CheckCollisionPointRec(GetMousePosition(), rect);

    // Khi hover, nút dịch sang phải 5 pixel.
    float hoverShift = pointed ? 5.0f : 0.0f;
    Rectangle moved = {
        rect.x + hoverShift, rect.y, rect.width, rect.height
    };

    // sinf() tạo giá trị lên xuống liên tục, dùng cho ánh sáng nhấp nháy.
    float pulse = 0.12f + 0.05f * sinf((float)GetTime() * 4.0f);

    // Chỉ vẽ quầng sáng khi chuột đang trỏ vào nút.
    if (pointed) {
        DrawRectangleRounded(
            {moved.x - 8, moved.y - 7,
             moved.width + 16, moved.height + 14},
            0.18f, 8, Fade(accent, 0.18f + pulse)
        );
    }

    // Bóng đen phía sau nút.
    DrawRectangleRounded(
        {moved.x + 7, moved.y + 8, moved.width, moved.height},
        0.18f, 8, Fade(BLACK, 0.58f)
    );

    // Thân nút sáng hơn khi hover.
    DrawRectangleRounded(
        moved, 0.18f, 8,
        pointed ? Color{34, 42, 70, 255}
                : Color{25, 32, 56, 255}
    );

    // Vùng nhỏ bên trong để tô gradient ngang.
    Rectangle inner = {
        moved.x + 6, moved.y + 6,
        moved.width - 12, moved.height - 12
    };
    DrawRectangleGradientH(
        (int)inner.x, (int)inner.y,
        (int)inner.width, (int)inner.height,
        Fade(accent, pointed ? 0.70f : 0.45f),
        {24, 31, 54, 255}
    );

    // Viền và thanh màu ở mép trái.
    DrawRectangleRoundedLines(
        moved, 0.18f, 8,
        pointed ? Fade(accent, 0.95f) : Fade(WHITE, 0.25f)
    );
    DrawRectangle((int)moved.x + 12, (int)moved.y + 12,
                  5, (int)moved.height - 24, accent);

    // Vòng tròn chứa số thứ tự.
    DrawCircle((int)moved.x + 43,
               (int)(moved.y + moved.height / 2),
               20, Fade(BLACK, 0.35f));
    DrawCircleLines((int)moved.x + 43,
                    (int)(moved.y + moved.height / 2),
                    20, Fade(accent, 0.90f));

    DrawVN(font, number,
           moved.x + 43,
           moved.y + moved.height / 2 - 11,
           19, WHITE, true);
    DrawVN(font, text,
           moved.x + 82,
           moved.y + moved.height / 2 - 14,
           26, WHITE);

    // Hai đường thẳng tạo thành dấu > ở bên phải nút.
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

// =============================== CÁC MÀN HÌNH ================================

// Vẽ menu chính và đổi trạng thái khi người dùng nhấn một nút.
void DrawMainMenu(Font font, ScreenState& screen, bool& quitRequested) {
    DrawBackground(font, u8"Cuộc chạy trốn khỏi mê cung của quỷ");

    // Khung lớn chứa ba nút.
    Rectangle menuPanel = {229, 202, 410, 337};
    DrawRectangleRounded(
        {menuPanel.x + 10, menuPanel.y + 12,
         menuPanel.width, menuPanel.height},
        0.08f, 8, Fade(BLACK, 0.42f)
    );
    DrawRectangleRounded(menuPanel, 0.08f, 8,
                         {17, 23, 43, 225});
    DrawRectangleRoundedLines(menuPanel, 0.08f, 8,
                              Fade(SKYBLUE, 0.25f));

    DrawVN(font, u8"CHỌN CHỨC NĂNG",
           SCREEN_W / 2.0f, 220, 18,
           Fade(WHITE, 0.65f), true);
    DrawLine(275, 250, 593, 250, Fade(SKYBLUE, 0.25f));

    // Rectangle lưu x, y, chiều rộng và chiều cao của vùng bấm.
    Rectangle startButton = {259, 270, 350, 68};
    Rectangle guideButton = {259, 360, 350, 68};
    Rectangle exitButton  = {259, 450, 350, 68};

    // Bắt đầu -> mở màn hình chọn nhân vật.
    if (DrawMenuButton(font, startButton, u8"BẮT ĐẦU", "01",
                       {46, 190, 147, 255})) {
        screen = CHARACTER_SCREEN;
    }

    // Hướng dẫn -> mở màn hình hướng dẫn.
    if (DrawMenuButton(font, guideButton, u8"HƯỚNG DẪN", "02",
                       {102, 141, 244, 255})) {
        screen = GUIDE_SCREEN;
    }

    // Thoát -> main() kết thúc vòng lặp.
    if (DrawMenuButton(font, exitButton, u8"THOÁT", "03",
                       {232, 79, 104, 255})) {
        quitRequested = true;
    }
}

// Vẽ trang hướng dẫn và nút quay về menu.
void DrawGuide(Font font, ScreenState& screen) {
    DrawBackground(font, u8"Hướng dẫn chơi");

    Rectangle panel = {115, 180, 638, 340};
    DrawRectangleRounded(panel, 0.08f, 8, {28, 37, 69, 245});
    DrawRectangleLinesEx(panel, 2, Fade(SKYBLUE, 0.40f));

    DrawVN(font, u8"WASD hoặc phím mũi tên: Di chuyển",
           160, 220, 23, WHITE);
    DrawVN(font, u8"Ăn vật phẩm: Tăng 0,2 tốc độ",
           160, 266, 23, WHITE);
    DrawVN(font, u8"Đi đến cổng sáng: Chiến thắng",
           160, 312, 23, WHITE);
    DrawVN(font, u8"Tránh quỷ có sừng đang đuổi theo",
           160, 358, 23, WHITE);
    DrawVN(font, u8"R: Bản đồ ngẫu nhiên   N: Bản đồ tiếp theo",
           160, 404, 21, LIGHTGRAY);
    DrawVN(font, u8"M: Trở về menu",
           160, 446, 21, LIGHTGRAY);

    if (DrawButton(font, {309, 552, 250, 58}, u8"QUAY LẠI",
                   {54, 92, 151, 255},
                   {71, 132, 201, 255})) {
        screen = MAIN_MENU;
    }
}

// Vẽ sáu thẻ nhân vật và cập nhật selectedCharacter khi người dùng nhấn.
void DrawCharacterSelection(Font font,
                            const CharacterSheet& characterSheet,
                            int& selectedCharacter,
                            ScreenState& screen,
                            bool& startGameRequested) {
    // Nền riêng cho màn hình chọn nhân vật.
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {24, 31, 61, 255},
                           {9, 14, 30, 255});

    DrawVN(font, u8"CHỌN NHÂN VẬT",
           SCREEN_W / 2.0f, 25, 39,
           {255, 221, 91, 255}, true);
    DrawVN(font, u8"Nhấp vào nhân vật mà bạn muốn sử dụng",
           SCREEN_W / 2.0f, 76, 20,
           LIGHTGRAY, true);

    // Vòng lặp tạo sáu thẻ từ chỉ số 0 đến 5.
    for (int i = 0; i < CHARACTER_COUNT; i++) {
        // i % 3 cho ra cột 0, 1, 2.
        // i / 3 cho ra hàng 0 hoặc 1.
        int column = i % 3;
        int row = i / 3;

        Rectangle card = {
            (float)(91 + column * 230),
            (float)(120 + row * 205),
            180, 174
        };

        bool hover = CheckCollisionPointRec(GetMousePosition(), card);
        bool selected = selectedCharacter == i;
        Color accent = CHARACTER_COLORS[i];

        // Ưu tiên màu đã chọn, tiếp theo là màu hover, cuối cùng là màu thường.
        Color cardColor = selected
            ? Fade(accent, 0.42f)
            : (hover ? Color{42, 54, 92, 255}
                     : Color{29, 38, 70, 255});

        DrawRectangleRounded(card, 0.10f, 8, cardColor);

        // Họa tiết caro mờ làm nền thẻ đỡ trống.
        for (int patternY = 0; patternY < 5; patternY++) {
            for (int patternX = 0; patternX < 6; patternX++) {
                if ((patternX + patternY) % 2 == 0) {
                    DrawRectangle(
                        (int)card.x + 8 + patternX * 27,
                        (int)card.y + 8 + patternY * 24,
                        13, 12, Fade(accent, 0.07f)
                    );
                }
            }
        }

        // Thẻ đang chọn có viền dày và mang màu riêng của nhân vật.
        DrawRectangleLinesEx(
            card,
            selected ? 4 : 2,
            selected ? accent : Fade(WHITE, 0.25f)
        );

        // Vẽ ảnh và tên nhân vật lên thẻ.
        DrawCharacterFromSheet(characterSheet, i,
                               card.x + 25, card.y + 10,
                               130, 120);
        DrawVN(font, CHARACTER_NAMES[i],
               card.x + card.width / 2.0f,
               card.y + 138, 21,
               selected ? accent : WHITE, true);

        if (selected) {
            DrawVN(font, u8"ĐÃ CHỌN",
                   card.x + card.width / 2.0f,
                   card.y + 160, 12, WHITE, true);
        }

        // Gán i vào selectedCharacter khi người dùng nhấn thẻ.
        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedCharacter = i;
        }
    }

    // Trở về menu mà không bắt đầu game.
    if (DrawButton(font, {115, 552, 230, 58}, u8"QUAY LẠI",
                   {67, 72, 104, 255},
                   {91, 101, 142, 255})) {
        screen = MAIN_MENU;
    }

    // Báo main() chuẩn bị ván mới rồi chuyển sang màn hình chơi.
    if (DrawButton(font, {523, 552, 230, 58}, u8"VÀO MÊ CUNG",
                   {35, 133, 90, 255},
                   {49, 181, 118, 255})) {
        startGameRequested = true;
        screen = PLAYING_SCREEN;
    }
}

// Vẽ màn hình chơi mẫu để người 4 kiểm tra HUD và nhân vật.
// Game chính sẽ thay phần này bằng map, vật phẩm và quái thật.
void DrawPlayingPreview(Font font,
                        const CharacterSheet& characterSheet,
                        int selectedCharacter) {
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {20, 44, 70, 255},
                           {8, 15, 28, 255});

    // HUD giả lập ở đầu màn hình.
    DrawRectangle(0, 0, SCREEN_W, 90, {12, 30, 54, 255});
    DrawRectangle(0, 85, SCREEN_W, 5,
                  CHARACTER_COLORS[selectedCharacter]);

    DrawVN(font, u8"MAZE RUNNER", 18, 12, 26, WHITE);
    DrawVN(font, CHARACTER_NAMES[selectedCharacter],
           20, 51, 19, CHARACTER_COLORS[selectedCharacter]);
    DrawVN(font, u8"BẢN DEMO RIÊNG CỦA NGƯỜI 4",
           SCREEN_W / 2.0f, 24, 22, LIGHTGRAY, true);

    // Bảng giữa màn hình chứa nhân vật đã chọn.
    Rectangle previewPanel = {274, 130, 320, 365};
    DrawRectangleRounded(previewPanel, 0.08f, 8,
                         {26, 36, 66, 245});
    DrawRectangleLinesEx(previewPanel, 3,
                         CHARACTER_COLORS[selectedCharacter]);

    DrawCharacterFromSheet(characterSheet, selectedCharacter,
                           334, 168, 200, 260);
    DrawVN(font, CHARACTER_NAMES[selectedCharacter],
           SCREEN_W / 2.0f, 442, 26,
           CHARACTER_COLORS[selectedCharacter], true);

    DrawVN(font, u8"Nhấn W để xem bảng thắng",
           SCREEN_W / 2.0f, 530, 19, WHITE, true);
    DrawVN(font, u8"Nhấn L để xem bảng thua | M để về menu",
           SCREEN_W / 2.0f, 565, 19, LIGHTGRAY, true);
}

// Vẽ lớp phủ kết quả lên trên màn hình chơi.
// Hàm trả về true nếu người dùng nhấn nút Chơi lại.
bool DrawResultScreen(Font font, bool win, int score,
                      ScreenState& screen) {
    // Màu đen có alpha 0.62 làm phần phía sau tối đi.
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.62f));

    Rectangle panel = {184, 190, 500, 290};
    DrawRectangleRounded(panel, 0.08f, 8, {24, 32, 60, 250});
    DrawRectangleLinesEx(panel, 3, win ? GREEN : RED);

    // Toán tử ba ngôi chọn nội dung và màu dựa trên biến win.
    DrawVN(font,
           win ? u8"BẠN ĐÃ THOÁT KHỎI MÊ CUNG!"
               : u8"BẠN ĐÃ BỊ QUỶ BẮT!",
           SCREEN_W / 2.0f, 230, 29,
           win ? GREEN : RED, true);

    DrawVN(font, TextFormat(u8"Điểm đạt được: %d", score),
           SCREEN_W / 2.0f, 286, 23, WHITE, true);

    // Nhấn Chơi lại: trả true để main() ẩn bảng kết quả.
    if (DrawButton(font, {224, 355, 195, 58}, u8"CHƠI LẠI",
                   {39, 121, 184, 255},
                   {56, 159, 222, 255})) {
        return true;
    }

    // Nhấn Về menu: đổi trạng thái màn hình.
    if (DrawButton(font, {449, 355, 195, 58}, u8"VỀ MENU",
                   {105, 62, 137, 255},
                   {144, 83, 182, 255})) {
        screen = MAIN_MENU;
    }

    return false;
}

// Hiện lỗi nếu chương trình không tìm thấy characters.png.
void DrawMissingCharacterImage(Font font) {
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H,
                           {53, 20, 31, 255},
                           {16, 10, 22, 255});

    DrawVN(font, u8"KHÔNG TÌM THẤY ẢNH NHÂN VẬT",
           SCREEN_W / 2.0f, 170, 36, RED, true);
    DrawVN(font, u8"Hãy đặt characters.png cạnh file exe",
           SCREEN_W / 2.0f, 260, 23, WHITE, true);
    DrawVN(font, u8"hoặc đặt ở thư mục cha của file exe.",
           SCREEN_W / 2.0f, 305, 23, WHITE, true);
    DrawVN(font, u8"Nhấn ESC để đóng chương trình.",
           SCREEN_W / 2.0f, 390, 20, LIGHTGRAY, true);
}

// ================================= MAIN =====================================

int main() {
    // Tạo cửa sổ và giới hạn chương trình ở 60 khung hình mỗi giây.
    InitWindow(SCREEN_W, SCREEN_H, "Maze Runner - Phan giao dien nguoi 4");
    SetTargetFPS(60);

    // Tải hai tài nguyên do người 4 quản lý: font và ảnh nhân vật.
    bool loadedCustomFont = false;
    Font vietnameseFont = LoadVietnameseFont(loadedCustomFont);
    CharacterSheet characterSheet = LoadCharacterSheet();

    // Khi mở chương trình, màn hình đầu tiên luôn là menu chính.
    ScreenState screen = MAIN_MENU;

    // Các biến giao diện được truyền tham chiếu vào những hàm phía trên.
    bool quitRequested = false;
    bool startGameRequested = false;
    int selectedCharacter = NOBITA;

    // Hai biến dưới chỉ dùng để thử màn hình kết quả trong bản demo.
    bool showResult = false;
    bool demoWin = true;

    // Vòng lặp chạy tới khi người dùng đóng cửa sổ hoặc nhấn nút Thoát.
    while (!WindowShouldClose() && !quitRequested) {
        // Trong màn hình chơi mẫu:
        // W mở kết quả thắng, L mở kết quả thua, M quay về menu.
        if (screen == PLAYING_SCREEN && !showResult) {
            if (IsKeyPressed(KEY_W)) {
                demoWin = true;
                showResult = true;
            }
            if (IsKeyPressed(KEY_L)) {
                demoWin = false;
                showResult = true;
            }
            if (IsKeyPressed(KEY_M)) {
                screen = MAIN_MENU;
            }
        }

        // Trong game chính, cờ này được dùng để gọi ResetGame().
        // Bản demo không có gameplay nên chỉ cần đặt lại cờ về false.
        if (startGameRequested) {
            showResult = false;
            startGameRequested = false;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Thiếu ảnh thì thay toàn bộ giao diện bằng màn hình báo lỗi.
        if (!characterSheet.loaded) {
            DrawMissingCharacterImage(vietnameseFont);
        }
        // Mỗi giá trị screen tương ứng với một hàm vẽ.
        else if (screen == MAIN_MENU) {
            DrawMainMenu(vietnameseFont, screen, quitRequested);
        }
        else if (screen == GUIDE_SCREEN) {
            DrawGuide(vietnameseFont, screen);
        }
        else if (screen == CHARACTER_SCREEN) {
            DrawCharacterSelection(vietnameseFont, characterSheet,
                                   selectedCharacter,
                                   screen, startGameRequested);
        }
        else if (screen == PLAYING_SCREEN) {
            DrawPlayingPreview(vietnameseFont, characterSheet,
                               selectedCharacter);

            // Kết quả được vẽ cuối nên nằm trên tất cả thành phần khác.
            if (showResult) {
                bool replay = DrawResultScreen(vietnameseFont,
                                               demoWin, 500, screen);
                if (replay) showResult = false;
            }
        }

        EndDrawing();
    }

    // Giải phóng đúng những tài nguyên đã tải thành công.
    if (characterSheet.loaded) {
        UnloadTexture(characterSheet.texture);
    }
    if (loadedCustomFont) {
        UnloadFont(vietnameseFont);
    }

    CloseWindow();
    return 0;
}

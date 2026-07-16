
#include "raylib.h"


#include <algorithm>
#include <cmath>

// Hai thư viện này dùng để lưu map và làm đường di chuyển cho quái vật
#include <queue>
#include <vector>


#include <random>

// Thư viện này để lưu mê cung dạng chuỗi
#include <string>


using namespace std;

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

enum ScreenState {
    MAIN_MENU,
    GUIDE_SCREEN,
    CHARACTER_SCREEN,
    PLAYING_SCREEN
};

enum CharacterType {
    NOBITA,
    JAIAN,
    SUNEO,
    SHIZUKA,
    DORAEMON,
    DEKISUGI
};

struct Enemy {
    int r, c;
    float timer;
};

struct Item {
    int r, c;
    bool collected;
};

struct MapTheme {
    Color wall;
    Color wallTop;
    Color wallShadow;
    Color floorA;
    Color floorB;
    Color accent;
    Color background;
};

struct CharacterSheet {
    Texture2D texture;
    bool loaded;
};

const char* CHARACTER_NAMES[CHARACTER_COUNT] = {
    u8"Nobita", u8"Jaian", u8"Suneo",
    u8"Shizuka", u8"Doraemon", u8"Dekisugi"
};

const Color CHARACTER_COLORS[CHARACTER_COUNT] = {
    {250, 208, 45, 255}, {239, 112, 35, 255}, {77, 180, 110, 255},
    {241, 112, 160, 255}, {32, 151, 219, 255}, {82, 156, 220, 255}
};

const char* MAP_NAMES[MAP_COUNT] = {
    u8"Thành phố xanh", u8"Thung lũng tím", u8"Rừng ngọc bích",
    u8"Sa mạc hoàng hôn", u8"Đại dương sâu", u8"Đền cổ",
    u8"Vương quốc kẹo", u8"Núi lửa đỏ", u8"Băng nguyên",
    u8"Khu vườn đêm", u8"Phòng thí nghiệm", u8"Lâu đài bóng tối"
};

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
};

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
    };

    vector<int> codepoints = CreateVietnameseCodepoints();
    for (const char* path : fontPaths) {
        if (FileExists(path)) {
            Font font = LoadFontEx(path, 38, codepoints.data(), (int)codepoints.size());
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            loadedCustomFont = true;
            return font;
        }
    }

    loadedCustomFont = false;
    return GetFontDefault();
}

void DrawVN(Font font, const char* text, float x, float y,
            float size, Color color, bool centered = false) {
    Vector2 textSize = MeasureTextEx(font, text, size, 1.0f);
    if (centered) x -= textSize.x / 2.0f;
    DrawTextEx(font, text, {x, y}, size, 1.0f, color);
}

// Tạo mê cung hoàn chỉnh bằng DFS. Mỗi seed cho ra một bản đồ cố định khác nhau.
// DFS ở đây hiểu đơn giản là: đi sâu vào một hướng trước, nếu kẹt thì quay lui lại ô trước đó.
// Hàm này trả về một mê cung dạng vector<string>, tức là một danh sách nhiều dòng chữ.
// Mỗi string là một hàng của bản đồ, mỗi ký tự trong string là một ô của mê cung.
vector<string> GenerateMaze(unsigned int seed) {
    // Tạo biến maze để lưu mê cung.
    // ROWS là số hàng của mê cung.
    // string(COLS, '#') tạo ra một dòng có COLS ký tự, tất cả đều là '#'.
    // Ký tự '#' nghĩa là tường.
    // Vậy dòng này tạo ra một bản đồ ban đầu toàn là tường.
    vector<string> maze(ROWS, string(COLS, '#'));

    // mt19937 là bộ sinh số ngẫu nhiên trong C++.
    // rng là tên biến dùng để random.
    // seed giúp việc random có thể lặp lại: cùng một seed thì tạo ra cùng một mê cung.
    mt19937 rng(seed);

    // Tạo một stack để lưu các ô đang xét trong quá trình tạo mê cung.
    // Ở đây dùng vector<pair<int, int>> thay cho stack.
    // pair<int, int> dùng để lưu tọa độ của một ô: số thứ nhất là hàng, số thứ hai là cột.
    vector<pair<int, int>> stack;

    // Đưa ô bắt đầu hàng 1, cột 1 vào stack.
    // Đây là vị trí đầu tiên mà thuật toán bắt đầu đào đường.
    stack.push_back({1, 1});

    // Đổi ô hàng 1, cột 1 từ tường '#' thành đường đi '.'.
    // Ký tự '.' nghĩa là ô có thể đi được.
    maze[1][1] = '.';

    // Vòng lặp chạy khi stack vẫn còn ô cần xử lý.
    // Khi stack rỗng nghĩa là không còn ô nào để đào tiếp, mê cung đã tạo xong.
    while (!stack.empty()) {
        // Lấy hàng của ô hiện tại.
        // stack.back() là ô cuối cùng trong stack, tức ô đang đứng hiện tại.
        // first là giá trị thứ nhất trong pair, ở đây là hàng.
        int r = stack.back().first;

        // Lấy cột của ô hiện tại.
        // second là giá trị thứ hai trong pair, ở đây là cột.
        int c = stack.back().second;

        // Tạo danh sách 4 hướng di chuyển.
        // {-2, 0} là đi lên 2 ô.
        // {2, 0} là đi xuống 2 ô.
        // {0, -2} là đi sang trái 2 ô.
        // {0, 2} là đi sang phải 2 ô.
        // Đi 2 ô vì mê cung cần giữ lại một bức tường nằm giữa hai đường đi.
        vector<pair<int, int>> directions = {
            {-2, 0}, {2, 0}, {0, -2}, {0, 2}
        };

        // Xáo trộn thứ tự 4 hướng di chuyển.
        // Nếu không xáo trộn thì mê cung sẽ bị giống nhau và ít tự nhiên.
        // rng là bộ random đã tạo ở trên.
        shuffle(directions.begin(), directions.end(), rng);

        // Biến moved dùng để kiểm tra từ ô hiện tại có đào sang ô mới được không.
        // Ban đầu gán false nghĩa là chưa di chuyển được.
        bool moved = false;

        // Duyệt từng hướng trong danh sách directions.
        // const nghĩa là không sửa trực tiếp giá trị direction trong vòng lặp.
        // pair<int, int>& nghĩa là lấy từng cặp tọa độ hướng đi.
        for (const pair<int, int>& direction : directions) {
            // Tính hàng mới dự định đi tới.
            // direction.first có thể là -2, 2 hoặc 0.
            int nr = r + direction.first;

            // Tính cột mới dự định đi tới.
            // direction.second có thể là -2, 2 hoặc 0.
            int nc = c + direction.second;

            // Kiểm tra ô mới có hợp lệ để đào đường hay không.
            // nr > 0: không cho đi ra hàng viền trên.
            // nr < ROWS - 1: không cho đi ra hàng viền dưới.
            // nc > 0: không cho đi ra cột viền trái.
            // nc < COLS - 1: không cho đi ra cột viền phải.
            // maze[nr][nc] == '#': ô mới phải còn là tường thì mới đào được.
            // Nếu ô mới đã là đường '.' rồi thì không đào lại để tránh làm rối thuật toán.
            if (nr > 0 && nr < ROWS - 1 && nc > 0 && nc < COLS - 1 &&
                maze[nr][nc] == '#') {
                // Đào ô nằm giữa ô hiện tại và ô mới.
                // Vì mỗi lần đi xa 2 ô nên cần mở thêm ô ở giữa để nối đường.
                // direction.first / 2 và direction.second / 2 giúp lấy tọa độ ô giữa.
                maze[r + direction.first / 2][c + direction.second / 2] = '.';

                // Đào ô mới, đổi từ tường '#' thành đường đi '.'.
                maze[nr][nc] = '.';

                // Thêm ô mới vào stack.
                // Sau đó thuật toán sẽ tiếp tục đào đường từ ô mới này.
                stack.push_back({nr, nc});

                // Đánh dấu là đã di chuyển/đào được sang ô mới.
                moved = true;

                // Dừng vòng for vì ở lượt này chỉ cần chọn một hướng để đi tiếp.
                break;
            }
        }

        // Nếu không đào được sang hướng nào thì ô hiện tại là đường cụt.
        // Khi đó pop_back() sẽ xóa ô hiện tại khỏi stack để quay lui về ô trước đó.
        // Đây là phần quay lui của thuật toán DFS.
        if (!moved) stack.pop_back();
    }

    // Sau khi DFS tạo xong mê cung cơ bản, tạo danh sách các bức tường có thể phá thêm.
    // Mục đích là làm bản đồ có nhiều đường vòng hơn, không quá đơn điệu.
    vector<pair<int, int>> removableWalls;

    // Duyệt các hàng bên trong mê cung, không duyệt sát viền ngoài.
    // Bắt đầu từ 2 và kết thúc trước ROWS - 2 để tránh phá tường biên.
    for (int r = 2; r < ROWS - 2; r++) {
        // Duyệt các cột bên trong mê cung, không duyệt sát viền ngoài.
        for (int c = 2; c < COLS - 2; c++) {
            // Nếu ô hiện tại không phải tường thì bỏ qua.
            // continue nghĩa là bỏ qua lần lặp hiện tại và xét ô tiếp theo.
            if (maze[r][c] != '#') continue;

            // Kiểm tra tường này có nằm giữa hai đường đi theo chiều dọc không.
            // Nếu ô phía trên là '.' và ô phía dưới là '.' thì phá tường này sẽ nối được hai đường.
            bool vertical = maze[r - 1][c] == '.' && maze[r + 1][c] == '.';

            // Kiểm tra tường này có nằm giữa hai đường đi theo chiều ngang không.
            // Nếu ô bên trái là '.' và ô bên phải là '.' thì phá tường này sẽ nối được hai đường.
            bool horizontal = maze[r][c - 1] == '.' && maze[r][c + 1] == '.';

            // Nếu tường có thể nối đường dọc hoặc nối đường ngang thì thêm vào danh sách có thể phá.
            if (vertical || horizontal) removableWalls.push_back({r, c});
        }
    }

    // Xáo trộn danh sách các tường có thể phá.
    // Nhờ vậy những bức tường được phá thêm sẽ khác nhau theo từng seed.
    shuffle(removableWalls.begin(), removableWalls.end(), rng);

    // Chọn số lượng tường sẽ phá thêm.
    // min(18, removableWalls.size()) nghĩa là phá tối đa 18 tường.
    // Nếu danh sách có ít hơn 18 tường thì chỉ phá đúng số lượng đang có.
    int openings = min(18, (int)removableWalls.size());

    // Duyệt qua các bức tường được chọn để phá.
    for (int i = 0; i < openings; i++) {
        // removableWalls[i].first là hàng của bức tường.
        // removableWalls[i].second là cột của bức tường.
        // Đổi ô đó từ '#' thành '.' nghĩa là phá tường thành đường đi.
        maze[removableWalls[i].first][removableWalls[i].second] = '.';
    }

    // Đặt ký tự 'S' ở hàng 1, cột 1.
    // S là Start, tức điểm bắt đầu của người chơi.
    maze[1][1] = 'S';

    // Đặt ký tự 'E' ở gần góc dưới bên phải của mê cung.
    // ROWS - 2 là hàng gần cuối, COLS - 2 là cột gần cuối.
    // E là Exit, tức cửa thoát.
    maze[ROWS - 2][COLS - 2] = 'E';

    // Trả về mê cung đã tạo xong để các phần khác của game sử dụng.
    return maze;
}

// Hàm BuildMazeList() dùng để tạo ra danh sách tất cả các bản đồ trong game.
// Ở bản đầu, mình thường viết sẵn một map cố định bằng vector<string>.
// Nhưng ở bản hoàn chỉnh này, map được tạo tự động bằng hàm GenerateMaze().
// Vì game có nhiều bản đồ, nên cần một hàm riêng để tạo và gom tất cả map vào một danh sách.
// Hàm này trả về vector<vector<string>>.
// Hiểu đơn giản:
// - vector<string> là 1 bản đồ, gồm nhiều dòng chữ.
// - vector<vector<string>> là danh sách nhiều bản đồ.
// Ví dụ:
// maps[0] là bản đồ số 1.
// maps[1] là bản đồ số 2.
// maps[2] là bản đồ số 3.
// ...
vector<vector<string>> BuildMazeList() {
    // Tạo một mảng seeds để chứa các số seed dùng cho việc tạo map.
    // unsigned int là kiểu số nguyên không âm.
    // Nghĩa là các seed này không dùng số âm, chỉ dùng số dương hoặc 0.
    //
    // MAP_COUNT là số lượng bản đồ trong game.
    // Ở phía trên file có khai báo:
    // const int MAP_COUNT = 12;
    //
    // Vậy dòng này có thể hiểu là:
    // tạo một mảng seeds có 12 phần tử.
    //
    // Mỗi số seed sẽ được đưa vào hàm GenerateMaze(seed).
    // Cùng một seed thì GenerateMaze() sẽ tạo lại đúng một mê cung giống nhau.
    // Seed khác nhau thì mê cung tạo ra sẽ khác nhau.
    //
    // Ví dụ:
    // seed 41 tạo ra map 1.
    // seed 73 tạo ra map 2.
    // seed 109 tạo ra map 3.
    //
    // Nhờ có seed cố định, game vừa có cảm giác nhiều map khác nhau,
    // vừa đảm bảo mỗi lần chạy lại chương trình thì 12 map vẫn ổn định.
    unsigned int seeds[MAP_COUNT] = {
        // Đây là 12 số seed tương ứng với 12 bản đồ.
        // Các số này không có ý nghĩa đặc biệt về mặt gameplay.
        // Chúng chỉ là các giá trị đầu vào để bộ random trong GenerateMaze()
        // tạo ra những kiểu mê cung khác nhau.
        41, 73, 109, 151, 197, 251, 313, 401, 503, 617, 733, 887
    };

    // Tạo biến maps để lưu danh sách các bản đồ.
    //
    // Kiểu dữ liệu vector<vector<string>> nhìn hơi khó, nhưng tách ra sẽ dễ hiểu:
    //
    // vector<string>:
    //     là 1 bản đồ.
    //     Mỗi string là 1 hàng trong mê cung.
    //
    // vector<vector<string>>:
    //     là nhiều bản đồ.
    //     Mỗi phần tử bên trong nó là một vector<string>.
    //
    // Ví dụ:
    // maps[0] = bản đồ thứ nhất.
    // maps[1] = bản đồ thứ hai.
    // maps[2] = bản đồ thứ ba.
    //
    // Ban đầu maps đang rỗng, chưa có bản đồ nào.
    vector<vector<string>> maps;

    // Vòng lặp for dùng để tạo đủ số lượng bản đồ theo MAP_COUNT.
    //
    // int i = 0:
    //     bắt đầu từ i bằng 0.
    //
    // i < MAP_COUNT:
    //     vòng lặp chạy khi i còn nhỏ hơn MAP_COUNT.
    //     Vì MAP_COUNT = 12 nên i sẽ chạy từ 0 đến 11.
    //
    // i++:
    //     sau mỗi lần lặp thì tăng i lên 1.
    //
    // Nói dễ hiểu:
    // vòng lặp này sẽ chạy 12 lần để tạo 12 map.
    for (int i = 0; i < MAP_COUNT; i++) {
        // Gọi hàm GenerateMaze(seeds[i]) để tạo ra một bản đồ mới.
        //
        // seeds[i] nghĩa là lấy seed tại vị trí i trong mảng seeds.
        //
        // Khi i = 0:
        //     seeds[0] = 41
        //     GenerateMaze(41) tạo map đầu tiên.
        //
        // Khi i = 1:
        //     seeds[1] = 73
        //     GenerateMaze(73) tạo map thứ hai.
        //
        // Khi i = 2:
        //     seeds[2] = 109
        //     GenerateMaze(109) tạo map thứ ba.
        //
        // Cứ như vậy cho tới i = 11.
        //
        // Hàm GenerateMaze() trả về kiểu vector<string>,
        // tức là trả về một bản đồ hoàn chỉnh.
        //
        // maps.push_back(...) nghĩa là thêm bản đồ vừa tạo vào cuối danh sách maps.
        //
        // Sau dòng này:
        // - lần lặp đầu tiên: maps có 1 map.
        // - lần lặp thứ hai: maps có 2 map.
        // - lần lặp thứ ba: maps có 3 map.
        // - ...
        // - sau 12 lần: maps có đủ 12 map.
        maps.push_back(GenerateMaze(seeds[i]));
    }

    // Sau khi tạo xong toàn bộ bản đồ, trả về danh sách maps.
    //
    // Các phần khác của game sẽ dùng danh sách này để:
    // - chọn map ngẫu nhiên khi bấm R.
    // - chuyển sang map kế tiếp khi bấm N.
    // - lấy map hiện tại để vẽ ra màn hình.
    //
    // Nói ngắn gọn:
    // return maps; nghĩa là gửi danh sách 12 bản đồ ra ngoài hàm.
    return maps;
}

// Tạo hàm "IsWall" để kiểm tra lại ô hàng r, cột c có phải tường không
// maze là bản đồ
// r là hàng
// c là cột
// bool là hàm để trả về true hoặc false
bool IsWall(const vector<string>& maze, int r, int c) {

    // r < 0 là hàng không được nhỏ hơn 0. Ví dụ người chơi đang đứng ở vị trí hàng số 0 thì không được nhỏ hơn nữa
    // r >= Rows là không được quá hàng (r) thứ 21. Vì hàng cuối cùng chỉ là 20
    // c < 0 là cột không được nhỏ hơn 0. Ví dụ người chơi đang đứng ở vị trí cột số 0 thì không được nhỏ hơn nữa
    // c >= COLS là không được quá cột (c) thứ 31. Vì cột cuối cùng chỉ là 30
    // Giải thích tổng phần code này: Điều kiện if nằm trong hàm bool IsWall là trả về đúng hoặc sai. Nếu điền kiện trong if đúng thì trả về true là Tường => Không được đi tiếp
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return true;
    return maze[r][c] == '#';
}

// Hàm này để kiểm tra người chơi và quái có được đi vào ô đó không
// !IsWall có nghĩa là phủ định của hàm IsWall nghĩa là được đi
// Giải thích tổng: Hàm này kiểm tra xem có phải là tường không? Nếu không phải thì được đi
bool CanMove(const vector<string>& maze, int r, int c) {
    return !IsWall(maze, r, c);
}

vector<pair<int, int>> FindPath(const vector<string>& maze,
                                int sr, int sc, int tr, int tc) {
    vector<vector<int>> visited(ROWS, vector<int>(COLS, 0));
    vector<vector<pair<int, int>>> parent(
        ROWS, vector<pair<int, int>>(COLS, {-1, -1})
    );
    queue<pair<int, int>> cells;
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

// Hàm FindStartAndExit dùng để tìm vị trí bắt đầu và vị trí cửa thoát trong mê cung.
// Lý do cần hàm này:
// - Ở các bản đầu, vị trí người chơi và cửa thoát có thể được gán cố định, ví dụ playerR = 1, playerC = 1.
// - Nhưng trong bản hoàn chỉnh, mê cung được tạo tự động bởi GenerateMaze().
// - Vì vậy chương trình cần tự quét bản đồ để xem ký tự 'S' nằm ở đâu và ký tự 'E' nằm ở đâu.
// - 'S' là Start, tức điểm bắt đầu của người chơi.
// - 'E' là Exit, tức cửa thoát để người chơi đi tới và chiến thắng.
//
// Kiểu trả về của hàm là void.
// void nghĩa là hàm không trả về giá trị bằng return như int, bool hay vector.
// Thay vào đó, hàm sẽ sửa trực tiếp các biến playerR, playerC, exitR, exitC thông qua tham chiếu &.
void FindStartAndExit(const vector<string>& maze,
                      int& playerR, int& playerC, int& exitR, int& exitC) {
    // Tham số maze là bản đồ mê cung cần kiểm tra.
    // const nghĩa là hàm này chỉ đọc bản đồ, không được sửa nội dung bản đồ.
    // vector<string>& nghĩa là truyền bản đồ bằng tham chiếu để không phải copy cả bản đồ lớn.
    // playerR là biến lưu hàng của người chơi.
    // playerC là biến lưu cột của người chơi.
    // exitR là biến lưu hàng của cửa thoát.
    // exitC là biến lưu cột của cửa thoát.
    // Dấu & ở int& nghĩa là truyền tham chiếu.
    // Nhờ có dấu &, khi gán playerR = r trong hàm này thì biến playerR ở bên ngoài cũng thay đổi theo.

    // Vòng for này dùng để duyệt qua từng hàng của mê cung.
    // int r = 0 nghĩa là bắt đầu từ hàng đầu tiên, có chỉ số là 0.
    // r < ROWS nghĩa là còn nhỏ hơn tổng số hàng thì còn tiếp tục duyệt.
    // ROWS trong game là 21, nên r sẽ chạy từ 0 đến 20.
    // r++ nghĩa là sau mỗi lần lặp thì tăng r lên 1 để chuyển sang hàng kế tiếp.
    for (int r = 0; r < ROWS; r++) {
        // Vòng for bên trong dùng để duyệt qua từng cột của hàng hiện tại.
        // int c = 0 nghĩa là bắt đầu từ cột đầu tiên, có chỉ số là 0.
        // c < COLS nghĩa là còn nhỏ hơn tổng số cột thì còn tiếp tục duyệt.
        // COLS trong game là 31, nên c sẽ chạy từ 0 đến 30.
        // c++ nghĩa là sau mỗi lần lặp thì tăng c lên 1 để chuyển sang cột kế tiếp.
        for (int c = 0; c < COLS; c++) {
            // maze[r][c] là ký tự nằm tại hàng r, cột c của bản đồ.
            // Ví dụ maze[1][1] là ô ở hàng 1, cột 1.
            // Nếu ký tự tại ô hiện tại là 'S' thì đây là vị trí bắt đầu của người chơi.
            // Dùng == để so sánh xem hai giá trị có bằng nhau không.
            if (maze[r][c] == 'S') {
                // Gán hàng hiện tại r cho playerR.
                // Nghĩa là lưu lại người chơi đang bắt đầu ở hàng nào.
                // Vì playerR được truyền bằng tham chiếu int& nên giá trị này sẽ được giữ lại sau khi hàm kết thúc.
                playerR = r;

                // Gán cột hiện tại c cho playerC.
                // Nghĩa là lưu lại người chơi đang bắt đầu ở cột nào.
                // Sau dòng này, chương trình biết chính xác vị trí xuất hiện ban đầu của người chơi.
                playerC = c;
            } else if (maze[r][c] == 'E') {
                // else if nghĩa là: nếu ô hiện tại không phải 'S' thì mới kiểm tra tiếp nó có phải 'E' không.
                // Nếu ký tự tại ô hiện tại là 'E' thì đây là vị trí cửa thoát.
                // 'E' là Exit, người chơi đi tới ô này thì thắng.

                // Gán hàng hiện tại r cho exitR.
                // Nghĩa là lưu lại cửa thoát đang nằm ở hàng nào.
                // Biến này cũng truyền bằng tham chiếu nên sau khi ra khỏi hàm vẫn còn giá trị đã tìm được.
                exitR = r;

                // Gán cột hiện tại c cho exitC.
                // Nghĩa là lưu lại cửa thoát đang nằm ở cột nào.
                // Sau dòng này, chương trình biết vị trí cần tới để chiến thắng.
                exitC = c;
            }
        }
    }

    // Sau khi hai vòng for chạy xong, toàn bộ mê cung đã được quét qua.
    // Nếu trong bản đồ có 'S', playerR và playerC đã được cập nhật thành vị trí bắt đầu.
    // Nếu trong bản đồ có 'E', exitR và exitC đã được cập nhật thành vị trí cửa thoát.
    // Hàm không cần return vì các biến cần lấy kết quả đã được sửa trực tiếp qua tham chiếu &.
}

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

void ResetGame(const vector<vector<string>>& mazeList,
               vector<string>& maze, int& playerR, int& playerC,
               int& exitR, int& exitC, Enemy& enemy, vector<Item>& items,
               int& score, int& itemsCollected, int& currentMap,
               bool& win, bool& gameOver, float& playerSpeed,
               float& moveCharge, float& boostMessageTimer, int selectedMap) {
    if (selectedMap < 0) {
        currentMap = GetRandomValue(0, (int)mazeList.size() - 1);
    } else {
        currentMap = selectedMap % (int)mazeList.size();
    }

    maze = mazeList[currentMap];
    FindStartAndExit(maze, playerR, playerC, exitR, exitC);
    PlaceEnemy(maze, enemy, playerR, playerC, exitR, exitC);
    AddRandomItems(maze, items, playerR, playerC, exitR, exitC, enemy.r, enemy.c);
    score = 0;
    itemsCollected = 0;
    playerSpeed = 1.0f;
    moveCharge = 0.0f;
    boostMessageTimer = 0.0f;
    win = false;
    gameOver = false;
}

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

void DrawPixelShadow(float x, float y, float scale) {
    Color shadow = {8, 12, 24, 95};
    PixelRect(x, y, scale, 7, 26, 14, 2, shadow);
    PixelRect(x, y, scale, 5, 27, 18, 1, shadow);
}

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

void DrawCharacter(int character, float x, float y, float scale) {
    if (character == NOBITA) DrawNobitaPixel(x, y, scale);
    else if (character == JAIAN) DrawJaianPixel(x, y, scale);
    else if (character == SUNEO) DrawSuneoPixel(x, y, scale);
    else if (character == SHIZUKA) DrawShizukaPixel(x, y, scale);
    else if (character == DORAEMON) DrawDoraemonPixel(x, y, scale);
    else DrawDekisugiPixel(x, y, scale);
}

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

void DrawExit(int r, int c, Color accent) {
    int x = c * TILE + TILE / 2;
    int y = r * TILE + UI_HEIGHT + TILE / 2;
    float pulse = 1.0f + 0.10f * sinf((float)GetTime() * 4.0f);
    DrawCircle(x, y, 13 * pulse, Fade(accent, 0.20f));
    DrawCircle(x, y, 10, accent);
    DrawCircle(x, y, 6, Fade(WHITE, 0.75f));
    DrawCircle(x, y, 3, accent);
}

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

int main() {
    InitWindow(SCREEN_W, SCREEN_H, "Maze Runner");
    SetTargetFPS(60);

    bool loadedCustomFont = false;
    Font vietnameseFont = LoadVietnameseFont(loadedCustomFont);
    CharacterSheet characterSheet = LoadCharacterSheet();
    vector<vector<string>> mazeList = BuildMazeList();

    ScreenState screen = MAIN_MENU;
    bool quitRequested = false;
    bool startGameRequested = false;
    int selectedCharacter = NOBITA;

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

    while (!WindowShouldClose() && !quitRequested) {
        if (!characterSheet.loaded) {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawMissingCharacterImage(vietnameseFont);
            EndDrawing();
            continue;
        }

        if (startGameRequested) {
            ResetGame(mazeList, maze, playerR, playerC, exitR, exitC,
                      enemy, items, score, itemsCollected, currentMap,
                      win, gameOver, playerSpeed, moveCharge,
                      boostMessageTimer, -1);
            startGameRequested = false;
        }

        // Kiểm tra xem màn hình hiện tại có phải là màn hình đang chơi game không.
        //
        // screen là biến lưu trạng thái màn hình hiện tại của chương trình.
        // Ở phía trên file có enum ScreenState gồm:
        // - MAIN_MENU: màn hình menu chính.
        // - GUIDE_SCREEN: màn hình hướng dẫn.
        // - CHARACTER_SCREEN: màn hình chọn nhân vật.
        // - PLAYING_SCREEN: màn hình đang chơi trong mê cung.
        //
        // Dấu == dùng để so sánh.
        // screen == PLAYING_SCREEN nghĩa là:
        // nếu người chơi đang ở màn hình chơi game thì mới xử lý các phím điều khiển trong game.
        //
        // Lý do cần kiểm tra điều kiện này:
        // - Khi đang ở menu, bấm R hoặc N không nên đổi map.
        // - Chỉ khi thật sự đang chơi thì R mới restart/random map, N mới đổi sang map kế tiếp.
        if (screen == PLAYING_SCREEN) {
            // Kiểm tra người chơi có bấm phím M không.
            //
            // IsKeyPressed(KEY_M) là hàm của raylib.
            // Hàm này trả về true đúng 1 lần tại thời điểm người chơi vừa bấm phím M.
            // Nó khác với IsKeyDown, vì IsKeyDown sẽ true liên tục khi giữ phím.
            //
            // Nếu bấm M thì gán screen = MAIN_MENU.
            // Nghĩa là chuyển từ màn hình chơi game về menu chính.
            //
            // Dòng này viết gọn trên một dòng:
            // nếu bấm M thì quay về menu.
            if (IsKeyPressed(KEY_M)) screen = MAIN_MENU;

            // Kiểm tra người chơi có bấm phím R không.
            //
            // Trong game này, phím R dùng để restart ván chơi.
            // Nhưng không chỉ restart lại map hiện tại, dòng ResetGame bên dưới truyền selectedMap = -1.
            // selectedMap = -1 được quy ước là chọn map ngẫu nhiên.
            //
            // Nói dễ hiểu:
            // bấm R -> tạo lại ván mới -> chọn ngẫu nhiên một map trong danh sách 12 map.
            if (IsKeyPressed(KEY_R)) {
                // Gọi hàm ResetGame để đặt lại toàn bộ trạng thái của ván chơi.
                //
                // ResetGame là hàm đã viết ở phía trên.
                // Hàm này nhận rất nhiều biến bằng tham chiếu & để sửa trực tiếp trạng thái game.
                //
                // mazeList:
                //     là danh sách tất cả bản đồ đã tạo bởi BuildMazeList().
                //     Trong game hiện tại có MAP_COUNT = 12 map.
                //
                // maze:
                //     là bản đồ hiện tại đang được chơi.
                //     Sau khi ResetGame chạy, maze sẽ được gán thành một map mới lấy từ mazeList.
                //
                // playerR, playerC:
                //     là hàng và cột của người chơi.
                //     ResetGame sẽ đưa người chơi về vị trí bắt đầu 'S' của map mới.
                //
                // exitR, exitC:
                //     là hàng và cột của cửa thoát.
                //     ResetGame sẽ tìm lại vị trí 'E' trong map mới.
                ResetGame(mazeList, maze, playerR, playerC, exitR, exitC,
                          // enemy:
                          //     là biến lưu thông tin quái vật.
                          //     ResetGame sẽ đặt lại vị trí quái và timer của quái.
                          //
                          // items:
                          //     là danh sách vật phẩm trên map.
                          //     ResetGame sẽ xóa vật phẩm cũ và tạo lại vật phẩm mới.
                          //
                          // score:
                          //     là điểm số của người chơi.
                          //     Khi restart thì điểm được đưa về 0.
                          //
                          // itemsCollected:
                          //     là số vật phẩm đã nhặt.
                          //     Khi restart thì số vật phẩm đã nhặt trở về 0.
                          //
                          // currentMap:
                          //     là chỉ số map hiện tại.
                          //     Nếu chọn map ngẫu nhiên thì biến này sẽ được cập nhật thành map mới.
                          enemy, items, score, itemsCollected, currentMap,
                          // win:
                          //     biến kiểm tra người chơi đã thắng chưa.
                          //     Restart thì win phải trở về false.
                          //
                          // gameOver:
                          //     biến kiểm tra người chơi đã thua chưa.
                          //     Restart thì gameOver phải trở về false.
                          //
                          // playerSpeed:
                          //     tốc độ di chuyển của người chơi.
                          //     Restart thì tốc độ về x1.0.
                          //
                          // moveCharge:
                          //     biến tích lũy lượt di chuyển khi người chơi được tăng tốc.
                          //     Restart thì đưa về 0.
                          //
                          // boostMessageTimer:
                          //     thời gian hiện thông báo tăng tốc.
                          //     Restart thì đưa về 0.
                          win, gameOver, playerSpeed, moveCharge,
                          // -1 là tham số selectedMap.
                          //
                          // Trong hàm ResetGame có đoạn:
                          // if (selectedMap < 0) {
                          //     currentMap = GetRandomValue(0, mazeList.size() - 1);
                          // }
                          //
                          // Vì -1 nhỏ hơn 0 nên chương trình hiểu rằng:
                          // không chọn map cố định, mà random một map bất kỳ.
                          //
                          // Vì vậy phím R ở đây có tác dụng:
                          // restart game và đổi sang một bản đồ ngẫu nhiên.
                          boostMessageTimer, -1);
            }

            // Kiểm tra người chơi có bấm phím N không.
            //
            // Phím N dùng để chuyển sang bản đồ tiếp theo.
            // Khác với phím R:
            // - R chọn map ngẫu nhiên.
            // - N chọn map kế tiếp theo thứ tự.
            //
            // Ví dụ:
            // đang ở map 1 -> bấm N sang map 2.
            // đang ở map 2 -> bấm N sang map 3.
            // ...
            // đang ở map 12 -> bấm N quay lại map 1.
            if (IsKeyPressed(KEY_N)) {
                // Gọi ResetGame để tạo lại ván chơi ở map tiếp theo.
                //
                // Các tham số từ mazeList đến boostMessageTimer có ý nghĩa giống phần phím R ở trên:
                // đặt lại bản đồ, người chơi, cửa thoát, quái vật, vật phẩm, điểm,
                // số vật phẩm đã nhặt, trạng thái thắng/thua, tốc độ và thông báo tăng tốc.
                ResetGame(mazeList, maze, playerR, playerC, exitR, exitC,
                          enemy, items, score, itemsCollected, currentMap,
                          win, gameOver, playerSpeed, moveCharge,
                          // (currentMap + 1) % MAP_COUNT là cách tính map kế tiếp.
                          //
                          // currentMap:
                          //     là chỉ số map hiện tại.
                          //     Lưu ý trong lập trình, chỉ số thường bắt đầu từ 0.
                          //
                          // Nếu có 12 map thì chỉ số map là:
                          // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
                          //
                          // currentMap + 1:
                          //     lấy map hiện tại cộng thêm 1 để sang map tiếp theo.
                          //
                          // % MAP_COUNT:
                          //     là phép chia lấy dư.
                          //     Nó giúp khi đi quá map cuối thì quay lại map đầu.
                          //
                          // Ví dụ MAP_COUNT = 12:
                          // nếu currentMap = 0 thì (0 + 1) % 12 = 1 -> sang map 2.
                          // nếu currentMap = 1 thì (1 + 1) % 12 = 2 -> sang map 3.
                          // nếu currentMap = 11 thì (11 + 1) % 12 = 0 -> quay lại map 1.
                          //
                          // Nhờ vậy chương trình không bị vượt quá số lượng map.
                          boostMessageTimer, (currentMap + 1) % MAP_COUNT);
            }

            if (!win && !gameOver) {
                int dr = 0;
                int dc = 0;
                bool movePressed = false;
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

                if (movePressed) {
                    moveCharge += playerSpeed;
                    int moveSteps = (int)moveCharge;
                    moveCharge -= moveSteps;
                    for (int step = 0; step < moveSteps; step++) {
                        int nr = playerR + dr;
                        int nc = playerC + dc;
                        if (!CanMove(maze, nr, nc)) break;
                        playerR = nr;
                        playerC = nc;

                        if (CheckCollectItem(items, playerR, playerC, score,
                                             itemsCollected, playerSpeed)) {
                            boostMessageTimer = 1.2f;
                        }
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

                if (!win && !gameOver) {
                    enemy.timer += GetFrameTime();
                    if (enemy.timer >= ENEMY_DELAY) {
                        enemy.timer = 0.0f;
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

        if (boostMessageTimer > 0.0f) boostMessageTimer -= GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);

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

        EndDrawing();
    }

    if (characterSheet.loaded) UnloadTexture(characterSheet.texture);
    if (loadedCustomFont) UnloadFont(vietnameseFont);
    CloseWindow();
    return 0;
}

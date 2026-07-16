//Chức năng vật phẩm 

// Lưu thông tin của một vật phẩm trên bản đồ.
struct Item {
    int r, c;          // Tọa độ hàng (r) và cột (c) của vật phẩm.
    bool collected;    // true = đã nhặt, false = chưa nhặt.
};

// Số lượng vật phẩm sẽ được sinh trong mỗi màn chơi.
const int ITEM_COUNT = 5;
// Sinh ngẫu nhiên các vật phẩm trên mê cung.
// Không sinh lên:
// - Tường
// - Người chơi
// - Quỷ
// - Cửa thoát
// - Vị trí đã có vật phẩm khác
//---------------------------------------------------------
void AddRandomItems(const vector<string>& maze, vector<Item>& items,
                    int playerR, int playerC, int exitR, int exitC,
                    int enemyR, int enemyC)
{
    items.clear();                     // Xóa vật phẩm cũ trước khi sinh mới.

    while ((int)items.size() < ITEM_COUNT)
    {
        // Sinh ngẫu nhiên một vị trí.
        int r = GetRandomValue(1, ROWS - 2);
        int c = GetRandomValue(1, COLS - 2);

        // Nếu là tường thì bỏ qua.
        if (!CanMove(maze, r, c))
            continue;

        // Không được trùng vị trí người chơi, quỷ hoặc cửa thoát.
        if ((r == playerR && c == playerC) ||
            (r == exitR && c == exitC) ||
            (r == enemyR && c == enemyC))
            continue;

        // Kiểm tra có bị trùng vật phẩm khác không.
        bool duplicated = false;
        for (const Item& item : items)
        {
            if (item.r == r && item.c == c)
                duplicated = true;
        }

        // Nếu không trùng thì thêm vào danh sách.
        if (!duplicated)
            items.push_back({r, c, false});
    }
}
// Kiểm tra người chơi có nhặt được vật phẩm hay không.
//
// Nếu nhặt:
// +10 điểm
// Tăng số vật phẩm đã nhặt
// Tăng tốc độ di chuyển
bool CheckCollectItem(vector<Item>& items, int playerR, int playerC,
                      int& score, int& itemsCollected, float& playerSpeed)
{
    for (Item& item : items)
    {
        // Người chơi đứng đúng vị trí vật phẩm.
        if (!item.collected &&
            item.r == playerR &&
            item.c == playerC)
        {
            item.collected = true;         // Đánh dấu đã nhặt.
            score += 10;                   // Cộng điểm.
            itemsCollected++;              // Tăng số vật phẩm.
            playerSpeed = 1.0f + itemsCollected * 0.2f; // Tăng tốc.

            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
// Vẽ tất cả vật phẩm chưa được nhặt.
//---------------------------------------------------------
for (const Item& item : items)
{
    if (!item.collected)
    {
        DrawCircle(
            item.c * CELL_SIZE + CELL_SIZE / 2,
            item.r * CELL_SIZE + CELL_SIZE / 2,
            CELL_SIZE / 4,
            GOLD
        );
    }
}
//Chức năng di chuyển
// Kiểm tra ô hiện tại có phải là tường hay không.
bool IsWall(const vector<string>& maze, int r, int c)
{
    // Nếu vượt khỏi biên mê cung thì xem như tường.
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS)
        return true;

    // Nếu ký tự là '#' thì là tường.
    return maze[r][c] == '#';
}

// Kiểm tra nhân vật có được phép đi vào ô này hay không.
bool CanMove(const vector<string>& maze, int r, int c)
{
    return !IsWall(maze, r, c);
}
// Thuật toán BFS tìm đường đi ngắn nhất.
// Được Enemy sử dụng để đuổi theo người chơi.
vector<pair<int, int>> FindPath(const vector<string>& maze,
                                int sr, int sc, int tr, int tc)
{
    // Đánh dấu các ô đã duyệt.
    vector<vector<int>> visited(ROWS, vector<int>(COLS, 0));

    // Lưu ô cha để khôi phục đường đi.
    vector<vector<pair<int, int>>> parent(
        ROWS, vector<pair<int, int>>(COLS, {-1, -1}));

    // Hàng đợi BFS.
    queue<pair<int, int>> cells;

    cells.push({sr, sc});
    visited[sr][sc] = 1;

    // 4 hướng di chuyển.
    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};

    while (!cells.empty())
    {
        pair<int, int> current = cells.front();
        cells.pop();

        int r = current.first;
        int c = current.second;

        // Đã đến đích.
        if (r == tr && c == tc)
            break;

        // Duyệt 4 ô xung quanh.
        for (int i = 0; i < 4; i++)
        {
            int nr = r + dr[i];
            int nc = c + dc[i];

            if (nr >= 0 && nr < ROWS &&
                nc >= 0 && nc < COLS &&
                !visited[nr][nc] &&
                CanMove(maze, nr, nc))
            {
                visited[nr][nc] = 1;
                parent[nr][nc] = {r, c};
                cells.push({nr, nc});
            }
        }
    }

    // Khôi phục đường đi.
    vector<pair<int, int>> path;

    if (!visited[tr][tc])
        return path;

    int r = tr;
    int c = tc;

    while (!(r == sr && c == sc))
    {
        path.push_back({r, c});

        pair<int, int> previous = parent[r][c];

        r = previous.first;
        c = previous.second;
    }

    reverse(path.begin(), path.end());

    return path;
}
// Đặt vị trí ban đầu của Enemy.
// Enemy sẽ xuất hiện cách người chơi vài ô.
void PlaceEnemy(const vector<string>& maze, Enemy& enemy,
                int playerR, int playerC,
                int exitR, int exitC)
{
    vector<pair<int, int>> path =
        FindPath(maze, exitR, exitC, playerR, playerC);

    int position = min(4, (int)path.size() - 1);

    if (path.empty())
    {
        enemy.r = exitR;
        enemy.c = exitC;
    }
    else
    {
        enemy.r = path[position].first;
        enemy.c = path[position].second;
    }

    // Reset thời gian di chuyển của Enemy.
    enemy.timer = 0.0f;
}
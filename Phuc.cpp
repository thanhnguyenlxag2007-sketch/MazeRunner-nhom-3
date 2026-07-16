// ========================= BẢN ĐỒ VÀ TÌM ĐƯỜNG =========================

// Kiểm tra một ô có phải là tường hay không.
// Nếu tọa độ nằm ngoài bản đồ thì cũng xem như là tường
// để tránh truy cập ra ngoài mảng.
bool IsWall(const vector<string>& maze, int r, int c) {
    // Kiểm tra vượt biên của mê cung.
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return true;

    // Ký tự '#' đại diện cho tường.
    return maze[r][c] == '#';
}

// Kiểm tra xem người chơi hoặc quỷ có thể đi vào ô này hay không.
// Chỉ những ô không phải tường mới được phép đi qua.
bool CanMove(const vector<string>& maze, int r, int c) {
    return !IsWall(maze, r, c);
}

// Tìm đường đi ngắn nhất bằng thuật toán BFS
// từ ô bắt đầu (sr, sc) đến ô đích (tr, tc).
vector<pair<int, int>> FindPath(const vector<string>& maze,
                                int sr, int sc, int tr, int tc) {

    // visited[r][c] = 1 nghĩa là ô này đã được duyệt.
    vector<vector<int>> visited(ROWS, vector<int>(COLS, 0));

    // parent[r][c] lưu ô mà ta đi từ đó đến (r, c).
    // Dùng để truy ngược lại đường đi sau khi BFS kết thúc.
    vector<vector<pair<int, int>>> parent(
        ROWS, vector<pair<int, int>>(COLS, {-1, -1})
    );

    // BFS sử dụng queue (hàng đợi).
    queue<pair<int, int>> cells;

    // Đưa ô bắt đầu vào hàng đợi.
    cells.push({sr, sc});
    visited[sr][sc] = 1;

    // Bốn hướng di chuyển:
    // lên, xuống, trái, phải.
    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};

    // Lặp cho đến khi không còn ô nào cần duyệt.
    while (!cells.empty()) {

        // Lấy ô ở đầu hàng đợi.
        pair<int, int> current = cells.front();
        cells.pop();

        int r = current.first;
        int c = current.second;

        // Nếu đã đến đích thì dừng BFS.
        if (r == tr && c == tc) break;

        // Thử đi sang 4 ô xung quanh.
        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];

            // Ô mới phải:
            // - nằm trong bản đồ
            // - chưa được duyệt
            // - không phải là tường
            if (nr >= 0 && nr < ROWS &&
                nc >= 0 && nc < COLS &&
                !visited[nr][nc] &&
                CanMove(maze, nr, nc)) {

                // Đánh dấu đã duyệt.
                visited[nr][nc] = 1;

                // Ghi nhớ ô cha để khôi phục đường đi sau này.
                parent[nr][nc] = {r, c};

                // Đưa ô mới vào hàng đợi.
                cells.push({nr, nc});
            }
        }
    }

    // Khôi phục lại đường đi bằng cách
    // đi ngược từ đích về điểm xuất phát.
    vector<pair<int, int>> path;

    // Nếu ô đích chưa được duyệt
    // nghĩa là không tồn tại đường đi.
    if (!visited[tr][tc]) return path;

    int r = tr;
    int c = tc;

    // Truy ngược mảng parent.
    while (!(r == sr && c == sc)) {
        path.push_back({r, c});

        pair<int, int> previous = parent[r][c];
        r = previous.first;
        c = previous.second;
    }

    // Vì đang đi từ đích về đầu nên cần đảo ngược.
    reverse(path.begin(), path.end());

    return path;
}

// Duyệt toàn bộ bản đồ để tìm:
// 'S' = vị trí bắt đầu của người chơi.
// 'E' = vị trí cổng thoát.
void FindStartAndExit(const vector<string>& maze,
                      int& playerR, int& playerC,
                      int& exitR, int& exitC) {

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {

            // Tìm vị trí bắt đầu.
            if (maze[r][c] == 'S') {
                playerR = r;
                playerC = c;
            }

            // Tìm vị trí cổng thoát.
            else if (maze[r][c] == 'E') {
                exitR = r;
                exitC = c;
            }
        }
    }
}

// Đặt quỷ gần cổng thoát.
// Quỷ sẽ nằm trên đường đi từ cổng thoát về phía người chơi.
void PlaceEnemy(const vector<string>& maze, Enemy& enemy,
                int playerR, int playerC,
                int exitR, int exitC) {

    // Tìm đường từ cổng thoát đến người chơi.
    vector<pair<int, int>> path =
        FindPath(maze, exitR, exitC, playerR, playerC);

    // Đặt quỷ cách cổng thoát tối đa 4 bước.
    int position = min(4, (int)path.size() - 1);

    // Nếu không tìm được đường đi,
    // đặt quỷ ngay tại cổng thoát.
    if (path.empty()) {
        enemy.r = exitR;
        enemy.c = exitC;
    }
    else {
        enemy.r = path[position].first;
        enemy.c = path[position].second;
    }

    // Đặt lại bộ đếm thời gian di chuyển của quỷ.
    enemy.timer = 0.0f;
}
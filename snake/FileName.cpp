#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <string>

using namespace sf;
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;
const int GRID_SIZE = 20;
const int GRID_WIDTH = WIDTH / GRID_SIZE;
const int GRID_HEIGHT = HEIGHT / GRID_SIZE;

enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    HELP,
    LEVEL_SELECT
};

enum class Difficulty {
    EASY,
    HARD
};

enum class FoodType {
    NORMAL,     // 普通食物，+1分
    GOLDEN,     // 金色食物，+5分
    SPEED_UP,   // 加速食物
    SPEED_DOWN, // 减速食物
    BONUS       // 特殊奖励
};

struct Food {
    int x, y;
    FoodType type;
    sf::Color color;
    int scoreValue;
    float effectDuration; // 效果持续时间(秒)
};

// 方向枚举
enum class Direction { UP, DOWN, LEFT, RIGHT, None };

// 蛇的段结构
struct SnakeSegment {
    int x, y;
    SnakeSegment(int x, int y) : x(x), y(y) {}
};

// 菜单类
class Menu {
private:
    RenderWindow& window;
    Font font;
    vector<Text> menuItems;
    int selectedItemIndex;
    Color normalColor;
    Color selectedColor;
    Texture backgroundTexture;
    Sprite backgroundSprite;

public:
    Menu(RenderWindow& window) : window(window), selectedItemIndex(0) {
        if (!font.loadFromFile("arial.ttf")) {
            font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
        }

        normalColor = Color::White;
        selectedColor = Color::Green;

        // 菜单项
        vector<string> items = { "START", "SELECT", "INSTRUCTION", "EXIT" };
        
        for (size_t i = 0; i < items.size(); ++i) {
            Text item;
            item.setFont(font);
            item.setString(items[i]);
            item.setCharacterSize(36);
            item.setFillColor(i == selectedItemIndex ? selectedColor : normalColor);
            item.setPosition(WIDTH / 2 - item.getLocalBounds().width / 2, 
                             HEIGHT / 2 - (items.size() * 40) / 2 + i * 60);
            menuItems.push_back(item);
        }

        // 加载背景
        if (backgroundTexture.loadFromFile("Images/Background.jpg")) {
            backgroundSprite.setTexture(backgroundTexture);
            // 调整背景大小以适应窗口
            backgroundSprite.setScale(
                float(WIDTH) / backgroundTexture.getSize().x,
                float(HEIGHT) / backgroundTexture.getSize().y
            );
        }
    }

    void draw() {
        window.clear();
        
        // 绘制背景
        if (backgroundTexture.getSize() != Vector2u(0, 0)) {
            window.draw(backgroundSprite);
        } else {
            // 如果背景加载失败，使用纯色背景
            window.clear(Color::Black);
        }

        // 绘制菜单项
        for (const auto& item : menuItems) {
            window.draw(item);
        }

        window.display();
    }

    void moveUp() {
        if (selectedItemIndex > 0) {
            menuItems[selectedItemIndex].setFillColor(normalColor);
            selectedItemIndex--;
            menuItems[selectedItemIndex].setFillColor(selectedColor);
        }
    }

    void moveDown() {
        if (selectedItemIndex < menuItems.size() - 1) {
            menuItems[selectedItemIndex].setFillColor(normalColor);
            selectedItemIndex++;
            menuItems[selectedItemIndex].setFillColor(selectedColor);
        }
    }

    int getSelectedItem() const {
        return selectedItemIndex;
    }
};

// 关卡选择菜单
class LevelSelect {
private:
    RenderWindow& window;
    Font font;
    vector<Text> levelItems;
    int selectedLevelIndex;
    Color normalColor;
    Color selectedColor;

public:
    LevelSelect(RenderWindow& window) : window(window), selectedLevelIndex(0) {
        if (!font.loadFromFile("arial.ttf")) {
            font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
        }

        normalColor = Color::White;
        selectedColor = Color::Green;

        // 关卡项
        vector<string> items = { "EASY", "HARD" };
        
        for (size_t i = 0; i < items.size(); ++i) {
            Text item;
            item.setFont(font);
            item.setString(items[i]);
            item.setCharacterSize(36);
            item.setFillColor(i == selectedLevelIndex ? selectedColor : normalColor);
            item.setPosition(WIDTH / 2 - item.getLocalBounds().width / 2, 
                             HEIGHT / 2 - (items.size() * 40) / 2 + i * 60);
            levelItems.push_back(item);
        }
    }

    void draw() {
        window.clear(Color::Black);

        // 绘制标题
        Text title;
        title.setFont(font);
        title.setString("SELECT");//选择关卡
        title.setCharacterSize(48);
        title.setFillColor(Color::White);
        title.setPosition(WIDTH / 2 - title.getLocalBounds().width / 2, 100);
        window.draw(title);

        // 绘制关卡项
        for (const auto& item : levelItems) {
            window.draw(item);
        }

        // 绘制返回提示
        Text backHint;
        backHint.setFont(font);
        backHint.setString("按 ESC 返回主菜单");
        backHint.setCharacterSize(20);
        backHint.setFillColor(Color::White);
        backHint.setPosition(20, HEIGHT - 50);
        window.draw(backHint);

        window.display();
    }

    void moveUp() {
        if (selectedLevelIndex > 0) {
            levelItems[selectedLevelIndex].setFillColor(normalColor);
            selectedLevelIndex--;
            levelItems[selectedLevelIndex].setFillColor(selectedColor);
        }
    }

    void moveDown() {
        if (selectedLevelIndex < levelItems.size() - 1) {
            levelItems[selectedLevelIndex].setFillColor(normalColor);
            selectedLevelIndex++;
            levelItems[selectedLevelIndex].setFillColor(selectedColor);
        }
    }

    Difficulty getSelectedDifficulty() const {
        return static_cast<Difficulty>(selectedLevelIndex);
    }
};

// 帮助/说明界面
class HelpScreen {
private:
    RenderWindow& window;
    Font font;
    vector<Text> helpTexts;

public:
    HelpScreen(RenderWindow& window) : window(window) {
        if (!font.loadFromFile("arial.ttf")) {
            font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
        }

        // 游戏说明文本
        vector<string> helpLines = {
            "贪吃蛇游戏说明",
            "",
            "控制:",
            "↑ ↓ ← → 方向键 或 WASD - 控制蛇的移动",
            "P - 暂停/继续游戏",
            "R - 游戏结束后重新开始",
            "",
            "游戏目标:",
            "吃掉食物使蛇变长，获得更高分数",
            "",
            "游戏模式:",
            "简单模式 - 撞墙后蛇会沿着墙滑行",
            "困难模式 - 撞墙后游戏结束",
            "",
            "按 ESC 返回主菜单"
        };

        for (size_t i = 0; i < helpLines.size(); ++i) {
            Text line;
            line.setFont(font);
            line.setString(helpLines[i]);
            line.setCharacterSize(i == 0 ? 36 : 24);
            line.setFillColor(i == 0 ? Color::Green : Color::White);
            line.setPosition(WIDTH / 2 - line.getLocalBounds().width / 2, 
                             100 + i * 40);
            helpTexts.push_back(line);
        }
    }

    void draw() {
        window.clear(Color::Black);

        for (const auto& text : helpTexts) {
            window.draw(text);
        }

        window.display();
    }
};

// 游戏类
class Game {
private:
    RenderWindow window;
    vector<SnakeSegment> snake;
    Direction dir, nextDir;
    int foodX, foodY;
    int score;
    bool gameOver;
    Font font;
    Text scoreText;
    Text gameOverText;
    Clock clock;
    float speed;
    bool paused;
    Texture snakeTexture;
    Texture foodTexture;
    
    float baseSpeed; // 基础移动速度
    float currentSpeed; // 当前速度(可能被食物效果修改)
    sf::Clock speedEffectClock;
    bool speedEffectActive = false;
    
    GameState currentState;
    Menu menu;
    LevelSelect levelSelect;
    HelpScreen helpScreen;
    Difficulty difficulty;

    // 生成随机位置
    int random(int min, int max) {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<> distrib(min, max);
        return distrib(gen);
    }

    // 放置食物
    void placeFood() {
        foodX = random(0, GRID_WIDTH - 1);
        foodY = random(0, GRID_HEIGHT - 1);

        // 确保食物不会出现在蛇身上
        for (const auto& segment : snake) {
            if (segment.x == foodX && segment.y == foodY) {
                placeFood();
                return;
            }
        }
    }

    // 检查碰撞 - 困难模式
    void checkCollision_hard() {
        // 检查是否撞墙
        if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH ||
            snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
            gameOver = true;
            return;
        }

        // 检查是否撞到自己
        for (size_t i = 1; i < snake.size(); ++i) {
            if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                gameOver = true;
                return;
            }
        }

        // 检查是否吃到食物
        if (snake[0].x == foodX && snake[0].y == foodY) {
            // 增加蛇的长度
            snake.push_back(SnakeSegment(foodX, foodY));

            // 增加分数
            score += 10;
            scoreText.setString("Score: " + to_string(score));

            // 提高速度
            speed *= 0.95f;

            // 放置新食物
            placeFood();
        }
    }
    
    // 检查碰撞 - 简单模式
    void checkCollision_easy() {
        // 检查是否撞墙并开始滑行
        if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH ||
            snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {

            // 确保蛇头停留在边界上
            snake[0].x = std::clamp(snake[0].x, 0, GRID_WIDTH - 1);
            snake[0].y = std::clamp(snake[0].y, 0, GRID_HEIGHT - 1);

            // 根据当前方向确定滑行方向
            if (dir == Direction::UP || dir == Direction::DOWN) {
                // 垂直运动撞墙，改为水平滑行
                if (snake[0].x < GRID_WIDTH / 2) {
                    nextDir = Direction::RIGHT; // 靠近左边界则向右滑
                }
                else {
                    nextDir = Direction::LEFT;  // 靠近右边界则向左滑
                }
            }
            else {
                // 水平运动撞墙，改为垂直滑行
                if (snake[0].y < GRID_HEIGHT / 2) {
                    nextDir = Direction::DOWN;  // 靠近上边界则向下滑
                }
                else {
                    nextDir = Direction::UP;    // 靠近下边界则向上滑
                }
            }
            dir = nextDir;
        }

        // 检查是否吃到食物
        if (snake[0].x == foodX && snake[0].y == foodY) {
            // 增加蛇的长度
            snake.push_back(SnakeSegment(foodX, foodY));

            // 增加分数
            score += 10;
            scoreText.setString("Score: " + to_string(score));

            // 提高速度
            speed *= 0.95f;

            // 放置新食物
            placeFood();
        }
    }

    // 移动蛇
    void moveSnake() {
        // 移动身体
        for (size_t i = snake.size() - 1; i > 0; --i) {
            snake[i].x = snake[i - 1].x;
            snake[i].y = snake[i - 1].y;
        }

        // 移动头部
        switch (dir) {
        case Direction::UP:    snake[0].y--; break;
        case Direction::DOWN:  snake[0].y++; break;
        case Direction::LEFT:  snake[0].x--; break;
        case Direction::RIGHT: snake[0].x++; break;
        }
    }

    // 更新游戏状态
    void update() {
        if (gameOver || paused) return;
        
        if (clock.getElapsedTime().asSeconds() >= speed) {
            clock.restart();
            
            // 应用方向变化
            dir = nextDir;
            
            // 移动蛇
            moveSnake();
            
            // 根据难度选择碰撞检测函数
            if (difficulty == Difficulty::HARD) {
                checkCollision_hard();
            } else {
                checkCollision_easy();
            }
        }
    }

    // 绘制游戏
    void draw() {
        window.clear(Color::Black);

        // 绘制食物
        Sprite foodSprite(foodTexture);
        if (!foodTexture.loadFromFile("Images/food.jpg")) {
            throw "Failed to load food.";
        }
        else {
            foodSprite.setTexture(foodTexture);
        }
        foodSprite.setPosition(foodX * GRID_SIZE, foodY * GRID_SIZE);
        // 调整大小以适应网格
        foodSprite.setScale(
            float(GRID_SIZE) / foodTexture.getSize().x,
            float(GRID_SIZE) / foodTexture.getSize().y
        );
        window.draw(foodSprite);

        // 绘制蛇
        for (size_t i = 0; i < snake.size(); ++i) {
            Sprite segment(snakeTexture);
            if (!snakeTexture.loadFromFile("Images/snake.jpg")) {
                throw "Failed to load snake.";
            }
            else {
                segment.setTexture(snakeTexture);
            }
            segment.setPosition(snake[i].x * GRID_SIZE, snake[i].y * GRID_SIZE);

            // 调整大小
            segment.setScale(
                float(GRID_SIZE) / snakeTexture.getSize().x,
                float(GRID_SIZE) / snakeTexture.getSize().y
            );

            // 头部可以有不同的纹理或颜色
            if (i == 0) {
                segment.setColor(Color::Green);
            }
            else {
                segment.setColor(Color(100, 255, 100));
            }
            window.draw(segment);
        }

        // 绘制分数
        window.draw(scoreText);

        // 如果游戏结束，显示游戏结束文本
        if (gameOver) {
            window.draw(gameOverText);
        }

        window.display();
    }

    // 重置游戏
    void reset() {
        snake.clear();
        snake.push_back(SnakeSegment(GRID_WIDTH / 2, GRID_HEIGHT / 2));
        dir = Direction::RIGHT;
        nextDir = Direction::RIGHT;
        score = 0;
        speed = 0.1f;
        gameOver = false;
        scoreText.setString("Score: " + to_string(score));
        placeFood();
    }

    // 处理菜单输入
    void handleMenuInput(Event& event) {
        if (event.type == Event::KeyPressed) {
            switch (event.key.code) {
            case Keyboard::Up:
                menu.moveUp();
                break;
            case Keyboard::Down:
                menu.moveDown();
                break;
            case Keyboard::Enter:
                switch (menu.getSelectedItem()) {
                case 0: // 开始游戏
                    currentState = GameState::PLAYING;
                    reset();
                    break;
                case 1: // 选择关卡
                    currentState = GameState::LEVEL_SELECT;
                    break;
                case 2: // 游戏说明
                    currentState = GameState::HELP;
                    break;
                case 3: // 退出
                    window.close();
                    break;
                }
                break;
            }
        }
    }

    // 处理关卡选择输入
    void handleLevelSelectInput(Event& event) {
        if (event.type == Event::KeyPressed) {
            switch (event.key.code) {
            case Keyboard::Up:
                levelSelect.moveUp();
                break;
            case Keyboard::Down:
                levelSelect.moveDown();
                break;
            case Keyboard::Enter:
                difficulty = levelSelect.getSelectedDifficulty();
                currentState = GameState::MENU;
                break;
            case Keyboard::Escape:
                currentState = GameState::MENU;
                break;
            }
        }
    }

    // 处理帮助界面输入
    void handleHelpInput(Event& event) {
        if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
            currentState = GameState::MENU;
        }
    }

    // 处理游戏输入
    void handleGameInput(Event& event) {
        if (event.type == Event::KeyPressed) {
            if (gameOver && event.key.code == Keyboard::R) {
                reset();
            }

            if (event.key.code == Keyboard::P) {
                paused = !paused;
            }

            if (!gameOver && !paused) {
                switch (event.key.code) {
                case Keyboard::Up:
                case Keyboard::W:
                    if (dir != Direction::DOWN) nextDir = Direction::UP;
                    break;
                case Keyboard::Down:
                case Keyboard::S:
                    if (dir != Direction::UP) nextDir = Direction::DOWN;
                    break;
                case Keyboard::Left:
                case Keyboard::A:
                    if (dir != Direction::RIGHT) nextDir = Direction::LEFT;
                    break;
                case Keyboard::Right:
                case Keyboard::D:
                    if (dir != Direction::LEFT) nextDir = Direction::RIGHT;
                    break;
                case Keyboard::Escape:
                    currentState = GameState::MENU;
                    break;
                }
            }
        }
    }

public:
    Game() : window(VideoMode(WIDTH, HEIGHT), "SFML Snake Game"), 
             dir(Direction::RIGHT), nextDir(Direction::RIGHT), 
             score(0), gameOver(false), speed(0.1f), paused(false),
             menu(window), levelSelect(window), helpScreen(window),
             difficulty(Difficulty::EASY), currentState(GameState::MENU) {
        
        // 初始化字体
        if (!font.loadFromFile("arial.ttf")) {
            // 如果加载失败，使用默认字体
            font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
        }

        // 设置分数文本
        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(10, 10);

        // 设置游戏结束文本
        gameOverText.setFont(font);
        gameOverText.setCharacterSize(48);
        gameOverText.setFillColor(Color::Red);
        gameOverText.setString("Game Over!\nPress R to restart\nESC to return to menu");
        gameOverText.setPosition(WIDTH / 2 - 150, HEIGHT / 2 - 70);

        reset();
    }

    void run() {
        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                }

                switch (currentState) {
                case GameState::MENU:
                    handleMenuInput(event);
                    break;
                case GameState::LEVEL_SELECT:
                    handleLevelSelectInput(event);
                    break;
                case GameState::HELP:
                    handleHelpInput(event);
                    break;
                case GameState::PLAYING:
                case GameState::GAME_OVER:
                    handleGameInput(event);
                    break;
                }
            }

            // 更新游戏状态
            if (currentState == GameState::PLAYING) {
                update();
            }

            // 绘制当前状态
            switch (currentState) {
            case GameState::MENU:
                menu.draw();
                break;
            case GameState::LEVEL_SELECT:
                levelSelect.draw();
                break;
            case GameState::HELP:
                helpScreen.draw();
                break;
            case GameState::PLAYING:
            case GameState::GAME_OVER:
                draw();
                break;
            }
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}

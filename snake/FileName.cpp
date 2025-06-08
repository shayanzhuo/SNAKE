#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <algorithm>

using namespace sf;
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;
const int GRID_SIZE = 20;
const int GRID_WIDTH = WIDTH / GRID_SIZE;
const int GRID_HEIGHT = HEIGHT / GRID_SIZE;
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
enum class Direction { UP, DOWN, LEFT, RIGHT,None };

// 蛇的段结构
struct SnakeSegment {
    int x, y;
    SnakeSegment(int x, int y) : x(x), y(y) {}
};

//class FoodSystem {
//private:
//    std::vector<Food> foods;
//    int gridWidth, gridHeight;
//
//public:
//    FoodSystem(int width, int height) : gridWidth(width), gridHeight(height) {}
//
//    void spawnFood(const std::vector<SnakeSegment>& snake) {
//        // 随机决定生成哪种食物
//        FoodType type = getRandomFoodType();
//
//        Food newFood;
//        newFood.type = type;
//        setFoodProperties(newFood); // 根据类型设置属性
//
//        // 确保食物不会生成在蛇身上
//        bool onSnake;
//        do {
//            onSnake = false;
//            newFood.x = rand() % gridWidth;
//            newFood.y = rand() % gridHeight;
//
//            for (const auto& segment : snake) {
//                if (segment.x == newFood.x && segment.y == newFood.y) {
//                    onSnake = true;
//                    break;
//                }
//            }
//        } while (onSnake);
//
//        foods.push_back(newFood);
//    }
//
//    void removeFood(int index) {
//        foods.erase(foods.begin() + index);
//    }
//
//    const std::vector<Food>& getFoods() const { return foods; }
//
//private:
//    FoodType getRandomFoodType() {
//        int randValue = rand() % 100;
//
//        if (randValue < 60) return FoodType::NORMAL;      // 60% 普通食物
//        if (randValue < 85) return FoodType::GOLDEN;      // 25% 金色食物
//        if (randValue < 95) return FoodType::SPEED_UP;    // 10% 加速食物
//        if (randValue < 98) return FoodType::SPEED_DOWN;  // 3% 减速食物
//        return FoodType::BONUS;                           // 2% 特殊奖励
//    }
//
//    void setFoodProperties(Food& food) {
//        switch (food.type) {
//        case FoodType::NORMAL:
//            food.color = sf::Color::Red;
//            food.scoreValue = 1;
//            food.effectDuration = 0;
//            break;
//        case FoodType::GOLDEN:
//            food.color = sf::Color::Yellow;
//            food.scoreValue = 5;
//            food.effectDuration = 0;
//            break;
//        case FoodType::SPEED_UP:
//            food.color = sf::Color::Green;
//            food.scoreValue = 2;
//            food.effectDuration = 10.0f;
//            break;
//        case FoodType::SPEED_DOWN:
//            food.color = sf::Color::Blue;
//            food.scoreValue = 3;
//            food.effectDuration = 8.0f;
//            break;
//        case FoodType::BONUS:
//            food.color = sf::Color::Magenta;
//            food.scoreValue = 10;
//            food.effectDuration = 15.0f;
//            break;
//        }
//    }
//};
// 游戏类
class Game {
private:
    RenderWindow window;
    vector<SnakeSegment> snake;
    Direction dir,nextDir;
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
    //FoodSystem foodSystem;
    float baseSpeed; // 基础移动速度
    float currentSpeed; // 当前速度(可能被食物效果修改)
    sf::Clock speedEffectClock;
    bool speedEffectActive = false;
    

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

    // 检查碰撞
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
            }dir = nextDir;
        }

        //// 检查是否撞到自己
        //for (size_t i = 1; i < snake.size(); ++i) {
        //    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        //        gameOver = true;
        //        return;
        //    }
        //}
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
    void update() {
        if (gameOver || paused) return;
        moveSnake();
        if (clock.getElapsedTime().asSeconds() >= speed) {
            clock.restart();

            // 应用方向变化
            //dir = nextDir;

            //if (dir == Direction::None) return;

            
            
            // 检查碰撞
            checkCollision_easy();

            // 检查是否吃到食物
          
        
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
        //case Direction::None:  break;
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
        /*RectangleShape foodRect(Vector2f(GRID_SIZE - 2, GRID_SIZE - 2));
        foodRect.setFillColor(Color::Red);
        foodRect.setPosition(foodX * GRID_SIZE + 1, foodY * GRID_SIZE + 1);
        window.draw(foodRect);*/

        // 绘制蛇
        for (size_t i = 0; i < snake.size(); ++i) {
            //RectangleShape segment(Vector2f(GRID_SIZE - 2, GRID_SIZE - 2));
            //segment.setPosition(snake[i].x * GRID_SIZE + 1, snake[i].y * GRID_SIZE + 1);

            //// 头部用不同颜色
            //if (i == 0) {
            //    segment.setFillColor(Color::Green);
            //}
            //else {
            //    segment.setFillColor(Color(100, 255, 100)); // 浅绿色
            //}
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
        score = 0;
        speed = 0.1f;
        gameOver = false;
        scoreText.setString("Score: " + to_string(score));
        placeFood();
    }

public:
    Game() : window(VideoMode(WIDTH, HEIGHT), "SFML Snake Game"), dir(Direction::RIGHT), score(0), gameOver(false), speed(0.1f), paused(false) {
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
        gameOverText.setString("Game Over!\nPress R to restart");
        gameOverText.setPosition(WIDTH / 2 - 150, HEIGHT / 2 - 50);

        reset();
    }

    void run() {
        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                }

                // 键盘输入处理
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
                            if (dir != Direction::DOWN) dir = Direction::UP;
                            break;
                        case Keyboard::Down:
                            if (dir != Direction::UP) dir = Direction::DOWN;
                            break;
                        case Keyboard::Left:
                            if (dir != Direction::RIGHT) dir = Direction::LEFT;
                            break;
                        case Keyboard::Right:
                            if (dir != Direction::LEFT) dir = Direction::RIGHT;
                            break;
                        }
                    }
                }
            }

            // 游戏逻辑更新
            if (!gameOver && !paused && clock.getElapsedTime().asSeconds() > speed) {
                clock.restart();
                moveSnake();
                checkCollision_easy();
            }

            // 绘制
            draw();
           
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
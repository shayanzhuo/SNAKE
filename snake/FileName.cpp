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
    NORMAL,     // ��ͨʳ�+1��
    GOLDEN,     // ��ɫʳ�+5��
    SPEED_UP,   // ����ʳ��
    SPEED_DOWN, // ����ʳ��
    BONUS       // ���⽱��
};

struct Food {
    int x, y;
    FoodType type;
    sf::Color color;
    int scoreValue;
    float effectDuration; // Ч������ʱ��(��)
};
// ����ö��
enum class Direction { UP, DOWN, LEFT, RIGHT,None };

// �ߵĶνṹ
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
//        // ���������������ʳ��
//        FoodType type = getRandomFoodType();
//
//        Food newFood;
//        newFood.type = type;
//        setFoodProperties(newFood); // ����������������
//
//        // ȷ��ʳ�ﲻ��������������
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
//        if (randValue < 60) return FoodType::NORMAL;      // 60% ��ͨʳ��
//        if (randValue < 85) return FoodType::GOLDEN;      // 25% ��ɫʳ��
//        if (randValue < 95) return FoodType::SPEED_UP;    // 10% ����ʳ��
//        if (randValue < 98) return FoodType::SPEED_DOWN;  // 3% ����ʳ��
//        return FoodType::BONUS;                           // 2% ���⽱��
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
// ��Ϸ��
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
    float baseSpeed; // �����ƶ��ٶ�
    float currentSpeed; // ��ǰ�ٶ�(���ܱ�ʳ��Ч���޸�)
    sf::Clock speedEffectClock;
    bool speedEffectActive = false;
    

    // �������λ��
    int random(int min, int max) {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<> distrib(min, max);
        return distrib(gen);
    }

    // ����ʳ��
    void placeFood() {
        foodX = random(0, GRID_WIDTH - 1);
        foodY = random(0, GRID_HEIGHT - 1);

        // ȷ��ʳ�ﲻ�������������
        for (const auto& segment : snake) {
            if (segment.x == foodX && segment.y == foodY) {
                placeFood();
                return;
            }
        }
    }

    // �����ײ
    void checkCollision_hard() {
        // ����Ƿ�ײǽ
        if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH ||
            snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
            gameOver = true;
            return;

        }


        // ����Ƿ�ײ���Լ�
        for (size_t i = 1; i < snake.size(); ++i) {
            if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                gameOver = true;
                return;
            }
        }

        // ����Ƿ�Ե�ʳ��
        if (snake[0].x == foodX && snake[0].y == foodY) {
            // �����ߵĳ���
            snake.push_back(SnakeSegment(foodX, foodY));

            // ���ӷ���
            score += 10;
            scoreText.setString("Score: " + to_string(score));

            // ����ٶ�
            speed *= 0.95f;

            // ������ʳ��
            placeFood();
        }
    }
    void checkCollision_easy() {
        // ����Ƿ�ײǽ����ʼ����
        if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH ||
            snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {

            // ȷ����ͷͣ���ڱ߽���
            snake[0].x = std::clamp(snake[0].x, 0, GRID_WIDTH - 1);
            snake[0].y = std::clamp(snake[0].y, 0, GRID_HEIGHT - 1);

            // ���ݵ�ǰ����ȷ�����з���
            if (dir == Direction::UP || dir == Direction::DOWN) {
                // ��ֱ�˶�ײǽ����Ϊˮƽ����
                if (snake[0].x < GRID_WIDTH / 2) {
                    nextDir = Direction::RIGHT; // ������߽������һ�
                }
                else {
                    nextDir = Direction::LEFT;  // �����ұ߽�������
                }
            }
            else {
                // ˮƽ�˶�ײǽ����Ϊ��ֱ����
                if (snake[0].y < GRID_HEIGHT / 2) {
                    nextDir = Direction::DOWN;  // �����ϱ߽������»�
                }
                else {
                    nextDir = Direction::UP;    // �����±߽������ϻ�
                }
            }dir = nextDir;
        }

        //// ����Ƿ�ײ���Լ�
        //for (size_t i = 1; i < snake.size(); ++i) {
        //    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        //        gameOver = true;
        //        return;
        //    }
        //}
            // ����Ƿ�Ե�ʳ��
            if (snake[0].x == foodX && snake[0].y == foodY) {
                // �����ߵĳ���
                snake.push_back(SnakeSegment(foodX, foodY));

                // ���ӷ���
                score += 10;
                scoreText.setString("Score: " + to_string(score));

                // ����ٶ�
                speed *= 0.95f;

                // ������ʳ��
                placeFood();
            }
        
    }
    void update() {
        if (gameOver || paused) return;
        moveSnake();
        if (clock.getElapsedTime().asSeconds() >= speed) {
            clock.restart();

            // Ӧ�÷���仯
            //dir = nextDir;

            //if (dir == Direction::None) return;

            
            
            // �����ײ
            checkCollision_easy();

            // ����Ƿ�Ե�ʳ��
          
        
        }
    }
    // �ƶ���
    void moveSnake() {
        // �ƶ�����
        for (size_t i = snake.size() - 1; i > 0; --i) {
            snake[i].x = snake[i - 1].x;
            snake[i].y = snake[i - 1].y;
        }

        // �ƶ�ͷ��
        switch (dir) {
        case Direction::UP:    snake[0].y--; break;
        case Direction::DOWN:  snake[0].y++; break;
        case Direction::LEFT:  snake[0].x--; break;
        case Direction::RIGHT: snake[0].x++; break;
        //case Direction::None:  break;
        }
    }
   
    // ������Ϸ
    void draw() {
        window.clear(Color::Black);

        // ����ʳ��
        Sprite foodSprite(foodTexture);
        if (!foodTexture.loadFromFile("Images/food.jpg")) {
            throw "Failed to load food.";

        }
        else {
            foodSprite.setTexture(foodTexture);

        }
        foodSprite.setPosition(foodX * GRID_SIZE, foodY * GRID_SIZE);
        // ������С����Ӧ����
        foodSprite.setScale(
            float(GRID_SIZE) / foodTexture.getSize().x,
            float(GRID_SIZE) / foodTexture.getSize().y
        );
        window.draw(foodSprite);
        /*RectangleShape foodRect(Vector2f(GRID_SIZE - 2, GRID_SIZE - 2));
        foodRect.setFillColor(Color::Red);
        foodRect.setPosition(foodX * GRID_SIZE + 1, foodY * GRID_SIZE + 1);
        window.draw(foodRect);*/

        // ������
        for (size_t i = 0; i < snake.size(); ++i) {
            //RectangleShape segment(Vector2f(GRID_SIZE - 2, GRID_SIZE - 2));
            //segment.setPosition(snake[i].x * GRID_SIZE + 1, snake[i].y * GRID_SIZE + 1);

            //// ͷ���ò�ͬ��ɫ
            //if (i == 0) {
            //    segment.setFillColor(Color::Green);
            //}
            //else {
            //    segment.setFillColor(Color(100, 255, 100)); // ǳ��ɫ
            //}
            Sprite segment(snakeTexture);
            if (!snakeTexture.loadFromFile("Images/snake.jpg")) {
                throw "Failed to load snake.";

            }
            else {
                segment.setTexture(snakeTexture);

            }
            segment.setPosition(snake[i].x * GRID_SIZE, snake[i].y * GRID_SIZE);

            // ������С
            segment.setScale(
                float(GRID_SIZE) / snakeTexture.getSize().x,
                float(GRID_SIZE) / snakeTexture.getSize().y
            );

            // ͷ�������в�ͬ���������ɫ
            if (i == 0) {
                segment.setColor(Color::Green);
            }
            else {
                segment.setColor(Color(100, 255, 100));
            }
            window.draw(segment);
        }

        // ���Ʒ���
        window.draw(scoreText);

        // �����Ϸ��������ʾ��Ϸ�����ı�
        if (gameOver) {
            window.draw(gameOverText);
        }

        window.display();
    }

    // ������Ϸ
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
        // ��ʼ������
        
        if (!font.loadFromFile("arial.ttf")) {
            // �������ʧ�ܣ�ʹ��Ĭ������
            font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
        }

        // ���÷����ı�
        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(10, 10);

        // ������Ϸ�����ı�
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

                // �������봦��
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

            // ��Ϸ�߼�����
            if (!gameOver && !paused && clock.getElapsedTime().asSeconds() > speed) {
                clock.restart();
                moveSnake();
                checkCollision_easy();
            }

            // ����
            draw();
           
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
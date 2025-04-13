#include <GLUT/glut.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

// ---------------------- Structures ----------------------
struct Star {
    float x, y, z;
    float r, g, b;
};

struct Asteroid {
    float x, y, z;
    float size;
    float speed;
    float angle;
    float rotationSpeed;
    float driftX;
    float driftY;
};

struct Bullet {
    float x, y, z;
    float dx, dy, dz;
};

struct Explosion {
    float x, y, z;
    float life;   // in frames
    float size;
};

// ---------------------- Global Variables ----------------------
float shipX = 0.0f, shipY = 0.0f, shipZ = -5.0f;
float rotateX = 0.0f, rotateY = 0.0f;
bool perspective = true;

std::vector<Star> stars;
std::vector<Asteroid> asteroids;
std::vector<Bullet> bullets;
std::vector<Explosion> explosions;

int score = 0;
int spaceshipHealth = 3;
float flashTimer = 0.0f;  // when >0, spaceship flashes red
int level = 1;
float cameraShake = 0.0f; // camera shake intensity

// Constants
const float starSpeed = 0.1f;

// Retro colors for stars
struct Color { float r, g, b; };
Color retroColors[] = {
    {0.0f, 1.0f, 0.0f},   // Green
    {0.5f, 0.0f, 0.5f},   // Purple
    {1.0f, 0.0f, 0.0f},   // Red
    {1.0f, 1.0f, 0.0f},   // Yellow
    {1.0f, 1.0f, 1.0f}    // White
};
const int numRetroColors = sizeof(retroColors) / sizeof(retroColors[0]);

// ---------------------- Generation Functions ----------------------

// Generate stars with random positions (z between -50 and -5) and retro colors.
void generateStars(int numStars) {
    stars.clear();
    for (int i = 0; i < numStars; i++) {
        Star s;
        s.x = ((rand() % 200) - 100) / 10.0f;
        s.y = ((rand() % 200) - 100) / 10.0f;
        s.z = - (rand() % 46 + 5);
        int ci = rand() % numRetroColors;
        s.r = retroColors[ci].r;
        s.g = retroColors[ci].g;
        s.b = retroColors[ci].b;
        stars.push_back(s);
    }
}

// Generate asteroids with random positions, sizes, speeds, rotation and drift.
// Now generating more asteroids with a size between 0.2 and 0.5.
void generateAsteroids(int numAsteroids) {
    asteroids.clear();
    for (int i = 0; i < numAsteroids; i++) {
        Asteroid a;
        a.x = ((rand() % 200) - 100) / 10.0f;
        a.y = ((rand() % 200) - 100) / 10.0f;
        a.z = - (rand() % 50 + 5);
        a.size = ((((rand() % 30) / 100.0f) + 0.2f) * 1.5f);
        a.speed = ((rand() % 50) / 1000.0f) + 0.005f;
        a.angle = rand() % 360;
        a.rotationSpeed = ((rand() % 20) - 10) / 10.0f; // -1 to 1 degrees per frame
        a.driftX = ((rand() % 20) - 10) / 1000.0f;
        a.driftY = ((rand() % 20) - 10) / 1000.0f;
        asteroids.push_back(a);
    }
}

// ---------------------- OpenGL Setup ----------------------
void setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (perspective) {
        gluPerspective(60.0, 1.0, 1.0, 150.0);
    } else {
        glOrtho(-10, 10, -10, 10, 1, 150);
    }
    glMatrixMode(GL_MODELVIEW);
}

// ---------------------- Drawing Functions ----------------------

// Draw the rocket-style spaceship.
void drawRocketSpaceship() {
    glPushMatrix();
    glTranslatef(shipX, shipY, shipZ);
    glRotatef(rotateX, 1, 0, 0);
    glRotatef(rotateY, 0, 1, 0);

    // Choose color: flash red if hit, otherwise pastel rocket color
    if (flashTimer > 0)
        glColor3f(1.0f, 0.0f, 0.0f);
    else
        glColor3f(0.8f, 0.7f, 0.9f);

    // Draw nose cone (triangle)
    glBegin(GL_POLYGON);
        glVertex3f(0.0f, 0.6f, 0.0f);
        glVertex3f(0.15f, 0.3f, 0.0f);
        glVertex3f(-0.15f, 0.3f, 0.0f);
    glEnd();

    // Draw body (rectangle)
    glBegin(GL_POLYGON);
        glVertex3f(-0.15f, 0.3f, 0.0f);
        glVertex3f(0.15f, 0.3f, 0.0f);
        glVertex3f(0.15f, -0.3f, 0.0f);
        glVertex3f(-0.15f, -0.3f, 0.0f);
    glEnd();

    // Draw fins (triangles) at the bottom
    glBegin(GL_TRIANGLES);
        glVertex3f(-0.15f, -0.3f, 0.0f);
        glVertex3f(-0.25f, -0.45f, 0.0f);
        glVertex3f(-0.15f, -0.45f, 0.0f);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex3f(0.15f, -0.3f, 0.0f);
        glVertex3f(0.25f, -0.45f, 0.0f);
        glVertex3f(0.15f, -0.45f, 0.0f);
    glEnd();

    glPopMatrix();
}

// Draw bullets as white square lasers.
void drawBullets() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f); // white bullet laser color
    for (const auto &b : bullets) {
        glPushMatrix();
        glTranslatef(b.x, b.y, b.z);
        // Draw bullet as a big, long, square
        float halfWidth = 0.1f;
        float halfHeight = 0.05f;
        glBegin(GL_QUADS);
            glVertex3f(-halfWidth, -halfHeight, 0.0f);
            glVertex3f( halfWidth, -halfHeight, 0.0f);
            glVertex3f( halfWidth, halfHeight, 0.0f);
            glVertex3f(-halfWidth, halfHeight, 0.0f);
        glEnd();
        glPopMatrix();
    }
}

// Draw stars
void drawStars() {
    glDisable(GL_LIGHTING);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const auto &s : stars) {
        glColor3f(s.r, s.g, s.b);
        glVertex3f(s.x, s.y, s.z);
    }
    glEnd();
}

// Draw asteroids as solid spheres with rotation.
void drawAsteroids() {
    glDisable(GL_LIGHTING);
    for (const auto &a : asteroids) {
        glPushMatrix();
        glTranslatef(a.x, a.y, a.z);
        glRotatef(a.angle, 0, 0, 1);
        glColor3f(0.6f, 0.6f, 0.6f);
        glutSolidSphere(a.size, 10, 10);
        glPopMatrix();
    }
}

// Draw explosion effects as fading circles.
void drawExplosions() {
    glDisable(GL_LIGHTING);
    for (const auto &ex : explosions) {
        float alpha = ex.life / 30.0f; // fade based on remaining life
        glColor4f(1.0f, 0.5f, 0.0f, alpha);
        glPushMatrix();
        glTranslatef(ex.x, ex.y, ex.z);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(0, 0);
            int numSegments = 20;
            for (int i = 0; i <= numSegments; i++) {
                float theta = (2.0f * 3.1415926f * float(i)) / float(numSegments);
                float dx = ex.size * cosf(theta);
                float dy = ex.size * sinf(theta);
                glVertex2f(dx, dy);
            }
        glEnd();
        glPopMatrix();
    }
}

// ---------------------- Update Functions ----------------------
void updateStars() {
    for (auto &s : stars) {
        s.z += starSpeed;
        if (s.z > -1.0f) {
            s.x = ((rand() % 200) - 100) / 10.0f;
            s.y = ((rand() % 200) - 100) / 10.0f;
            s.z = -50.0f;
            int ci = rand() % numRetroColors;
            s.r = retroColors[ci].r;
            s.g = retroColors[ci].g;
            s.b = retroColors[ci].b;
        }
    }
}

void updateBullets() {
    for (size_t i = 0; i < bullets.size(); i++) {
        bullets[i].x += bullets[i].dx;
        bullets[i].y += bullets[i].dy;
        bullets[i].z += bullets[i].dz;
        if (bullets[i].z < -100 || bullets[i].z > 10 ||
            bullets[i].x > 10 || bullets[i].x < -10 ||
            bullets[i].y > 10 || bullets[i].y < -10) {
            bullets.erase(bullets.begin() + i);
            i--;
        }
    }
}

void updateAsteroids() {
    for (auto &a : asteroids) {
        a.z += a.speed;
        a.x += a.driftX;
        a.y += a.driftY;
        a.angle += a.rotationSpeed;
        if (a.z > 0.0f) {
            a.z = - (rand() % 50 + 5);
            a.x = ((rand() % 200) - 100) / 10.0f;
            a.y = ((rand() % 200) - 100) / 10.0f;
        }
    }
}

void updateExplosions() {
    for (size_t i = 0; i < explosions.size(); i++) {
        explosions[i].life -= 1.0f;
        explosions[i].size += 0.005f;  // expand slowly
        if (explosions[i].life <= 0) {
            explosions.erase(explosions.begin() + i);
            i--;
        }
    }
}

// Check collisions between bullets and asteroids and between spaceship and asteroids.
void checkCollisions() {
    // Bullet - Asteroid collisions
    for (size_t i = 0; i < bullets.size(); i++) {
        for (size_t j = 0; j < asteroids.size(); j++) {
            float dx = bullets[i].x - asteroids[j].x;
            float dy = bullets[i].y - asteroids[j].y;
            float dz = bullets[i].z - asteroids[j].z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);
            if (dist < asteroids[j].size) {
                // Create an explosion effect at asteroid position
                Explosion ex;
                ex.x = asteroids[j].x;
                ex.y = asteroids[j].y;
                ex.z = asteroids[j].z;
                ex.life = 30.0f;
                ex.size = 0.1f;
                explosions.push_back(ex);
                // (Placeholder: play explosion sound here)
                
                bullets.erase(bullets.begin() + i);
                asteroids.erase(asteroids.begin() + j);
                score += 10;
                i--;
                break;
            }
        }
    }
    
    // Spaceship - Asteroid collisions
    // (Assume spaceship collision radius ~0.3)
    for (size_t j = 0; j < asteroids.size(); j++) {
        float dx = shipX - asteroids[j].x;
        float dy = shipY - asteroids[j].y;
        float dz = shipZ - asteroids[j].z;
        float dist = sqrt(dx*dx + dy*dy + dz*dz);
        if (dist < (asteroids[j].size + 0.3f)) {
            // Spaceship hit!
            spaceshipHealth--;
            flashTimer = 30.0f;    // flash for 30 frames
            cameraShake = 0.5f;    // trigger camera shake
            // Create explosion at spaceship
            Explosion ex;
            ex.x = shipX; ex.y = shipY; ex.z = shipZ;
            ex.life = 30.0f;
            ex.size = 0.1f;
            explosions.push_back(ex);
            // Remove the asteroid
            asteroids.erase(asteroids.begin() + j);
            // (Placeholder: play damage sound)
            break;
        }
    }
    
    // Level progression: every 100 points, increase level and boost asteroid speeds.
    if (score >= level * 100) {
        level++;
        for (auto &a : asteroids) {
            a.speed *= 1.1f;
        }
    }
}

// ---------------------- UI Rendering ----------------------
void drawRetroUI() {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 600, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw a retro green border.
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2i(5, 5);
        glVertex2i(595, 5);
        glVertex2i(595, 595);
        glVertex2i(5, 595);
    glEnd();
    
    // Title, score, level and health.
    const char* title = "Retro Space Navigator FINAL";
    glRasterPos2i(10, 580);
    for (const char* c = title; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
    
    std::string scoreText = "Score: " + std::to_string(score);
    glRasterPos2i(10, 560);
    for (char c : scoreText) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
    
    std::string levelText = "Level: " + std::to_string(level);
    glRasterPos2i(10, 540);
    for (char c : levelText) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
    
    std::string healthText = "Health: " + std::to_string(spaceshipHealth);
    glRasterPos2i(10, 520);
    for (char c : healthText) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
    
    // If game over, show GAME OVER message.
    if (spaceshipHealth <= 0) {
        const char* gameOver = "GAME OVER";
        glRasterPos2i(250, 300);
        for (const char* c = gameOver; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ---------------------- Display & Callback ----------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Apply camera shake (random small offset) if active.
    float shakeX = 0.0f, shakeY = 0.0f;
    if (cameraShake > 0) {
        shakeX = ((rand() % 10) - 5) / 100.0f;
        shakeY = ((rand() % 10) - 5) / 100.0f;
    }
    
    setupProjection();
    gluLookAt(shipX + shakeX, shipY + 1 + shakeY, shipZ + 5,
              shipX, shipY, shipZ,
              0, 1, 0);
    
    glDisable(GL_LIGHTING);
    drawStars();
    drawAsteroids();
    drawBullets();
    drawRocketSpaceship();
    drawExplosions();
    drawRetroUI();
    
    glutSwapBuffers();
}

void timer(int value) {
    if (spaceshipHealth > 0) {
        updateStars();
        updateBullets();
        updateAsteroids();
        updateExplosions();
        checkCollisions();
    }
    
    if (flashTimer > 0)
        flashTimer--;
    if (cameraShake > 0)
        cameraShake *= 0.95f; // gradually reduce shake
    
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ---------------------- Input Handling ----------------------
// Increased movement step for faster spaceship control.
void keyboard(unsigned char key, int x, int y) {
    if (spaceshipHealth <= 0)
        return; // no movement if game over
    
    switch (key) {
        case 'w': shipY += 0.8f; break;
        case 's': shipY -= 0.8f; break;
        case 'a': shipX -= 0.8f; break;
        case 'd': shipX += 0.8f; break;
        case 'q': shipZ += 0.8f; break;
        case 'e': shipZ -= 0.8f; break;
        case 'p': perspective = !perspective; break;
        case 27: exit(0); // ESC to exit
        case ' ': {
            // Spawn bullet from spaceship's center that travels straight ahead (-Z).
            Bullet b;
            b.x = shipX;
            b.y = shipY;
            b.z = shipZ;
            b.dx = 0.0f;
            b.dy = 0.0f;
            b.dz = -0.5f;
            bullets.push_back(b);
            break;
        }
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    if (spaceshipHealth <= 0)
        return;
    
    switch (key) {
        case GLUT_KEY_LEFT:  rotateY -= 5; break;
        case GLUT_KEY_RIGHT: rotateY += 5; break;
        case GLUT_KEY_UP:    rotateX -= 5; break;
        case GLUT_KEY_DOWN:  rotateX += 5; break;
    }
    glutPostRedisplay();
}

// ---------------------- Main ----------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Retro Space Navigator FINAL");
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING); // Use flat retro colors.
    
    generateStars(200);
    generateAsteroids(60);
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    
    return 0;
}

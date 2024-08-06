int x, y;
SDL_GetRelativeMouseState(&x, &y);
camera->rotation += (float)x * 0.0005;
camera->rotationZ -= (float)y * 0.0005;
camera->rotationZ = clamp(camera->rotationZ, (-3.14159 / 2) + 0.01, (3.14159 / 2) - 0.01);

// Move
float direction = 0;
float directionZ = 0;
float speed = 0;
const float MOVE_SPEED = 0.08;
float pitch = camera->rotationZ;
float yaw = camera->rotation;

vec3 forward = {
    cos(pitch) * cos(yaw),
    cos(pitch) * sin(yaw),
    sin(pitch)
};

vec3 right = {
    -sin(yaw),
    cos(yaw),
    0
};

vec3 up = {0, 0, 1};
const float cameraSpeed = 6 * game->delta;

if (game->keyboard[SDL_SCANCODE_W]) {
    vec3 move;
    glm_vec3_scale(forward, cameraSpeed, move);
    glm_vec3_add(camera->eyes, move, camera->eyes);
}
if (game->keyboard[SDL_SCANCODE_D]) {
    vec3 move;
    glm_vec3_scale(right, cameraSpeed, move);
    glm_vec3_add(camera->eyes, move, camera->eyes);
} 
if (game->keyboard[SDL_SCANCODE_A]) {
    vec3 move;
    glm_vec3_scale(right, -cameraSpeed, move);
    glm_vec3_add(camera->eyes, move, camera->eyes);
} 
if (game->keyboard[SDL_SCANCODE_S]) {
    vec3 move;
    glm_vec3_scale(forward, -cameraSpeed, move);
    glm_vec3_add(camera->eyes, move, camera->eyes);
}
if (game->keyboard[SDL_SCANCODE_SPACE]) {
    camera->eyes[2] += cameraSpeed;
}
if (game->keyboard[SDL_SCANCODE_LCTRL]) {
    camera->eyes[2] -= cameraSpeed;
}
if (speed != 0) {
    camera->eyes[0] -= speed * cos(direction);
    camera->eyes[1] -= speed * sin(direction);
    camera->eyes[2] += speed * tan(directionZ);
}